# SerialTempSens Firmware
![build](https://github.com/oschmidh/SerialTempSens-FW/actions/workflows/build.yml/badge.svg?branch=main)

This is the repo for the FW of the SerialTempSens Project.
It implements a Serial interface to enable a connected device (e.g. PC) to read temperatures from various sensors.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``SerialTempSens-FW`` and all Zephyr modules will be cloned. Run the following
command:

```shell
# initialize my-workspace for the example-application (main branch)
west init -m https://github.com/oschmidh/SerialTempSens-FW --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

### Building and running

To build the application, run the following command:

```shell
west build -b $BOARD app
```

where `$BOARD` is the target board.

Currently, `serial_temp_sens` and `nucleo_l432kc` are the only supported boards.

A sample debug configuration is also provided. To apply it, run the following
command:

```shell
west build -b $BOARD app -- -DOVERLAY_CONFIG="debug.conf"
```

Once you have built the application, run the following command to flash it:

```shell
west flash
```

### Testing

To execute Twister integration tests, run the following command:

```shell
west twister -T tests --integration
```
