# cynpro
Simple tool for a Razer Cynosa Pro keyboard to control led state on Linux

Dependencies: libusb 1.0

To use it you need root privileges so use it with sudo or set the "s" bit.

Usage: cynpro [-c green|blue|cyan] [-l on|off] [-b on|off|cycling] [-i 0-255]
 -c Led color: green, blue, cyan
 -l Led state: on, off
 -b Led breathing mode: on, off, cycling
 -i Led intensity: 0-255
