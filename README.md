# Wireless UART

## Installing the nRF Connect SDK

https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/

```bash
west init -m https://github.com/NordicPlayground/fw-nrfconnect-nrf
west update
```

## Usage

To spin up a new project: 

```bash
source zephyr/zephyr-env.sh
mkdir $NAME_OF_YOU_RBUILDFOLDER
cd $NAME_OF_YOUR_BUILDFOLDER
cmake -G Ninja -D BOARD=$BOARD_NAME $PATH_TO_THE_prj.conf
```

## Compadible board/ board strings

reel_board

