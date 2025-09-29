# API Documentation for Hertz Hunter

Hertz Hunter provides an API accessible from a Wi-Fi hotspot for the purpose of connecting the device to other software. The required schema for interacting with this API is documented here, and it includes the following features:

- Requesting the current battery voltage
- Requesting up-to-date RSSI data
- Switching between high and low band scanning
- Requesting the current settings for the scan interval, buzzer state, and low battery alarm
- Updating the current settings for the scan interval, buzzer state, and low battery alarm
- Requesting the calibrated minimum and maximum signal strength values
- Setting the calibrated minimum and maximum signal strength values

## `GET /api/battery`

Returns the current measured battery voltage in the following format:

```json
{
    "voltage": 37
}
```

Divide the number by 10 for the decimal voltage value.

For more information about properly calibrating this value, refer to [here](README.md#battery-calibration).

## `GET /api/values`

When the Wi-Fi hotspot is active, scanning runs continuously in the background to update the internal list of signal strength values. When a `GET` request is sent to this endpoint it returns the most recent values in the following format:

```json
{
    "lowband": false,
    "min_frequency": 5645,
    "max_frequency": 5945,
    "values": [
        635,
        639,
        645,
        652,
        662,
        650,
        640,
        628,
        647,
        609,
        611,
        603,
        600,
        603,
        602,
        595
    ]
}
```

> [!NOTE]
>
> The number of returned values will change with the scanning interval. A smaller scanning interval will result in more values. Each value will be between 0 and 4095 inclusive.

> [!IMPORTANT]
>
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. It is useful to also request the calibrated minimum and maximum values from [here](#get-apicalibration) to use as a reference point for these. A further explanation of the internal signal strength calculation used on the `Scan` menu is explained [here](README.md#rssi-calibration).

## `POST /api/values`

Allows for switching between scanning on the normal high-band (5645MHz to 5945MHz) frequencies and scanning on low-band (5345MHz to 5645MHz). This updates the device's internal scanning state. A schema example for the request body is shown below:

```json
{
    "lowband": true
}
```

## `GET /api/settings`

Returns the current indices and settings for `Scan interval`, `Buzzer`, and `Battery alarm` in the following format:

```json
{
    "scan_interval_index": 2,
    "scan_interval": 10,
    "buzzer_index": 1,
    "buzzer": false,
    "battery_alarm_index": 0,
    "battery_alarm": 36
}
```

The indices refer to the list of possible values for each setting, displayed below:

- `Scan interval` possible settings `{ 2.5MHz, 5MHz, 10MHz }`
- `Buzzer` possible settings `{ On, Off }`
- `Battery alarm` possible settings `{ 3.6v, 3.3v, 3.0v }`

In the given example format, the indices refer to the following values:

- `Scan interval` is set to `10MHz`
- `Buzzer` is set to `Off`
- `Battery alarm` is set to `3.6v`

## `POST /api/settings`

Allows for updating the settings of the device by providing the desired settings index. This updates the device's internal state. A schema example for the request body is shown below:

```json
{
    "scan_interval_index": 2,
    "buzzer_index": 1,
    "battery_alarm_index": 0,
}
```

The list of possible settings indices and their corresponding values is shown in the above section.

> [!NOTE]
>
> It is not required to have all three settings indices in each request. Below are perfectly valid requests:
>
> ```json
> {
>     "scan_interval_index": 0
> }
> ```
>
> ```json
> {
>     "battery_alarm_index": 1,
>     "buzzer_index": 1
> }
> ```

## `GET /api/calibration`

Returns the calibrated minimum and maximum signal strength in the following format:

```json
{
    "low_rssi": 619,
    "high_rssi": 1572
}
```

> [!NOTE]
> 
> When the device hasn't been calibrated, `low_rssi` will be `0`, and `high_rssi` will be `4095`.

> [!IMPORTANT]
>
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. These can be combined with values requested from `GET /api/values`, similarly to the internal signal strength calculation method explained [here](README.md#rssi-calibration).

## `POST /api/calibration`

Allows for updating the calibrated minimum and maximum signal strength. This updates the device's internal state. A schema example for the request body is shown below:

```json
{
    "low_rssi": 550,
    "high_rssi": 1600
}
```

> [!NOTE]
>
> It is not required to have both a high and low value in each request. Below is a perfectly valid request:
>
> ```json
> {
>     "high_rssi": 1450
> }
> ```

> [!IMPORTANT]
>
> Similarly to when sending a `GET` request to `/api/values` or `/api/calibration`, the values here are not actual RSSI values, rather the raw analog-to-digital converter reading on the ESP32.
