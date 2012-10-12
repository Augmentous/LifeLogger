
//******************************************************************************
//
//  GoPro HD Camera Control using BUS connector
//  for MSP430F2012
//
//  Uses PWR/MODE button to wake camera up and take photo in One Button Mode
//
//  Set SHOT_RATE for number of seconds between shots
//  If not using xtal control, time requires CALIBRATION
//
//  closes PWR button for 250 mSec to wake up camera
//  waits  3.75 seconds for snap and store
//  closes PWR button for 3 seconds to turn camera off
//  flashes LED to indicate start of next cycle
//
//  Peter Jennings  http://benlo.com/msp430
//
//******************************************************************************
//Modified by Jacob Rosenthal and Colin Ho for Lifelogger Camera

#include "msp430.h"
#include<signal.h>

#define SHOT_RATE 158    //   seconds between shots
#define CALIBRATION 7  //   calibration to improve accuracy


#define WAITING 0    //   waiting for next cycle
#define STARTING 1   //   button down to start
#define WAITING_CAMERA 2  //   waiting for camera to take pic
#define STOPPING 3   //   button down to stop

static int tick;
static int state;

static int time;  // seconds since last save


void main(void)
  {
  WDTCTL = WDTPW + WDTHOLD;           // Stop watchdog timer

  P1SEL |= 0x00;                      // P1.1 option select - just I/O
  P1DIR |= 0x11;                      // Set P1.0 P1.4 to output direction
  P1OUT |= 0x10;                      // LED off, GoPro button off

  BCSCTL2 |= DIVS_3;                  // SMCLK/8
  WDTCTL = WDT_MDLY_32;               // WDT Timer interval 32mS

  tick = 0;
  time = 10;                       // wait for cap to charge up
  state = WAITING;

  IE1   |= WDTIE;                  // Enable WDT interrupt 256 mSec
  _BIS_SR(LPM0_bits + GIE);        // Enter LPM0 with interrupt
  }

// Watchdog Timer interrupt service routine
interrupt(WDT_VECTOR) watchdog_timer(void)
  {
  if ( (state == STARTING) && (time >= 3 ) )  // start takes .25 seconds
      {
      state = WAITING_CAMERA;
      P1OUT  |= 0x10;           // button up
      }

  if ( tick & 0x03 )            // most of the time (does not enter mod 3=0)
     {
     P1OUT &= ~0x01;            // LED off and go back to sleep
  } else                          // about once very 1.024 seconds
     {
     time++;
     if ( (state == WAITING) && (time >= SHOT_RATE+CALIBRATION) )  // time for photo
         {
         P1OUT &= ~0x10;             // button down
         time = 0;
         tick = 0;
         state = STARTING;
         }
     else if ((state == WAITING_CAMERA) && (time >= 8) ) // time to turn off
         {
         state = STOPPING;
         P1OUT &= ~0x10;             // button down
         }
     else if ((state == STOPPING) && (time >= 11)) // should be off now
         {
         state = WAITING;
         P1OUT  |= 0x10;             // button up
         P1OUT  |= 0x01;           // LED flash to indicate done cycle
         }
     }
  tick++;                  // 256 mSec ticks
  }

