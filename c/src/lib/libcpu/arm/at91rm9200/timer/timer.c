/*
 * Cogent CSB337 Timer driver
 *
 * This uses timer 0 for timing measurments.
 *
 * Copyright (c) 2004 by Jay Monkman <jtm@lopingdog.com>
 *	
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *
 *  http://www.OARcorp.com/rtems/license.html.
 *
 * Notes:
 *  This file manages the benchmark timer used by the RTEMS Timing Test
 *  Suite.  Each measured time period is demarcated by calls to
 *  Timer_initialize() and Read_timer().  Read_timer() usually returns
 *  the number of microseconds since Timer_initialize() exitted.
 *
 *  It is important that the timer start/stop overhead be determined 
 *  when porting or modifying this code.
 *
 *  $Id$
 */

#include <rtems.h>
#include <bsp.h>
#include <at91rm9200.h>
#include <at91rm9200_pmc.h>

rtems_unsigned16 tstart;
rtems_boolean Timer_driver_Find_average_overhead;
unsigned32 tick_time;
/*
 * Set up TC0 - 
 *   timer_clock2 (MCK/8)
 *   capture mode - this shouldn't matter
 */
void Timer_initialize( void )
{
    unsigned32 tmr_freq;

    /* since we are using timer_clock2, divide mck by 8 */
    tmr_freq = at91rm9200_get_mck() / 8;

    TC_TC0_REG(TC_CMR) = TC_CMR_TCCLKS(1);   /* timer_clock2 */
    TC_TC0_REG(TC_CCR) = (TC_CCR_CLKEN       /* enable the counter */
                          | TC_CCR_SWTRG);   /* start it up */

    /* tick time in nanoseconds */
    tick_time = 1000000000/tmr_freq;

}

/*
 *  The following controls the behavior of Read_timer().
 *
 *  AVG_OVEREHAD is the overhead for starting and stopping the timer.  It
 *  is usually deducted from the number returned.
 *
 *  LEAST_VALID is the lowest number this routine should trust.  Numbers
 *  below this are "noise" and zero is returned.
 */

#define AVG_OVERHEAD      0  /* It typically takes X.X microseconds */
                             /* (Y countdowns) to start/stop the timer. */
                             /* This value is in microseconds. */
#define LEAST_VALID       1  /* Don't trust a clicks value lower than this */

int Read_timer( void )
{
  rtems_unsigned16 t;
  rtems_unsigned32 total;
  t = TC_TC0_REG(TC_CV);

  /*
   *  Total is calculated by taking into account the number of timer overflow
   *  interrupts since the timer was initialized and clicks since the last
   *  interrupts.
   */

  total = t * tick_time;

  if ( Timer_driver_Find_average_overhead == 1 )
    return total;          /* in nanosecond units */
  else {
    if ( total < LEAST_VALID )
      return 0;            /* below timer resolution */
  /*
   *  Somehow convert total into microseconds
   */
      return (total - AVG_OVERHEAD);
    }
}

/*
 *  Empty function call used in loops to measure basic cost of looping
 *  in Timing Test Suite.
 */

rtems_status_code Empty_function( void )
{
  return RTEMS_SUCCESSFUL;
}

void Set_find_average_overhead(
  rtems_boolean find_flag
)
{
  Timer_driver_Find_average_overhead = find_flag;
}

