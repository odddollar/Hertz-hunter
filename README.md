# Hertz Hunter

## Introduction

A poor-man's [RF Explorer](https://j3.rf-explorer.com/) for FPV drones. Useful for quickly determining which frequencies are in use, where background noise is occurring, and diagnosing malfunctioning video transmitters (VTXs). Designed to be cheap (<$60 AUD) and easy to build yourself.

At a racing event I attended there was an issue with someone's damaged VTX broadcasting at full power on two channels, thus interfering with another pilot. A spectrum analyser was essential for diagnosing this issue, as two peaks at different frequencies could be seen in the spectrum graph when only the damaged VTX was powered on.

This project aims to make this useful tool more accessible to pilots and race organisers, and can be easily added to a race-day tool bag. It uses a common RX5808 video receiver to scan from 5645MHz to 5945MHz (and 5345MHz to 5645MHz for low-band channels) and displays a graph of the received signal strength (RSSI) on different frequencies within this range on a small OLED display.

*Example of a soldered prototype*

<div align="center">
    <img src="./images/Device example.jpg" alt="Device example" width="40%" />
    <img src="./images/Scan example.jpg" alt="Scan example" width="40%" />
</div>

## Features

- Scanning of the RF spectrum commonly used for video by FPV racing drones (5645MHz to 5945MHz) and additional low-band (5345MHz to 5645MHz) frequencies
- Graphing RSSI to show which frequencies VTXs are broadcasting on
- Three buttons (`PREV`, `SEL`, `NEXT`) for navigating menus and controlling the device
- Selectable scanning interval
    - A 2.5MHz interval offers the highest resolution at the slowest update rate
      - The RX5808 only supports 1MHz increments, so this interval is rounded to the nearest integer
    - A 5MHz interval offers a medium resolution at a medium update rate
    - A 10MHz interval offers the lowest resolution at the fastest update rate
- Battery voltage monitoring with a low battery alarm
- Calibration between known low and high RSSI values
- Displaying calibrated signal strength for the selected frequency
- Settings saved between reboots
- API accessible from a Wi-Fi hotspot for integration with other software

### Potential future features

> [!NOTE]
>
> No commitment is made to implementing these. They're things I think would be cool to do, but may never actually see the light of day.

- Custom PCB with integrated power management circuitry
- 3D printed case for a custom PCB
- Web interface to interact with the scanner and display more detailed graphs
    - **IN DEVELOPMENT- Check out the `web-ui` branch for progress updates**

## Further documentation

Further documentation can be found below:

- [`API.md`](API.md) - API documentation and schema
- [`HARDWARE.md`](HARDWARE.md) - Hardware designs
- [`SOFTWARE.md`](SOFTWARE.md) - Software setup and firmware flashing
- [`USAGE.md`](USAGE.md) - Firmware usage and settings

