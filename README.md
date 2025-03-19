# Hertz Hunter

A poor-man's [RF Explorer](https://j3.rf-explorer.com/) for FPV drones. Useful for quickly determining which frequencies are in use, where background noise is occurring, and diagnosing malfunctioning video transmitters (VTXs). Designed to be cheap (<$60 AUD) and easy to build yourself.

At a racing event I attended there was an issue with someone's damaged VTX broadcasting at full power on two channels, thus interfering with another pilot. A spectrum analyser was essential for diagnosing this issue, as two peaks at different frequencies could be seen in the spectrum graph when only the damaged VTX was powered on.

This project aims to make this useful tool more accessible to pilots and race organisers, and can be easily added to a race-day tool bag. It uses a common RX5808 video receiver to scan from 5645MHz to 5945MHz and displays a graph of the received signal strength (RSSI) on different frequencies within this range on a small OLED display.

*Example of a soldered prototype*

<div align="center">
    <img src="./images/Device example.jpg" width="40%">
    <img src="./images/Scan example.jpg" width="40%">
</div>


## Features

- Scanning of the RF spectrum commonly used by FPV racing drones (5645MHz to 5945MHz)
- Graphing RSSI to show which frequencies VTXs are broadcasting on
- Three buttons (`PREV`, `SEL`, `NEXT`) for navigating menus and controlling the device
- Selectable scanning interval
    - A 5MHz interval offers the highest resolution at the slowest update rate
    - A 10MHz interval offers a medium resolution at a medium update rate
    - A 20MHz interval offers the lowest resolution at the fastest update rate
- Battery voltage monitoring with low battery alarm
- Calibration between known low and high RSSI values
- Displaying calibrated signal strength for the selected frequency

### Potential future features

> [!NOTE]
>
> No commitment is made to implementing these. They're things I think would be cool to do, but may never actually see the light of day.

- Custom PCB with integrated power management circuitry
- 3D printed case for a custom PCB
- API accessible from a Wi-Fi hotspot for integration with other software
- Web interface to interact with the scanner and display more detailed graphs

## Hardware

### Components

These components can be connected together on a bread-board, or soldered more permanently onto some type of perf-board. All prices are in Australian dollars (AUD).

- 1x [ESP32-C3 Super Mini](https://www.aliexpress.com/w/wholesale-esp32-c3-super-mini.html) (<$5)
- 1x [RX5808 with SPI mod](https://www.aliexpress.com/w/wholesale-rx5808-spi.html) (\$25 to \$50 depending on seller)
- 1x [1.3" I<sup>2</sup>C 128x64 OLED](https://www.aliexpress.com/w/wholesale-1.3-oled.html) (<$5)
    - I use an OLED with the SH1106 controller chip, but the SSD1306 chip *should* work as well. The modifications that need to be made to the source code are explained in [Configuration](#Configuration)
- 1x [Active 3.3V buzzer](https://www.aliexpress.com/w/wholesale-active-buzzer.html) (<$3)
- 1x [TP4056 lithium battery charger module](https://zaitronics.com.au/products/tp4056-type-c-18650-lithium-battery-charger-protection) (<$2)
- 1x [5V boost converter](https://zaitronics.com.au/products/mt3608-step-up-module) (<$3)
    - If using an adjustable boost converter, set the output to 5V using a multimeter. Lock the potentiometer in place with a dab of super glue
- 3x Momentary buttons
- 2x 100kΩ resistors
- 1x Power switch
- 1x Li-ion/Li-po cell
- 1x 5.8GHz antenna
    - I've used a U.FL to SMA pigtail so I can connect an external antenna

### Wiring

![Wiring](./images/Wiring.png)

## Software

### Setup



### Configuration



### Flashing



## Usage

### Menus



### Scanning



### Calibration



### Resetting

Due to the fact that the settings and calibration values are stored in non-volatile memory, flashing the firmware again won't wipe them. If, for some reason, the device needs to be completely reset, press `PREV`, `SEL` and `NEXT` simultaneously. The device should reboot with everything completely reset.
