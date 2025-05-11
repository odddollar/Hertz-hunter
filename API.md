# API Documentation for Hertz Hunter

Hertz Hunter provides an API accessible from a Wi-Fi hotspot for the purpose of integrating the device with other software. The required format for interacting with this API is documented here, and includes the following features:

- Requesting up-to-date RSSI data
- Requesting the current battery voltage
- Requesting the current settings for the scan interval, buzzer state, and low battery alarm
- Requesting the calibrated minimum and maximum signal strength values

> [!NOTE]
>
> Currently only `GET` requests are supported. When I was first developing this project I didn't realise how big it would get or how much interest there would be. As such I didn't put enough consideration into the overall project architecture and structure. Therefore I'm going to attempt a very significant rewrite of almost all the project's code. Once this is complete I'll start working on supporting `POST` requests to modify things like settings and calibration values.

## `GET /api/values`

When the Wi-Fi hotspot is active scanning runs continuously in the background to update the internal list of RSSI values. When a `GET` request is sent to this endpoint it returns the most recent RSSI values in the following format:

```json
{
  "values": [
    606,
    609,
    596,
    587,
    576,
    598,
    601,
    610,
    606,
    618,
    642,
    643,
    647,
    638,
    640,
    647
  ]
}
```

> [!NOTE]
>
> The number of returned values will change with the scanning interval. A smaller scanning interval will result in more values. Each value will be between 0 and 4095 inclusive.

> [!IMPORTANT]
>
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. It is useful to also request the calibrated minimum and maximum values from [here](#get-apicalibration) to use as a reference point for these. A further explanation of the internal signal strength calculation is explained [here](README.md#rssi-calibration).

## `GET /api/battery`



## `GET /api/settings`



## `GET /api/calibration`

