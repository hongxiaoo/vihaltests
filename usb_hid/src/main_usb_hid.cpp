/*
 * file:     main_usb_hid.cpp
 * brief:    USB HID example for VIHAL
 * created:  2021-11-18
 * authors:  nvitya
 *
 * description:
 *   Simple HID example which just moves the mouse cursor every second
*/

#include "platform.h"
#include "hwpins.h"
#include "hwclk.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwusbctrl.h"
#include "board_pins.h"
#include "usb_hid_test.h"

#include "traces.h"

volatile unsigned hbcounter = 0;

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else
  #if 0
    SystemCoreClock = MCU_INTERNAL_RC_SPEED;
  #else
    //if (!hwclk_init(0, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
    //if (!hwclk_init(0, 64000000))  // special for STM32F3, STM32F1
    if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
    {
      while (1)
      {
        // error
      }
    }
  #endif
#endif

  mcu_enable_fpu();    // enable coprocessor if present
  mcu_enable_icache(); // enable instruction cache if present

  clockcnt_init();

  // go on with the hardware initializations
  board_pins_init();
  traces_init();

  TRACE("\r\n--------------------------------------\r\n");
  TRACE("VIHAL USB HID TEST\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  TRACE_FLUSH();

  mcu_enable_interrupts();

  usb_hid_test_init();

  unsigned hbclocks = SystemCoreClock / 2;

  unsigned t0, t1;

  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    usb_hid_test_run();

    tracebuf.Run();

    if (t1-t0 > hbclocks)
    {
      ++hbcounter;

      usb_hid_test_heartbeat();  // just move the mouse cursor...

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

      t0 = t1;
    }
  }
}

// ----------------------------------------------------------------------------
