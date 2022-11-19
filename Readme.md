# TeensyBridge

Serial, Program, and NMI interface for Teensy 4.x (or any other board!) on a Raspberry Pi Pico board.

* USB to UART bridge on pins __GPIO0__ (Tx) and __GPIO1__ (Rx)<br>

* USB to UART bridge on pins __GPIO4__ (Tx) and __GPIO5__ (Rx)<br>

* Active LOW (pull up) Program signal for Teensy on __GPIO2__.<br>
To simulate a 50 ms button press, change the USB second serial port to __301__ baud (the actual UART speed won't change).

* Active HIGH (pull down) NMI signal (or any other use) on __GPIO3__.<br>
To simulate a 50 ms button press, change the USB second serial port speed to __302__ baud (the actual UART speed won't change).

* All other speed values and line parameters will actually change the UART settings.

* The default line parameters are 115200 b/s, 8N1 on the first serial port and 2 Mb/s, 8N1 on the second one.

* Other pins can be easily associated with other speed settings.

Licensed under BSD 3 clause (TeensyBridge, Pico SDK) and MIT (Tinyusb)