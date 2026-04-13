#include "stm32f303xe.h"

#define SERVO_MIN     100   // -90 degrees (1.0ms)
#define SERVO_CENTER  150   //   0 degrees (1.5ms)
#define SERVO_MAX     200   // +90 degrees (2.0ms)
#define LED_MAX_BRIGHT 2000 // Full ARR is 2000

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
    GPIOA->MODER &= ~(GPIO_MODER_MODER2_Msk | GPIO_MODER_MODER3_Msk);     // Wipe out configuration for PA3 (D0) and PA2 (D1)
    GPIOA->MODER |= (GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);        // Set mode to alternate function (for LED and for SERVO)

    // AFR: Alternate Function Register (the bridge of TIM2 peripheral to GPIO)
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
    // According to the data sheet, it count from 0 to the end value, thats why we subtracted by 1.
    TIM2->PSC = 79;             // 8MHz / 80 = 1KHz, which is 10 microseconds per tick
    TIM2->ARR = 1999;           // The whole cycle will be then 20ms (20000 ticks x 10us = 20ms)
    TIM2->CNT = 0;              // Reset counter so the first pulse is perfect

    TIM2->CCMR2 &= ~(TIM_CCMR2_OC3M_Msk | TIM_CCMR2_OC4M_Msk);                  // Clear settings
    TIM2->CCMR2 |= (0x6U << TIM_CCMR2_OC3M_Pos) | (0x6U << TIM_CCMR2_OC4M_Pos); // PWM mode 1 for CH3 and CH4
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;                           // Preload enable

    TIM2->CCER |= TIM_CCER_CC3E | TIM_CCER_CC4E;                                // Enable outputs for CH2 and CH3
    TIM2->CR1 |= TIM_CR1_CEN;
}

int main(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);  // CPU runs at 8MHz or 8 Mil, thus divide by 1000 will be 0.001 sec

    GPIO_Init();
    TIM_Init();

    while (1) {
        // At start
        TIM2->CCR3 = SERVO_MIN;
        TIM2->CCR4 = 0; // LED off at start
        delay_ms(1000);

        // Move to 0
        for (int i = SERVO_MIN; i <= SERVO_CENTER; i++) {
            TIM2->CCR3 = i;
            TIM2->CCR4 = (i - SERVO_MIN) * 40;
            delay_ms(20);                      // 50 steps * 20ms = 1000ms
        }
        delay_ms(1000);

        // Move to +90
        for (int i = SERVO_CENTER; i <= SERVO_MAX; i++) {
            TIM2->CCR3 = i;
            TIM2->CCR4 = 2000 - ((i - SERVO_CENTER) * 40);
            delay_ms(20);
        }
        delay_ms(1000);

        // Move to 0
        for (int i = SERVO_MAX; i >= SERVO_CENTER; i--) {
            TIM2->CCR3 = i;
            TIM2->CCR4 = (SERVO_MAX - i) * 40;
            delay_ms(20);
        }
        delay_ms(1000);

        // Move to -90
        for (int i = SERVO_CENTER; i >= SERVO_MIN; i--) {
            TIM2->CCR3 = i;
            TIM2->CCR4 = (i - SERVO_MIN) * 40;
            delay_ms(20);
        }
    }

    return 0;
}
