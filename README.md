# STM32F4_CanBus_Communicator
This repository contains code that operates the CanBus controller on the STM32F4 microcontroller

How to hook up the board:

	1) Connect the STM32F4 CanBus to a MCP2551 High-Speed Can Transceiver.Look at http://ww1.microchip.com/downloads/en/DeviceDoc/21667f.pdf for wiring details
	
	2) Enable a slew rate of 24 V/us on the Can Transceiver by connecting RS to GND through a 10K resistor.  

	3) Hookup the MCP2551 to a differential bus in order to reduce the possibility of noise interfering with any signals

This code was developed using the KEIL UVision IDE. Due to potential copyright issues only the source code has been included.

Note: This code can easily be modified to transmit or receive different signals and is free to use
