# Black Magic Probe for ESP32-S2

WiFi/USB capable version of the famous Black Magic Probe debugger.

# Clone the Repository

Clone the repository with:
```shell
git clone --recursive https://github.com/flipperdevices/blackmagic-esp32-s2.git
```

You must recursively clone the submodules, which `--recursive` will do for you. When you pull new changes, be sure to run `git submodule update --init --recursive`.

# Develop with ESP-IDF

Set up ESP-IDF following [Espressif's instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html). Make sure `idf.py` is in your PATH and you can successfully run the program.

## Build with ESP-IDF

In your clone of this repository, run:
```shell
idf.py build
```
> Note: the `sdkconfig` file already configures the project to target the ESP32-S2. You don't need to run `idf.py set-target esp32s2`, which will overwrite some of the necessary settings in `sdkconfig`.

## Flash with ESP-IDF

Connect the dev board to your computer with a USB cable. Find the name of the serial port device on your computer that corresponds to the dev board. See the [Flipper Zero guide] on how to connect to the dev board over USB or [Espressif's guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html) on how to find the port name.

Run:
```shell
idf.py -p <port> flash
```

## Test with ESP-IDF

Connect to the dev board with:
```shell
idf.py -p <port> monitor
```

You should not see errors in the logs if the firmware is installed and running correctly. 

## Schematic

[Altium Live Schematic](https://kernel-viewer-cdn1.365.altium.com/71/client/index.html?feature=embed&source=6B62619E-E72F-44F7-BA3B-299EEE73CB9E&activeView=SCH)
