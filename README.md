# Adaptive Street Lighting Network on a RISC-V FPGA Soft-Core

The aim is to implement a network of street lights that only lights up when a pedestrian is detected. On each node of the network there will be a VexRiscv RV32 RISC-V soft-core microcontroller implemented on a Lattice iCE40 UP5K FPGA, with FreeRTOS as the operating system. A real-time task reads a thermal array sensor and an ambient light sensor over I2C. It classifies heat signatures to tell people from animals and controls a PWM-dimmed lamp, simulated by the FPGA board’s RGB LED. A second task handles communication between the lamp posts. The posts send each other short radio messages, using radio modules, to say when someone has been detected, confirm the messages arrived, and check that every post is still working. The two tasks are synchronised through FreeRTOS queues and semaphores. The real-time analysis will measure the latency from detection to light-on, both locally and across nodes, along with its worst-case bounds.

Main components per lamp post node:
- custom Lattice iCE40 UP5K FPGA board
- thermal array sensor (AMG8833)
- ambient light sensor (BH1750)
- sub-GHz radio module (RFM69HCW 868 MHz)
- onboard RGB LED, driven by PWM

Some benefits of this project are reduced light pollution in low-traffic areas (such as rural areas) and the possibility of reducing energy consumption.
