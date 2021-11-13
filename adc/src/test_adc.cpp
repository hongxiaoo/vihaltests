/*
 *  file:     test_adc.cpp
 *  brief:    ADC Test
 *  date:     2021-11-13
 *  authors:  nvitya
 *
 *  description:
 *    two ADC channels are continously measured and printed to the UART console
*/

#include "stdint.h"
#include "stdlib.h"
#include "platform.h"
#include "hwpins.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "board_pins.h"
#include "traces.h"

#include "hwadc.h"

THwAdc adc;

#define ADC_REC_LEN  1024

uint16_t adc_rec_buffer[ADC_REC_LEN];

#define ADC_ANALYZE_LEN  256

uint16_t adc_value_cnt[ADC_ANALYZE_LEN];

uint8_t adc_ch_x;
uint8_t adc_ch_y;
uint8_t adc_num;

uint8_t adc_shift = 6; // keep only the highest 10 bits

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_MIN_F103) || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
    || defined(BOARD_MIBO64_STM32F405) \
    || defined(BOARD_MIBO48_STM32G473)

void adc_init()
{
  hwpinctrl.PinSetup(PORTNUM_A, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ch0
  hwpinctrl.PinSetup(PORTNUM_A, 1, PINCFG_INPUT | PINCFG_ANALOGUE); // ch1

  adc_num = 1;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#elif defined(BOARD_MIBO48_STM32F303)

void adc_init()
{
  hwpinctrl.PinSetup(PORTNUM_A, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ch0
  hwpinctrl.PinSetup(PORTNUM_A, 1, PINCFG_INPUT | PINCFG_ANALOGUE); // ch1

  adc_num = 1;
  adc_ch_x = 1;
  adc_ch_y = 2;
}


#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743) || defined(BOARD_NUCLEO_H723)

void adc_init()
{
  hwpinctrl.PinSetup(PORTNUM_A, 3, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC123_in3
  hwpinctrl.PinSetup(PORTNUM_C, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC123_in10

  adc_num = 1;
  adc_ch_x = 3;
  adc_ch_y = 10;
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE) || defined(BOARD_MIBO64_ATSAM4S)

void adc_init()
{
  // The ATSAM does not require pin setup, the pins switched automatically to analogue mode on channel enabling

  // CH0, CH1 Pins:
  // Atsam-4S: PA17, PA18
  // Arduino DUE: PA2(AD7), PA3(AD6)
  // ATSAM-E70: PD30, PA21

  adc_num = 0;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#elif defined(BOARD_XPLAINED_SAME70)

void adc_init()
{
  // The ATSAM does not require pin setup, the pins switched automatically to analogue mode on channel enabling

  adc_num = 0;
  adc_ch_x = 0;  // PD30 (AD2): AFE0_AD0
  adc_ch_y = 8;  // PA19 (AD3): AFE0_AD8
}

#elif 0 //defined(BOARD_MIBO64_ATSAME5X)


#else
  #error "ADC board specific setup is missing"
#endif


void adc_test_freerun()
{
  TRACE("*** ADC Test Freerun ***\r\n");

  unsigned t0, t1;
  unsigned hbcounter = 0;

  unsigned dcnt = 0;

  unsigned sampspeed = 20;

  t0 = CLOCKCNT;
  while (1)
  {
    t1 = CLOCKCNT;
    if (t1 - t0 > SystemCoreClock / sampspeed)
    {
      uint16_t advx = (adc.ChValue(adc_ch_x) >> adc_shift);
      uint16_t advy = (adc.ChValue(adc_ch_y) >> adc_shift);

      TRACE("\rx: %5u, y: %5u", advx, advy);

      // and finally do the usual led blinking

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

      board_show_hexnum(hbcounter);

      t0 = t1;
    }


  }
}

void test_adc()
{
  TRACE("Testing ADC\r\n");

  adc_init();

  adc.Init(adc_num, (1 << adc_ch_x) | (1 << adc_ch_y));
  //adc.Init(adc_num, (1 << adc_ch_x));

  TRACE("ADC speed: %u conversions / s\r\n", adc.act_conv_rate);

  adc_test_freerun();  // does not return

  TRACE("ADC test finished.\r\n");
}

// ----------------------------------------------------------------------------
