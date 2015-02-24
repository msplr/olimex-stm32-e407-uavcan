/*
 * Copyright (C) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
 */

#include <unistd.h>
#include <ch.hpp>
#include <hal.h>
#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/protocol/NodeStatus.hpp>

#include "debug.h"
#include <errno.h>

#define TIME_SYNC_MASTER 1

#if TIME_SYNC_MASTER
#define UAVCAN_NODE_ID 1
#include <uavcan/protocol/global_time_sync_master.hpp>
#else
#define UAVCAN_NODE_ID 2
#include <uavcan/protocol/global_time_sync_slave.hpp>
#endif

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
    palWritePad(GPIOC, GPIOC_LED, state);
}

#if __GNUC__
__attribute__((noreturn))
#endif
void die(int status)
{
    lowsyslog("Now I am dead x_x %i\n", status);
    while (1)
    {
        ledSet(false);
        chThdSleepMilliseconds(500);
        ledSet(true);
        chThdSleepMilliseconds(500);
    }
}

void node_status_cb(const uavcan::ReceivedDataStructure<uavcan::protocol::NodeStatus>& msg)
{
    const uint8_t st = msg.status_code;
    const char *st_name;
    switch (st) {
        case 0:
            st_name = "STATUS_OK";
            break;
        case 1:
            st_name = "STATUS_INITIALIZING";
            break;
        case 2:
            st_name = "STATUS_WARNING";
            break;
        case 3:
            st_name = "STATUS_CRITICAL";
            break;
        case 15:
            st_name = "STATUS_OFFLINE";
            break;
        default:
            st_name = "UNKNOWN_STATUS";
            break;
    }
    palTogglePad(GPIOC, GPIOC_LED);
    lowsyslog("NodeStatus from %d: %u (%s)\n", msg.getSrcNodeID().get(), st, st_name);
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
#if TIME_SYNC_MASTER
        uavcan::UtcDuration adjustment;
        uint64_t utc_time_init = 1234;
        adjustment = uavcan::UtcTime::fromUSec(utc_time_init) - uavcan::UtcTime::fromUSec(0);
        node.getSystemClock().adjustUtc(adjustment);

        static uavcan::GlobalTimeSyncMaster time_sync_master(node);
        {
            const int res = time_sync_master.init();;
            if (res < 0)
            {
                die(res);
            }
        }
#else
        static uavcan::GlobalTimeSyncSlave time_sync_slave(node);
        {
            const int res = time_sync_slave.start();
            if (res < 0)
            {
                die(res);
            }
        }
#endif

        /*
         * NodeStatus subscriber
         */
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
            const int spin_res = node.spin(uavcan::MonotonicDuration::fromMSec(1000));
            if (spin_res < 0)
            {
                lowsyslog("Spin failure: %i\n", spin_res);
            }

#if TIME_SYNC_MASTER
            time_sync_master.publish();
#else
            lowsyslog("Time sync master: %u\n", unsigned(time_sync_slave.getMasterNodeID().get()));
#endif

            lowsyslog("Memory usage: used=%u free=%u\n",
                      node.getAllocator().getNumUsedBlocks(), node.getAllocator().getNumFreeBlocks());

            lowsyslog("CAN errors: %lu\n",
                      static_cast<unsigned long>(can.driver.getIface(0)->getErrorCount()));

// #if !UAVCAN_TINY
//             node.getLogger().setLevel(uavcan::protocol::debug::LogLevel::INFO);
//             node.logInfo("app", "UTC %* sec, %* corr, %* jumps",
//                          uavcan_stm32::clock::getUtc().toMSec() / 1000,
//                          uavcan_stm32::clock::getUtcSpeedCorrectionPPM(),
//                          uavcan_stm32::clock::getUtcAjdustmentJumpCount());
// #endif
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

void _exit(int status){
   (void) status;
   chSysHalt();
   while(TRUE){}
}

pid_t _getpid(void){
   return 1;
}

#undef errno
extern int errno;
int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

} // extern "C"
