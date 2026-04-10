#include "stm32f303xe.h"

volatile uint32_t ticks = 0;

void GPIO_Init(void) {
    // Enable Clock for GPIO ports (usually they're off to save power).
    // AHB: Advanded High-performance Bus.
    // APB2: Advanced Peripheral Bus 2
    // ENR: Enable Register
    // For now port A and Timer 2 are enabled
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Note: ~ symbol is to flip the bit
    // So for example GPIOSOMETHING_SOMEREGISTER is 0000 0001 becomes 1111 1110

    // MODER: Mode Register (Set to Input/Output etc.)
    // Outputs
    GPIOA->MODER &= ~GPIO_MODER_MODER3_Msk;     // Wipe out configuration for PA3 (D0)
    GPIOA->MODER |= GPIO_MODER_MODER3_1;        // Set mode to alternate function (for LED PWM)

	GPIOA->MODER &= ~GPIO_MODER_MODER2_Msk;     // Wipe out configuration for PA2 (D1)
    GPIOA->MODER |= GPIO_MODER_MODER2_1;        // Set mode to alternate function (for SERVO)

    //[0]: Low (0-7) [1]: High (8-15)
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL2_Msk | GPIO_AFRL_AFRL3_Msk);
    GPIOA->AFR[0] |= (1U << GPIO_AFRL_AFRL2_Pos) | (1U << GPIO_AFRL_AFRL3_Pos); // AF1 for both PA2 and PA3
}

void SysTick_Handler(void) {
    ticks++;
}

void delay_ms(uint32_t ms) {
    uint32_t started = ticks;
    while ((ticks - started) < ms);
}

void TIM_Init(void) {
    TIM2->PSC = 80;
    TIM2->ARR = 1999;

    TIM2->CCMR2 &= ~(TIM_CCMR2_OC3M_Msk | TIM_CCMR2_OC4M_Msk);
    TIM2->CCMR2 |= (0x6U << TIM_CCMR2_OC3M_Pos) | (0x6U << TIM_CCMR2_OC4M_Pos); // PWM mode 1 for CH3 and CH4
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE; // Preload enable

    TIM2->CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E; // Enable outputs for CH3 and CH4
    TIM2->CR1 |= TIM_CR1_CEN;
}

int main(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);

    GPIO_Init();
    TIM_Init();

    while (1) {
        // Slow move to -90
        for(int i = 150; i >= 100; i--) {
            TIM2->CCR3 = i;
            TIM2->CCR4 = (150 - i) * 40; // Fade LED ON (0 -> 2000)
            delay_ms(20);
        }

        // Pause for 1 sec
    }

    return 0;
}
