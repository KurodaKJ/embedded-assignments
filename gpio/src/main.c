#include <stdbool.h>
#include "stm32f303xe.h"

void GPIO_Init(void) {
    // Enable Clock for GPIO ports (usually they're off to save power).
    // AHB: Advanded High-performance Bus.
    // ENR: Enable Register
    // For now port A, b and C are enabled
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN);

    // Note: ~ symbol is to flip the bit
    // So for example GPIOSOMETHING_SOMEREGISTER is 0000 0001 becomes 1111 1110

    // MODER: Mode Register (Set to Input/Output etc.)
    GPIOA->MODER &= ~GPIO_MODER_MODER9_Msk;     // Wipe out configuration for PA9 (D8)
    GPIOA->MODER |= GPIO_MODER_MODER9_0;        // Set mode to output

	GPIOC->MODER &= ~GPIO_MODER_MODER7_Msk;     // Wipe out configuration for PC7 (D9)
    GPIOC->MODER |= GPIO_MODER_MODER7_0;        // Set mode to output

    GPIOA->MODER &= ~GPIO_MODER_MODER8_Msk;     // Set mode to input (D7)
    GPIOB->MODER &= ~GPIO_MODER_MODER10_Msk;    // Set mode to input (D6)

    // PUPDR: Pull-Up / Pull-Down Register
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR8_Msk;     // Wipe out configuration for PA8 (D7)
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_0;        // Set to 01 (Pull-up)

    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR10_Msk;    // Wipe out configuration for PB10 (D6)
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR10_1;       // Set to 10 (Pull-down)
}

int main(void) {
    GPIO_Init();

    bool isLedOn = false;
    bool isLock = false;

    while (1) {
        // IDR: Input data register for inputs
        // ODR: Output data register for outputs

        // Check if PA8 (or D7) is 0 (Pressed)
        if (!(GPIOA->IDR & GPIO_IDR_8)) {
            GPIOA->ODR |= GPIO_ODR_9;           // Turn LED on
        } else {
            GPIOA->ODR &= ~GPIO_ODR_9;          // Turn LED off
        }

        // Check if PB10 (or D6) is 1 (Pressed)
        if (GPIOB->IDR & GPIO_IDR_10) {
            // Only enter if this is a "new" press
            if (!isLock) {
                isLedOn = !isLedOn;             // Set state to the opposite

                if (isLedOn) {
                    GPIOC->ODR |= GPIO_ODR_7;   // Turn LED ON
                } else {
                    GPIOC->ODR &= ~GPIO_ODR_7;  // Turn LED off
                }

                isLock = true;
            }
        } else {
            isLock = false;
        }
    }

    return 0;
}
