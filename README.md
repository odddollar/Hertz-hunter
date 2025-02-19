# Hertz Hunter

A poor-man's [RF Explorer](https://j3.rf-explorer.com/) for FPV drones. Useful for quickly determining what frequencies are in use, where background noise is occurring, and diagnosing malfunctioning video transmitters. Designed to be cheap and easy to build yourself.

Made with an ESP32-C3 microcontroller, RX5808 video receiver, OLED display, and power management components.

### Wires

- 4x SPI (RX5808 channels)
- 2x I^2^C (OLED display)
- 2x ADC (RX5808 rssi, battery monitoring)
- 3x Digital input (navigation buttons)
