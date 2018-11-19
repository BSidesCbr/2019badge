/*! @file isr.h
 * @brief Header file of interrupt service routines.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef ISR_H_
#define ISR_H_

/*------------------------------------------------------------------------*/
/*                            Global macros                               */
/*------------------------------------------------------------------------*/

/*!
 * @brief Definition of enable all ITs.
 */
#define mIsr_EnableAllIt() EA = 1

/*!
 * @brief Definition of disable all ITs.
 */
#define mIsr_DisableAllIt() EA = 0

/*------------------------------------------------------------------------*/
/*                          Function prototypes                           */
/*------------------------------------------------------------------------*/
#ifdef SDCC
/*! Interrupt Service Handler function prototype declarations required
 * by SDDC */
INTERRUPT_PROTO(vIsr_Timer2Isr, INTERRUPT_TIMER2);
INTERRUPT_PROTO(vIsr_PcaTimerIsr, INTERRUPT_PCA0);
#endif

/*------------------------------------------------------------------------*/
/*                          External variables                            */
/*------------------------------------------------------------------------*/

extern volatile BIT wIsr_Timer2Tick;


#endif /* ISR_H_ */
