#include <LPC23XX.H>                        /* LPC23xx definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "demo.h"
#include "LCD.h"


U8  Mute;                                   /* Mute State */
U32 Volume;                                 /* Volume Level */

#if USB_DMA
#pragma arm section zidata = "USB_RAM"
U32 InfoBuf[P_C];                           /* Packet Info Buffer */
S16 DataBuf[B_S];                           /* Data Buffer */
#pragma arm section zidata
#else
S16 DataBuf[B_S];                           /* Data Buffer */
#endif

U16 DataOut;                                /* Data Out Index */
U16 DataIn;                                 /* Data In Index */

U8  DataRun;                                /* Data Stream Run State */

U16 PotVal;                                 /* Potenciometer Value */

U32 VUM;                                    /* VU Meter */

U32 Tick;                                   /* Time Tick */


/*
 * Get Potentiometer Value
 */

void get_potval (void) {
  U32 val;

  AD0CR |= 0x01000000;                      /* Start A/D Conversion */
  do {
    val  = AD0GDR;                          /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);        /* Wait for end of A/D Conversion */
  AD0CR &= ~0x01000000;                     /* Stop A/D Conversion */
  PotVal = ((val >> 8) & 0xF8) +            /* Extract Potenciometer Value */
           ((val >> 7) & 0x08);
}


/*
 * Timer Counter 0 Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency)
 */

void tc1_isr (void) __irq {
  long  val;
  U32 cnt;

  if (DataRun) {                            /* Data Stream is running */
    val = DataBuf[DataOut];                 /* Get Audio Sample */
    cnt = (DataIn - DataOut) & (B_S - 1);   /* Buffer Data Count */
    if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
      DataOut++;                            /* Skip one Sample */
    }
    if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
      DataOut++;                            /* Update Data Out Index */
    }
    DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
    if (val < 0) VUM -= val;                /* Accumulate Neg Value */
    else         VUM += val;                /* Accumulate Pos Value */
    val  = Volume;                         / Apply Volume Level */
    val >>= 16;                             /* Adjust Value */
    val  += 0x8000;                         /* Add Bias */
    val  &= 0xFFFF;                         /* Mask Value */
  } else {
    val = 0x8000;                           /* DAC Middle Point */
  }

  if (Mute) {
    val = 0x8000;                           /* DAC Middle Point */
  }

  DACR = val & 0xFFC0;                      /* Set Speaker Output */

  if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
    get_potval();                           /* Get Potenciometer Value */
    if (VolCur == 0x8000) {                 /* Check for Minimum Level */
      Volume = 0;                           /* No Sound */
    } else {
      Volume = VolCur * PotVal;             /* Chained Volume Level */
    }
    val = VUM >> 20;                        /* Scale Accumulated Value */
    VUM = 0;                                /* Clear VUM */
    if (val > 7) val = 7;                   /* Limit Value */
    FIO2CLR = LED_MSK;                      /* Turn Off all LEDs */
    FIO2SET = LED_MSK >> (7 - val);         /* LEDs show VU Meter */
  }

  T1IR = 1;                                 /* Clear Interrupt Flag */
  VICVectAddr = 0;                          /* Acknowledge Interrupt */
}


/* Main Program */

int main (void) {

  PINSEL10 = 0;                             /* Disable ETM interface */
  PINSEL1  = 0x00204000;                    /* Select AOUT,AIN0 */
  FIO2DIR |= LED_MSK;                       /* LEDs, port 2, bit 0~7 output only */

  PCONP  |= (1 << 12);                      /* Enable power to AD block    */
  AD0CR   = 0x00200E01;                     /* ADC: 10-bit AIN0 @ 4MHz */
  DACR    = 0x00008000;                     /* DAC Output set to Middle Point */

  /* Setup Timer Counter 1: Periodic Timer with Interrupt at DATA_FREQ Rate */
  T1MR0 = VPB_CLOCK/DATA_FREQ - 1;          /* TC1 Match Value 0 */
  T1MCR = 3;                                /* TC1 Interrupt and Reset on MR0 */
  T1TCR = 1;                                /* TC1 Enable */

  /* Setup Timer Counter 1 Interrupt */
  VICVectAddr5 = (unsigned long)tc1_isr;    /* TC1 Interrupt -> Vector 5   */
  VICVectCntl5 = 0x02;                      /* TC1 Interrupt -> Priority 2 */
  VICIntEnable = 1 << 5;                    /* Enable TC1 Interrupt */

  lcd_init();
  lcd_clear();
  lcd_print ((unsigned char*)"Audio mp3");
  set_cursor (0, 1);
  lcd_print ((unsigned char*)"Group 11");

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
