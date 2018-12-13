# ECE 4180 - Design Project
## 3D Printer Status Lights - Invention Studio @ Georgia Tech

### Group Memebers
* Brandon Redder
* Seth Peden
* Dylan Mauldin
* Jacob Southerland

### Purpose
* The invention studio has 30 3D printers
* They are each controlled by a web management system with Raspberry PI’s
* There is no easy way to physically see the statuses of the printers

### Hardware
* Each printer is already connected to a Raspberry Pi v3
* The status lights are ShiftBrite SPI LEDs (shift registers)
* Each printer has one light connected to it
* Initial designs for the project utilized discrete RGB LEDs or an alternate microcontroller (with embedded RGB LED)

### Software
* The lights are controlled using the PIGPIO C++ library
* Status data is pulled from a pre-existing cloud service called 3D Printer OS
* Data from 3D Printer OS is retrieved with in a JSON format from their REST API
