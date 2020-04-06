cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project("Wireless UART")

target_sources(app
	PRIVATE
	src/main.c
	src/esb.c
	src/serial.c
	src/led.c)
