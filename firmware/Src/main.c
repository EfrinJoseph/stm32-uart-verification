#include <stdint.h>

#define RCC_AHB1ENR  (*(volatile uint32_t*)0x40023830)
#define RCC_APB2ENR  (*(volatile uint32_t*)0x40023844)
#define GPIOA_MODER  (*(volatile uint32_t*)0x40020000)
#define GPIOA_AFRH   (*(volatile uint32_t*)0x40020024)
#define USART1_SR    (*(volatile uint32_t*)0x40011000)
#define USART1_DR    (*(volatile uint32_t*)0x40011004)
#define USART1_BRR   (*(volatile uint32_t*)0x40011008)
#define USART1_CR1   (*(volatile uint32_t*)0x4001100C)

int main(void)
{
    RCC_AHB1ENR |= (1U << 0);                            // clock: GPIOA
    RCC_APB2ENR |= (1U << 4);                            // clock: USART1

    GPIOA_MODER &= ~(3U << 18);  GPIOA_MODER |= (2U << 18);   // PA9  = AF mode
    GPIOA_MODER &= ~(3U << 20);  GPIOA_MODER |= (2U << 20);   // PA10 = AF mode
    GPIOA_AFRH  &= ~(15U << 4);  GPIOA_AFRH  |= (7U << 4);    // PA9  -> AF7 (USART1)
    GPIOA_AFRH  &= ~(15U << 8);  GPIOA_AFRH  |= (7U << 8);    // PA10 -> AF7 (USART1)

    USART1_BRR = 139;                                     // 115200 baud at 16 MHz
    USART1_CR1 |= (1U << 13) | (1U << 3) | (1U << 2);     // UART on, TX on, RX on

    while (1)
    {
        while (!(USART1_SR & (1U << 5)));   // wait: byte arrived?
        uint8_t c = USART1_DR;              // read it
        while (!(USART1_SR & (1U << 7)));   // wait: transmitter free?
        USART1_DR = c;                      // send it back
    }
}
