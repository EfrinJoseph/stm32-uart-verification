# STM32 UART Verification

A small device with a defined command protocol, built and verified from one written specification: a bare-metal UART bring-up on an STM32F407, and the protocol's command logic as SystemVerilog RTL, verified in simulation.

The specification is in [`docs/protocol-spec.md`](docs/protocol-spec.md). Every rule in it has a requirement ID (REQ-F1, REQ-S2, …) so that each test can be traced back to the rule it checks.

## Status

**Phase 1: SystemVerilog RTL and verification**

| Step | Content | Status |
| --- | --- | --- |
| 1 | Protocol spec + bare-metal UART echo on the STM32 | ✅ done |
| 2 | Command FSM in SystemVerilog (`rtl/`) | ✅ written, compiles; tests next |
| 3 | Command parser + UART RX/TX in SystemVerilog | planned |
| 4 | SystemVerilog testbench: one directed test per transition-table cell | planned |
| 5 | UVM environment (driver, monitor, scoreboard) + functional coverage | planned |

**Phase 2: hardware (optional)**

| Step | Content | Status |
| --- | --- | --- |
| 6 | Clock-tree configuration (HSE + PLL, MCO output) | planned |
| 7 | Command parser + FSM in firmware | planned |
| 8 | Python/pytest test harness over UART | planned |

**What works today:** the board receives bytes on USART1 and sends each one straight back (echo), tested with a serial terminal at 115200 baud. The command FSM exists as RTL and compiles cleanly with Icarus Verilog and Verilator; it has not been verified by tests yet.

## RTL

`rtl/` holds the SystemVerilog design, written from section 6 of the spec.

| File | Content |
| --- | --- |
| `rtl/fsm_pkg.sv` | Package with the state type `state_t` (IDLE, RUNNING, PAUSED, FAULT), shared by the RTL and, later, the testbench |
| `rtl/fsm.sv` | The command FSM: a state register (`always_ff`, asynchronous reset to IDLE, REQ-S1) and next-state logic (`always_comb`) implementing the 9 transitions of the transition table |

The FSM's inputs are one-clock pulses (`set_mode_run`, `set_mode_pause`, `set_mode_stop`, `trigger_fault`, `clear_fault`) that the command parser will produce from the received text. `PING`, `GET_STATE` and `SET_LED` never change the state, so they don't reach the FSM.

Compile check:

```
iverilog -g2012 rtl/fsm_pkg.sv rtl/fsm.sv
verilator --lint-only -Wall rtl/fsm_pkg.sv rtl/fsm.sv
```

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
