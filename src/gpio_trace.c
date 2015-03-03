
#include <hal.h>
#include <ch.h>

void TRACE3(int x)
{
    if (x == 1) {
        palSetPad(GPIOC, GPIOC_UEXT3);
    } else if (x == 0) {
        palClearPad(GPIOC, GPIOC_UEXT3);
    } else {
        palTogglePad(GPIOC, GPIOC_UEXT3);
    }
}

void TRACE4(int x)
{
    if (x == 1) {
        palSetPad(GPIOC, GPIOC_UEXT4);
    } else if (x == 0) {
        palClearPad(GPIOC, GPIOC_UEXT4);
    } else {
        palTogglePad(GPIOC, GPIOC_UEXT4);
    }
}

void TRACE5(int x)
{
    if (x == 1) {
        palSetPad(GPIOB, GPIOB_UEXT5);
    } else if (x == 0) {
        palClearPad(GPIOB, GPIOB_UEXT5);
    } else {
        palTogglePad(GPIOB, GPIOB_UEXT5);
    }
}

void TRACE6(int x)
{
    if (x == 1) {
        palSetPad(GPIOB, GPIOB_UEXT6);
    } else if (x == 0) {
        palClearPad(GPIOB, GPIOB_UEXT6);
    } else {
        palTogglePad(GPIOB, GPIOB_UEXT6);
    }
}

void TRACE7(int x)
{
    if (x == 1) {
        palSetPad(GPIOC, GPIOC_UEXT7);
    } else if (x == 0) {
        palClearPad(GPIOC, GPIOC_UEXT7);
    } else {
        palTogglePad(GPIOC, GPIOC_UEXT7);
    }
}

void TRACE8(int x)
{
    if (x == 1) {
        palSetPad(GPIOC, GPIOC_UEXT8);
    } else if (x == 0) {
        palClearPad(GPIOC, GPIOC_UEXT8);
    } else {
        palTogglePad(GPIOC, GPIOC_UEXT8);
    }
}

// reserved for usart3
// void TRACE9(int x)
// {
//     if (x == 1) {
//         palSetPad(GPIOB, GPIOB_UEXT9);
//     } else if (x == 0) {
//         palClearPad(GPIOB, GPIOB_UEXT9);
//     } else {
//         palTogglePad(GPIOB, GPIOB_UEXT9);
//     }
// }

void TRACE10(int x)
{
    if (x == 1) {
        palSetPad(GPIOG, GPIOG_UEXT10);
    } else if (x == 0) {
        palClearPad(GPIOG, GPIOG_UEXT10);
    } else {
        palTogglePad(GPIOG, GPIOG_UEXT10);
    }
}
