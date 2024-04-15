# Desksniffer

This code is for the ESP32, and is part of a project to connect a VIVO Electric
Dual Motor Standing Desk Frame (V122EB) to the internet. It reads data that is
being sent to a Aip650EO LCD Display Driver from Wuxi I-core Elec. The format is
kinda-I2C. It is a 3 segment display, where each segment is its own i2c device.
Each segment receives exactly one byte of data, and the MSB of that byte
indicates whether a period should be displayed after the digit. It will output
to serial what is being displayed on the LCD. If you got here looking for how to
sniff i2c for your own hardware, you might want to look at
https://github.com/kn100/I2C-sniffer as that project is far more general. No
guarantees it works though :)

CHAOS.