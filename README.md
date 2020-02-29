# Wireless UART

## Installing the nRF Connect SDK

https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/

```bash
west init -m https://github.com/NordicPlayground/fw-nrfconnect-nrf
west update
```

## Usage

### with cmake
To spin up a new project:

```bash
source zephyr/zephyr-env.sh
cmake -G Ninja -D BOARD=<BOARD> -B <BUILD_FOLDER> -S application
```
Build the project:
```bash
cmake --build <BUILD_FOLDER> [--target <MAKE_TARGET>]
```
### with west
To spin up a new project:

```bash
source zephyr/zephyr-env.sh
west build -b <BOARD> -d <BUILD_FOLDER> application
```
Build the project:
```bash
cd <BUILD_FOLDER>
west build 
```
## Compatible boards/board strings

reel_board

## Roadmap/ToDos Wireless UART

### Concept

Connection speed as fast as possible, at least capable of handling one linux console.
Handling multiple uart endpoints.

- [ ] Config option for multiple UART endpoints
	* Config protocol
	* Config distribution to MASTER to UART client
		* Baudrate
		* RTS, CTS
		* Start Stop bit
	* Question: Safe config on clients or distribute config before usage?
- [ ] Stable Wireless to UART connection and vice versa
