
#include "platform.h"
//#include "hwclkctrl.h"
#include "hwpins.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "self_flashing.h"

#if defined(BOARD_VRV153)

TGpioPin  pin_led1(PORTNUM_A, 0, false);
THwSpi    spi;
TSpiFlash spiflash;

#define LED_COUNT 1

void setup_board()
{
	pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
	//pin_led1.Setup(PINCFG_INPUT);

	spi.speed = 8000000;
	spi.Init(1); // flash

	spiflash.spi = &spi;
	spiflash.has4kerase = true;
	spiflash.Init();
}

#endif

#ifndef LED_COUNT
  #define LED_COUNT 1
#endif

// the C libraries require "_start" so we keep it as the entry point
extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required
{
	// after ram setup, region copy the cpu jumps here, with probably RC oscillator

	// TODO: !!!
  mcu_preinit_code(); // inline code for preparing the MCU, RAM regions. Without this even the stack does not work on some MCUs.

	// run the C/C++ initialization:
	cppinit();

	// Set the interrupt vector table offset, so that the interrupts and exceptions work
	mcu_init_vector_table();

	unsigned cpu_clock_speed;

#if defined(MCU_FIXED_SPEED)

	cpu_clock_speed = MCU_FIXED_SPEED;

#else

  #error "Setup CPU clock!"
	{
		while (1)
		{
			// the external oscillator did not start.
		}
	}
#endif


	// provide info to the system about the clock speed:
	SystemCoreClock = cpu_clock_speed;

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	setup_board();

#if defined(BOARD_VRV153)
	if (self_flashing)
	{
		spi_self_flashing(&spiflash, BOOTBLOCK_STADDR);
	}
#endif

	mcu_enable_interrupts();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast
	unsigned hbcounter = 0;

	unsigned t0, t1;

	t0 = CLOCKCNT;

	volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		if (t1-t0 > hbclocks)
		{
			++hbcounter;
			*hexnum = hbcounter;

			pin_led1.Toggle();

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
