# Desksniffer

This code is for the ESP32, and is part of a project to connect a VIVO Electric
Dual Motor Standing Desk Frame (V122EB) to the internet. It is able to pilot 
the desk to a particular height via web requests, and also supports manual
control.

## How to use it

The electronics will be described at a later time - but if you're feeling daring
all you gotta do is attach your ESP32 to the I2C pins on your display controller,
and then use transistors connected to your ESP32 to attach to the up and down 
pins of your desk controller.

You'll find all the variables you need to edit in `desksniffer.cpp`.

## How it works

It sniffs the i2c data that is being sent to a Aip650EO LCD Display Driver from
Wuxi I-core Elec. to determine the current height of a desk. It then sits and
listens for web requests.

```
GET http://esp32-abcde/desk
{"height":720}

GET http://esp32-abcde/desk?height=780
`OK`
```

## Development

This is a PlatformIO project. Included is a `shell.nix` file that includes some
useful utilities for working with the esp32. They are not necessary. If you have
VS code, go install PlatformIO and then open this project just like any other.
You'll then be able to build and flash it to your very own ESP32.

It is structured like so:
* desksniffer.cpp - entrypoint
* deskheight.cpp - code for handling reading data from the i2c bus.
* aip650decoder.cpp - code for taking i2c frames and converting them to useful
  information.
* deskmover.cpp - code for managing the movement of a standing desk
* manualcontrols.cpp - code for managing the buttons you'll probably want to
  have on your rebuilt controller