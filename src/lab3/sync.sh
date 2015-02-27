#!/bin/bash

sudo avrdude -b 57600 -F -e -c arduino -P /dev/ttyUSB0 -p atmega328p -U flash:w:audio.hex 
