
#ifndef TIMER_H_
#define TIMER_H_

// add helper symbolic link to the UIF flag in the counter status register
#define SR_UIF		(1U<<0)
#define SR_CC1IF	(1U<<1)
void timer2_1Hz_init(void);
void timer2_1Hz_compare(void);
void timer3_input_capture(void);
void timer2_1Hz_interrupt_init(void);

#endif /* TIMER_H_ */
