# Black Magic Probe / DapLink for ESP32-S2

WiFi/USB capable version of the famous BlackMagicProbe (or DapLink) debugger.

# Clone the Repository

Clone the repository with:
```shell
git clone --recursive https://github.com/flipperdevices/blackmagic-esp32-s2.git
```

You must recursively clone the submodules, which `--recursive` will do for you. When you pull new changes, be sure to run `git submodule update --init --recursive`.

# Develop with ESP-IDF

Set up __ESP-IDF v4.4__ following [Espressif's instructions](https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/get-started/index.html). Make sure `idf.py` is in your PATH and you can successfully run the program.

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

## Web interface development

Web interface is located in `components/svelte-portal` and written in Svelte. To build it, you need to install Node.js and run `npm install` in `components/svelte-portal` directory. Then you can run `npm run dev` to start development server or `npm run build` to build production version.

Typical workflow is to fix the board's IP address in `components/svelte-portal/src/lib/Api.svelte` and then run `npm run dev`. After that, you can open `http://localhost:5000` in your browser and see changes in the web interface in real time with live reload.

If you want to change local ip or port, you need to run `export HOST={ip} PORT={port}` before `npm run dev`. 

```shell
export HOST=127.0.0.1 PORT=3000
npm run dev
```

When you're done, you need to run `npm run build`, `idf.py build` and then `idf.py -p <port> flash`. You can then open `http://blackmagic.local` in your browser and see the changes in the web interface.
```shell
npm run build
idf.py build
idf.py -p <port> flash
```


## Schematic

[Flipper Zero Wi-Fi Module Schematic](https://cdn.flipperzero.one/Flipper_Zero_WI-FI_Module_V1_Schematic.PDF)
