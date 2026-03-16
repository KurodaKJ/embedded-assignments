#include <stdbool.h>
#include "stm32f303xe.h"

// volatile tells the compiler to not optimized (for using inside an interrupt)
volatile bool isLedOn = false;

void GPIO_Init(void) {
    // Enable Clock for GPIO ports (usually they're off to save power).
    // AHB: Advanded High-performance Bus.
    // APB2: Advanced Peripheral Bus 2
    // ENR: Enable Register
    // For now port A, B, C  and system configuration (SYSCF) are enabled
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN);
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;       // Required to route GPIO to EXTI (External Interrupt)

    // Note: ~ symbol is to flip the bit
    // So for example GPIOSOMETHING_SOMEREGISTER is 0000 0001 becomes 1111 1110

    // MODER: Mode Register (Set to Input/Output etc.)
    // Outputs
    GPIOA->MODER &= ~GPIO_MODER_MODER9_Msk;     // Wipe out configuration for PA9 (D8)
    GPIOA->MODER |= GPIO_MODER_MODER9_0;        // Set mode to output

	GPIOC->MODER &= ~GPIO_MODER_MODER7_Msk;     // Wipe out configuration for PC7 (D9)
    GPIOC->MODER |= GPIO_MODER_MODER7_0;        // Set mode to output

    // Inputs
    GPIOA->MODER &= ~GPIO_MODER_MODER8_Msk;     // Set mode to input (D7)
    GPIOB->MODER &= ~GPIO_MODER_MODER10_Msk;    // Set mode to input (D6)

    // PUPDR: Pull-Up / Pull-Down Register
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR8_Msk;     // Wipe out configuration for PA8 (D7)
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_0;        // Set to 01 (Pull-up)

    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR10_Msk;    // Wipe out configuration for PB10 (D6)
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR10_1;       // Set to 10 (Pull-down)

    // Interrupt Routing
    // Connect PA8 to EXTI line 8 and PB10 to EXTI line 10
    // CR: Control Register
    SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8 | SYSCFG_EXTICR3_EXTI10);        // Wipe out the configuration first
    SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PA | SYSCFG_EXTICR3_EXTI10_PB);

    // RT: Rising Trigger, FT: Falling Trigger
    // SR: Selection Register
    // Trigger on BOTH Falling (press) and Rising (release) (on D7)
    EXTI->FTSR |= EXTI_FTSR_TR8;
    EXTI->RTSR |= EXTI_RTSR_TR8;

    // To trigger on rising only (for pull-down press on D6)
    EXTI->RTSR |= EXTI_RTSR_TR10;

    // IMR: Interrupt Mask Register
    // Unmask (Enable) lines 8 and 10
    // Allows the signal to reach the CPU
    EXTI->IMR |= (EXTI_IMR_MR8 | EXTI_IMR_MR10);

    // NVIC: Nested Vectored Interrupt Controller
    // IRQ: Interrupt Request
    // These will make the main loop stop
    NVIC_EnableIRQ(EXTI9_5_IRQn);    // For line 8
    NVIC_EnableIRQ(EXTI15_10_IRQn);  // For line 10
}

// INTERRUPT HANDLERS HERE
// PR: Pennding Register
// Handler for B0 (button D7)
void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR8) {               // Check if it was pin 8
        if (!(GPIOA->IDR & GPIO_IDR_8)) {
            GPIOA->ODR |= GPIO_ODR_9;           // Button Down -> LED ON
        } else {
            GPIOA->ODR &= ~GPIO_ODR_9;          // Button Up -> LED OFF
        }
        EXTI->PR |= EXTI_PR_PR8;                // Clear the flag
    }
}

// Handler for B1 (button D6)
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR10) {              // Check if it was pin 10
        isLedOn = !isLedOn;                     // Toggle state

        if (isLedOn) {
            GPIOC->ODR |= GPIO_ODR_7;
        } else {
            GPIOC->ODR &= ~GPIO_ODR_7;
        }
        EXTI->PR |= EXTI_PR_PR10;               // Clear the flag
    }
}

int main(void) {
    GPIO_Init();

    // If wee don't stay in while loop, every process will end
    while (1) {
        // Stay empty and wait for something to do.
    }

    return 0;
}
