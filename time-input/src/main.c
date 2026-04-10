#include "stm32f303xe.h"
#define ARM_MATH_CM4

void GPIO_Init(void);
void TIME2_us_Delay(uint32_t delay);

uint32_t data;
double time, dist;

void GPIO_Init() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Set PA3 as output
    GPIOA->MODER &= ~GPIO_MODER_MODER3_Msk;
    GPIOA->MODER |= GPIO_MODER_MODER3_0;

    // Set PA2 as input
    GPIOA->MODER &= ~GPIO_MODER_MODER2_Msk;
}

void TIME2_us_Delay(uint32_t delay) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->ARR = (int)(delay/0.0625);
    TIM2->CNT &= ~TIM_CNT_CNT_Msk;
    TIM2->CR1 |= TIM_CR1_CEN;
    while(!(TIM2->SR & TIM_SR_UIF));
    TIM2->SR &= ~TIM_SR_UIF;
}