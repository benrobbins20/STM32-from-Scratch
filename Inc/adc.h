#ifndef ADC_H_
#define ADC_H_

void adc_init(void);
void start_conversion(void);
uint32_t adc_read(void);
void adc_interrupt_init(void);

#define SR_EOC		(1U<<1)

#endif /* ADC_H_ */
