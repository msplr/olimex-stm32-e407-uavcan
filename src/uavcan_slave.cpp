/*
 * Copyright (C) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
 */

#include <unistd.h>
#include <ch.hpp>
#include <hal.h>
#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/protocol/NodeStatus.hpp>
#include <uavcan/test/ServiceCallStorm.hpp>

#include "debug.h"
#include <errno.h>

#define UAVCAN_NODE_ID 2
#define SERVER_NODE_ID 1
#include <uavcan/protocol/global_time_sync_slave.hpp>

namespace app
{
namespace
{

uavcan_stm32::CanInitHelper<128> can;

typedef uavcan::Node<16384> Node;

uavcan::LazyConstructor<Node> node_;

Node& getNode()
{
    if (!node_.isConstructed())
    {
        node_.construct<uavcan::ICanDriver&, uavcan::ISystemClock&>(can.driver, uavcan_stm32::SystemClock::instance());
    }
    return *node_;
}

void ledSet(bool state)
{
    if (state) {
        palWritePad(GPIOC, GPIOC_LED, 0);
    } else {
        palWritePad(GPIOC, GPIOC_LED, 1);
    }
}

#if __GNUC__
__attribute__((noreturn))
#endif
void die(int status)
{
    lowsyslog("Now I am dead x_x %i\n", status);
    ledSet(true);
    while (1)
    {
        // chThdSleepMilliseconds(500);
    }
}

void node_status_cb(const uavcan::ReceivedDataStructure<uavcan::protocol::NodeStatus>& msg)
{
    lowsyslog("CAN errors: %lu\n", static_cast<unsigned long>(can.driver.getIface(0)->getErrorCount()));
}

class : public chibios_rt::BaseStaticThread<8192>
{
public:
    msg_t main()
    {
        int res;
        res = can.init(1000000);
        if (res < 0)
        {
            die(res);
        }

        /*
         * Setting up the node parameters
         */
        Node& node = app::getNode();

        node.setNodeID(UAVCAN_NODE_ID);
        node.setName("org.uavcan.cvra_test_stm32f407");

        // TODO: fill software version info (version number, VCS commit hash, ...)
        // TODO: fill hardware version info (version number, unique ID)

        /*
         * Initializing the UAVCAN node - this may take a while
         */
        while (true)
        {
            // Calling start() multiple times is OK - only the first successfull call will be effective
            int res = node.start();

#if !UAVCAN_TINY
            uavcan::NetworkCompatibilityCheckResult ncc_result;
            if (res >= 0)
            {
                lowsyslog("Checking network compatibility...\n");
                res = node.checkNetworkCompatibility(ncc_result);
            }
#endif

            if (res < 0)
            {
                lowsyslog("Node initialization failure: %i, will try agin soon\n", res);
            }
#if !UAVCAN_TINY
            else if (!ncc_result.isOk())
            {
                lowsyslog("Network conflict with %u, will try again soon\n", ncc_result.conflicting_node.get());
            }
#endif
            else
            {
                break;
            }
            chThdSleepMilliseconds(1000);
        }

        /*
         * Time synchronizer
         */
        lowsyslog("Time sync slave start\n");
        static uavcan::GlobalTimeSyncSlave time_sync_slave(node);
        {
            const int res = time_sync_slave.start();
            if (res < 0)
            {
                die(res);
            }
        }

        lowsyslog("Test client start\n");
        static uint8_t service_call_counter = 0;
        using uavcan::test::ServiceCallStorm;
        uavcan::ServiceClient<ServiceCallStorm> client(node);
        {
            const int res = client.init();
            if (res < 0)
            {
                die(res);
            }
        }
        client.setCallback([](const uavcan::ServiceCallResult<ServiceCallStorm>& call_result)
        {
            palTogglePad(GPIOC, GPIOC_LED);
            if (call_result.isSuccessful()) {
                lowsyslog("sc-err\n");
                // lowsyslog("%d\n", call_result.response);
            } else {
                lowsyslog("service call failed");
            }
        });

        /*
         * NodeStatus subscriber
         */
        lowsyslog("NodeStatus subscriber start\n");
        uavcan::Subscriber<uavcan::protocol::NodeStatus> ns_sub(node);
        const int ns_sub_start_res = ns_sub.start(node_status_cb);
        if (ns_sub_start_res < 0) {
            lowsyslog("error NodeStatus subscriber init");
            while (1);
        }

        /*
         * Main loop
         */
        lowsyslog("UAVCAN node started\n");
        node.setStatusOk();
        while (true)
        {
            // const int spin_res = node.spin(uavcan::MonotonicDuration::fromMSec(1000));
            const int spin_res = node.spin(uavcan::MonotonicDuration::fromMSec(0));
            if (spin_res < 0)
            {
                lowsyslog("Spin failure: %i\n", spin_res);
            }

            // const bool active = time_sync_slave.isActive();
            // const int master_node_id = time_sync_slave.getMasterNodeID().get();
            // const long msec_since_last_adjustment = (node.getMonotonicTime() - time_sync_slave.getLastAdjustmentTime()).toMSec();
            // lowsyslog("Time sync master Node ID: %d, Active: %d\n"
            //           "    Last adjust %d ms ago, Monotonic: %U UTC: %U\n\n",
            //           master_node_id, int(active), msec_since_last_adjustment,
            //           node.getMonotonicTime().toUSec(), node.getUtcTime().toUSec());

            ServiceCallStorm::Request req;
            req.request = service_call_counter++;
            client.call(SERVER_NODE_ID, req);

            // lowsyslog("Memory usage: used=%u free=%u\n", node.getAllocator().getNumUsedBlocks(), node.getAllocator().getNumFreeBlocks());
            // lowsyslog("CAN errors: %lu\n", static_cast<unsigned long>(can.driver.getIface(0)->getErrorCount()));
        }
        return msg_t();
    }
} uavcan_node_thread;

}
}

extern "C" {

int uavcan_node_start(void *arg)
{
    (void) arg;
    lowsyslog("Starting the UAVCAN thread\n");
    app::uavcan_node_thread.start(LOWPRIO);
    return 0;
}

} // extern "C"
