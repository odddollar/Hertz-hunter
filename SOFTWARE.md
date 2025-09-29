# Software

## Environment setup

### 1. Install the Arduino IDE

Download the [Arduino IDE](https://www.arduino.cc/) and install it.

### 2. Update `Additional boards manager URLs`

The ESP32 board used in this project isn't supported out-of-the-box by the Arduino IDE, so it needs to be added manually.

In the Arduino IDE, open `File > Preferences`, and in the `Additional boards manager URLs` field, paste the following, then click `OK`:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

This will update the list that the Arduino IDE checks to know where to install additional boards from, but doesn't actually install the board. 

### 3. Install ESP32 support

Go to `Tools > Board > Boards Manager` and search for `ESP32`. Install the one by `Espressif Systems`.

<div align="center">
    <img src="./images/Board installation.png" alt="Board installation" />
</div>


### 4. Install required libraries

Go to `Tools > Manage Libraries`, then search for and install the following libraries:

- `U8G2` by `oliver <olikraus@gmail.com>`
- `ESP Async WebServer` by `ESP32Async`
- `Async TCP` by `ESP32Async`
- `ArduinoJson` by `Benoit Blanchon <blog.benoitblanchon.fr>`

<div align="center">
    <img src="./images/U8g2.png" alt="U8g2" />
</div>


## Firmware setup

### 1. Download firmware

On GitHub, under `Releases`, click the most recent version.

<div align="center">
    <img src="./images/Releases.png" alt="Releases" />
</div>


Under `Assets`, click the `Source code (zip)` link to download the firmware.

<div align="center">
    <img src="./images/Assets.png" alt="Assets" />
</div>


### 2. Open firmware in Arduino IDE

Unzip the downloaded file. You should see the project files within. Open the folder named `main`, which contains all the source code files for the firmware.

<div align="center">
    <img src="./images/Files.png" alt="Files" />
</div>


Double click `main.ino`, which should open in the Arduino IDE, along with the rest of the source code files.

<div align="center">
    <img src="./images/IDE.png" alt="IDE" width="80%" />
</div>


### 3. (If necessary) Change display chip being used

> [!IMPORTANT]
>
> This step is only necessary if using an OLED with the SSD1306 chip. OLEDs that use the SH1106 chip require no modification to the firmware.

As far as I can tell, most 0.96" I<sup>2</sup>C OLEDs use the SSD1306 chip, but I think a bigger 1.3" OLED is better for this project, which mostly seem to use the SH1106 controller. As such, the SH1106 controller is what this device has been developed for, but with some slight modifications it should be possible to use SSD1306 displays.

> [!NOTE]
>
> I haven't personally tested this. All the 1.3" OLEDs I've used have SH1106 chips.

Open `menu.h` and find the following line:

```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
```

Below this line there should be:

```cpp
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
```

Add `//` to the front of the first line and remove it from the front of the second line.

### 4. (If necessary) Change SSID and password for Wi-Fi hotspot

> [!IMPORTANT]
>
> This step is only necessary if the default SSID `Hertz Hunter` and password `hertzhunter` isn't suitable for your use case. The Wi-Fi hotspot is only used for web-based interactions with the device, such as accessing the [API](API.md).

Open `api.h` and find the following lines:

```cpp
#define WIFI_SSID "Hertz Hunter"
#define WIFI_PASSWORD "hertzhunter"
```

Change these values to whatever you want, but note that text that is too long will run off the screen on the Wi-Fi menu.

## Flashing

### 1. Connect ESP32

Plug the ESP32 module into the computer with a USB-C cable.

### 2. Select board

Go to `Tools > Board > esp32` and select `ESP32C3 Dev Module`. 

### 3. Select port

Go to `Tools > Port` and select the port the ESP32 is plugged into.

<div align="center">
    <img src="./images/Port selection.png" alt="Port selection" />
</div>


### 4. Compile and upload firmware

Click the `Upload` button to compile the firmware and upload it to the ESP32.

<div align="center">
    <img src="./images/Upload.png" alt="Upload" />
</div>


> [!TIP]
>
> If you're getting errors during flashing, or the device doesn't appear, go to `Tools > USB CDC On Boot` and change it to `Enabled`. This allows the USB connection to remain active during boot, which can help with problems where the port isn't detected after the ESP32 reboots.

## Battery calibration

Different boards, even of the same model, can have variations in their analog-to-digital converters, so performing a simple calibration is necessary to ensure the device reads the correct battery voltage.

Turn the device on, and in the bottom right corner of the main menu there will be a battery voltage readout, displaying, for example, `4.0v`. Take a multimeter and measure the raw battery voltage, rounded to 1 decimal place. The voltage on the multimeter and the voltage displayed on the main menu should ideally be the same, but it may be off by a small amount.

The value of `BATTERY_VOLTAGE_OFFSET` in `battery.h` can be increased or decreased, where a change of `1` in this value corresponds to a change of `0.1` in the displayed voltage.

For example, if the main menu is displaying `3.9v`, but the multimeter says the battery is at `4.0v`, then increase the value of `BATTERY_VOLTAGE_OFFSET` by `1`. If the menu displays a voltage higher than what the multimeter reads, then decrease the offset value.

Make the necessary changes, then compile and upload the firmware again.