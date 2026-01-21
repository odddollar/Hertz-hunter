# USB Serial

Hertz Hunter allows for USB serial communication for the purpose of connecting the device to other software, such as the [official client](https://github.com/odddollar/Hertz-hunter-usb-client). The required schema for interacting with this feature is documented here, and it includes the following features:

- Requesting up-to-date RSSI data
- Switching between high and low band scanning
- Requesting the current settings for the scan interval, buzzer state, and low battery alarm
- Updating the current settings for the scan interval, buzzer state, and low battery alarm
- Requesting the calibrated minimum and maximum signal strength values
- Setting the calibrated minimum and maximum signal strength values
- Requesting the current battery voltage

## Framing

Unlike Hertz Hunter's API, the USB serial feature doesn't have a webserver to handle requests, therefore a command framing schema has been developed using an HTTP JSON-inspired format, shown below:

```json
{
    "event": "",
    "location": "",
    "payload": {}
}
```

Each frame/command sent to the device must consist of three keys:

- `event` - Either `get` or `post` for getting/sending data from/to the device
  - A third value, `error` is used when the device sends an error message back to the client
- `location` - Either `values`, `settings`, `calibration`, or `battery` for denoting which endpoint to use
- `payload` - Contains the data being sent to the device when using the `post` event
  - Must be an empty object (`{}`) when using the `get` event

> [!IMPORTANT]
>
> The location endpoint `battery` is only available if `BATTERY_MONITORING` is defined in `battery.h`. See [here](SOFTWARE.md#5-if-necessary-disable-battery-monitoring) for more information.

The schema required for the endpoints matches that used by the [API](API.md). All endpoint-specific schemas shown in this document must/will be contained within the frame's `payload` key.

### Responses

Responses to a command will follow the same framing schema, with `event` and `location` mimicking that of the request. For example, a request to set the device to scan on the low-band frequencies would look like this:

```json
{
    "event":"post",
    "location":"values",
    "payload":{
        "lowband":true
    }
}
```

The response would be:

```json
{
    "event":"post",
    "location":"values",
    "payload":{
        "status":"ok"
    }
}
```

The data returned from a `get` event will be contained in the `payload` key.

### Errors

Errors are reported with the `event` key set to `error`. The `location` key will contain the endpoint that returned the error, with `payload` containing the error message. An example is shown below where a setting is requested to be set to an invalid value:

```json
{
    "event":"post",
    "location":"settings",
    "payload":{
        "scan_interval_index":4
    }
}
```

The response would be:

```json
{
    "event":"error",
    "location":"settings",
    "payload":{
        "status":"'scan_interval_index' must be between 0 and 2 inclusive"
    }
}
```

In the case of an error with the framing itself, the `location` key will be left blank.

## `{"event":"get","location":"values"}`

When USB serial is active, scanning runs continuously in the background to update the internal list of signal strength values. When a `get` event is sent to this endpoint it returns the most recent values in the following format:

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
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. It is useful to also request the calibrated minimum and maximum values from `{"event":"get","location":"calibration"}` to use as a reference point for these. A further explanation of the internal signal strength calculation used on the `Scan` menu can be found [here](USAGE.md#rssi-calibration).

## `{"event":"post","location":"values"}`

Allows for switching between scanning on the normal high-band (5645MHz to 5945MHz) frequencies and scanning on low-band (5345MHz to 5645MHz). This updates the device's internal scanning state. A schema example for the request body is shown below:

```json
{
    "lowband": true
}
```

## `{"event":"get","location":"settings"}`

> [!IMPORTANT]
> 
> Battery fields are only available if `BATTERY_MONITORING` is defined in `battery.h`. See [here](SOFTWARE.md#5-if-necessary-disable-battery-monitoring) for more information.

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

In the given above example format, the indices refer to the following values:

- `Scan interval` is set to `10MHz`
- `Buzzer` is set to `Off`
- `Battery alarm` is set to `3.6v`

## `{"event":"post","location":"settings"}`

> [!IMPORTANT]
> 
> Battery fields are only available if `BATTERY_MONITORING` is defined in `battery.h`. See [here](SOFTWARE.md#5-if-necessary-disable-battery-monitoring) for more information.

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

## `{"event":"get","location":"calibration"}`

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
> These are not actual RSSI values, rather the raw analog-to-digital converter reading from the ESP32. These can be combined with values requested from `{"event":"get","location":"values"}`, similarly to the internal signal strength calculation method explained [here](USAGE.md#rssi-calibration).

## `{"event":"post","location":"calibration"}`

Allows for updating the calibrated minimum and maximum signal strength. This updates the device's internal state. A schema example for the request `payload` is shown below:

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
> Similarly to when sending a `get` event to the `values` or `calibration` locations, the values here are not actual RSSI values, rather the raw analog-to-digital converter reading on the ESP32.

## `{"event":"get","location":"battery"}`

> [!IMPORTANT]
>
> This endpoint is only available if `BATTERY_MONITORING` is defined in `battery.h`. See [here](SOFTWARE.md#5-if-necessary-disable-battery-monitoring) for more information.

Returns the current measured battery voltage in the following format:

```json
{
    "voltage": 37
}
```

Divide the number by 10 for the decimal voltage value.

For more information about properly calibrating this value, refer to [here](SOFTWARE.md#battery-calibration).

