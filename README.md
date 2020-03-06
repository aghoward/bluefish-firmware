# bluefish password manager

bluefish is a hardware based password manager made to be easily transportable, cheap,
and secure.


## What's in this repository?

This repository contains the firmware and KiCad / Gerber files necessary for the hardware
to work. The hardware is based on the Arduino platform and requires some libraries from
the official Arduino standard library. You'll also need a few supporting libraries from
my own collection: [STL](https://github.com/aghoward/stl) and 
[either](https://github.com/aghoward/either). These will both need to be installed along
side other arduino libraries for the compiler to find them.

A make file has been provided which should allow you to compile the code for your atmel
chip; out of the box you can specify one of the following: `UNO=true`, `LEONARDO=true`,
`NANO=true`, or `MIGHTY1284P` to build for a specific platform. The most tested of these
is the UNO platform which is a atmel ATMega328P.

You can use `make` to build the code; and `make up` to use avrdude to flash the firmware
to your atmel chip. Have a look inside the hardware folder to find the wiring schematics
and all files necessary to print your own PCBs.

