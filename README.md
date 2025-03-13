# Hertz Hunter

A poor-man's [RF Explorer](https://j3.rf-explorer.com/) for FPV drones. Useful for quickly determining which frequencies are in use, where background noise is occurring, and diagnosing malfunctioning video transmitters (VTXs). Designed to be cheap and easy to build yourself.

At a racing event I attended there was an issue with someone's damaged VTX broadcasting at full power on two channels, thus interfering with another pilot. A spectrum analyser was essential for diagnosing this issue, as two peaks at different frequencies could be seen in the spectrum graph when only the damaged VTX was powered on.

This project aims to make this useful tool more accessible to pilots and race organisers, and can be easily added to a race-day tool bag. It uses a common RX5808 video receiver to scan from 5645MHz to 5945MHz and displays a graph of the received signal strength (RSSI) on different frequencies within this range on a small OLED display.

### Features

- Scanning of the RF spectrum commonly used by FPV racing drones (5645MHz to 5945MHz)
- Graphing RSSI to show which frequencies VTXs are broadcasting on
- Three buttons (`PREV`, `SEL`, `NEXT`) for navigating menus and controlling the device
- Selectable scanning interval
    - A 5MHz interval offers the highest resolution at the slowest update rate
    - A 20MHz interval offers the lowest resolution at the fastest update rate
- Battery voltage monitoring with low battery alarm
- Calibration between known low and high RSSI values

### Potential goals

> [!NOTE]
>
> No commitment is made to these goals. They're things I think would be cool to do, but may never actually see the light of day.

- Custom PCB with integrated power management
- 3D printed case for a custom PCB
- API accessible from a Wi-Fi hotspot for integration with other software
- Web interface to interact with the scanner and display more detailed graphs

## Hardware



## Software



## Usage

