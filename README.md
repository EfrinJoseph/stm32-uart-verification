# STM32 UART Verification

A small device with a defined command protocol, verified end to end: first as bare-metal firmware on an STM32F407, later as SystemVerilog RTL in simulation, both checked against the same specification.

The specification is in [`docs/protocol-spec.md`](docs/protocol-spec.md). Every rule in it has a requirement ID (REQ-F1, REQ-S2, …) so that each test can be traced back to the rule it checks.

## Status

| Step | Content | Status |
| --- | --- | --- |
| 1 | Protocol spec + bare-metal UART echo | ✅ done |
| 2 | Command parser + FSM in firmware | planned |
| 3 | Python/pytest test harness over UART | planned |
| 4 | Clock-tree configuration (HSE + PLL, MCO output) | planned |
| 5 | SystemVerilog RTL twin + UVM testbench | planned |

**What works today:** the board receives bytes on USART1 and sends each one straight back (echo), tested with a serial terminal at 115200 baud.

## Firmware

Bare-metal C, no HAL and no CMSIS: every peripheral is configured by writing its registers directly, with addresses and bit positions taken from ST's reference manual RM0090.

`firmware/Src/main.c` does this:

1. Enables the clocks for GPIOA (`RCC_AHB1ENR`, bit 0) and USART1 (`RCC_APB2ENR`, bit 4).
2. Sets PA9 and PA10 to alternate function mode (`GPIOA_MODER` = `10`).
3. Routes both pins to USART1 with AF7 (`GPIOA_AFRH`).
4. Sets 115200 baud: `USART1_BRR` = 16 MHz / 115200 ≈ 139 (0.08 % error).
5. Enables the UART, transmitter and receiver (`USART1_CR1`, bits 13, 3, 2).
6. Echo loop: wait for RXNE, read `DR`, wait for TXE, write `DR`.

## Hardware

| Part | Notes |
| --- | --- |
| STM32F407VET6 core board | Generic black board, no on-board debugger |
| ST-LINK/V2 | Flashing and debugging over SWD (20-pin ribbon cable) |
| CP2102 USB-to-UART adapter | Multi-mode board: both DIP switches ON for TTL, voltage jumper on 3V3 |

Wiring for the UART:

| Board | Adapter |
| --- | --- |
| PA9 (USART1 TX) | RXD |
| PA10 (USART1 RX) | TXD |
| GND | GND |

## Build and run

1. In STM32CubeIDE: **File → Import → General → Existing Projects into Workspace**, select the `firmware` folder. The project appears as "led blink" (its original name).
2. Build (Ctrl+B), then **Run As → STM32 C/C++ Application** with the ST-LINK connected.
3. Open a serial terminal (e.g. PuTTY) on the adapter's COM port: 115200, 8N1, no flow control.
4. Type: every character comes back from the board.
