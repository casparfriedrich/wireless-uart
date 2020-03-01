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
mkdir $NAME_OF_YOUR_BUILDFOLDER
cd $NAME_OF_YOUR_BUILDFOLDER
cmake -G Ninja -D BOARD=$BOARD_NAME $PATH_TO_THE_prj.conf
```

## Compadible boards/board strings

reel_board

## Roadmap/ToDos Wireless UART

### Concept

Connection speed as fast as possible, at least capable of handling one linux console.
Handling multiple uart endpoints.

- [ ] Config option for multiple UART endpoints
	* Congfig protocol
	* Config distribution to MASTER to UART client
		* Baudrate
		* RTS, CTS
		* Start Stop bit
	* Question: Safe config or distribute config before usage?
- [ ] Stable Wireless to UART connection and vica versa

