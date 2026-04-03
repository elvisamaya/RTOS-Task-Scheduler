# RTOS Task Scheduler on STM32 with FreeRTOS

This project demonstrates a multitasking embedded firmware system built on an STM32 Nucleo board using FreeRTOS.

## Features

- Sensor polling task
- Motor control task
- UART logging + command shell task
- Status LED task
- Task priorities and preemption
- Queue-based task communication
- Mutex-protected UART output
- Task suspend, resume, and terminate via shell commands

## Hardware

- STM32 NUCLEO-F446RE
- SG90 servo
- USB cable
- Breadboard + jumper wires
- External 5V supply for servo

## UART commands

- `help`
- `status`
- `led off`
- `led on`
- `led kill`
- `motor open`
- `motor close`
