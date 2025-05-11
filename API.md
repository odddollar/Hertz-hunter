# API Documentation for Hertz Hunter

> [!NOTE]
>
> Currently only `GET` requests are supported. When I was first developing this project I didn't realise how big it would get or how much interest there would be. As such I didn't put enough consideration into the overall project architecture or structure. Therefore I'm going to attempt a very significant rewrite of almost all the project's code. Once this is complete I'll start working on supporting `POST` requests to modify things like settings and calibration values.

> [!IMPORTANT]
>
> This API is not fixed and is subject to change, especially during an almost full codebase rewrite. I will do my best to keep the format the same, but can make no guarantees.

Hertz Hunter provides an API accessible from a Wi-Fi hotspot for the purpose of integrating the device with other software. The required format for interacting with this API is documented here, and includes the following features:

- Requesting up-to-date RSSI data
- Requesting the current battery voltage
- Requesting the current settings for the scan interval, buzzer state, and low battery alarm
- Requesting the calibrated minimum and maximum signal strength values

## `GET /api/values`

When the Wi-Fi hotspot is active scanning runs continuously in the background to update the internal list of signal strength values. When a `GET` request is sent to this endpoint it returns the most recent values in the following format:

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

Returns the current measured battery voltage in the following format:

```json
{
  "voltage": 3.7
}
```

For more information about properly calibrating this value, refer to [here](README.md#battery-calibration).

## `GET /api/settings`

Returns the current setting indices for `Scan interval`, `Buzzer`, and `Battery alarm` in the following format:

```json
{
  "scan_interval": 2,
  "buzzer": 1,
  "battery_alarm": 0
}
```

These indices refer to the list of possible values for each setting, displayed below:

- `Scan interval` possible settings `{ 5, 10, 20 }`
- `Buzzer` possible settings `{ On, Off }`
- `Battery alarm` possible settings `{ 3.6, 3.3, 3.0 }`

In the given example format, the indices refer to the following values:

- `Scan interval` is set to `20MHz`
- `Buzzer` is set to `Off`
- `Battery alarm` is set to `3.6v`

## `GET /api/calibration`

Returns the calibrated minimum and maximum signal strength in the following format:

```json
{
  "low": 619,
  "high": 1572
}
```

> [!NOTE]
> 
> When the device hasn't been calibrated, `low` will be `0`, and `high` will be `4095`.

> [!IMPORTANT]
>
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. These can be combined with values requested from `GET /api/values`, similarly to the internal signal strength calculation method explained [here](README.md#rssi-calibration).