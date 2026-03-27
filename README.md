# RTOS-Task-Scheduler
This project is a multitasking embedded firmware demo built on an STM32 Nucleo board using FreeRTOS.

## Goal

Instead of writing one large `while(1)` loop, this project splits the firmware into separate concurrent tasks:

- sensor polling
- motor control
- UART logging / shell
- status LED blinking

## Planned features

- FreeRTOS task scheduling
- multiple priorities
- task communication with queues
- mutex-protected UART output
- task suspend / resume / terminate
- PWM motor / servo control
- user button as a simple sensor trigger

## Target hardware

- STM32 NUCLEO-F446RE
- SG90 servo
- USB serial terminal
