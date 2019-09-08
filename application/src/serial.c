#include <device.h>
#include <logging/log.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "serial.h"
#include "types.h"

#define PRIORITY -3
#define STACKSIZE KB(2)

LOG_MODULE_REGISTER(Serial, LOG_LEVEL_DBG);

extern struct k_sem wired_activity_alert;
// extern struct k_mem_slab package_buffer_slab;
extern struct k_fifo serial2wireless_fifo;
extern struct k_fifo wireless2serial_fifo;

void serial_thread_function(void* arg0, void* arg1, void* arg2);

K_THREAD_DEFINE(serial_thread,
                STACKSIZE,
                serial_thread_function,
                NULL,
                NULL,
                NULL,
                PRIORITY,
                0,
                K_FOREVER);

void serial_callback(struct device* device)
{
	k_sem_give(&wired_activity_alert);

	uart_irq_update(device);

	if (uart_irq_rx_ready(device)) {
		do {
			static u8_t buffer[CONFIG_NRF_ESB_MAX_PAYLOAD_LENGTH];

			u32_t received = uart_fifo_read(device, buffer, sizeof(buffer));
			__ASSERT(received >= 0, "uart_fifo_read (%d)", received);

			if (received <= 0) {
				break;
			}

			static struct message_t* message;

			// int err = k_mem_slab_alloc(&package_buffer_slab, (void**)&message, K_NO_WAIT);
			// __ASSERT(err == 0, "k_mem_slab_alloc (%d)", err);

			// if (err) {
			// 	LOG_WRN("No memory");
			// 	break;
			// }

			memcpy(message->data, buffer, received);
			message->length = received;
			k_fifo_put(&serial2wireless_fifo, message);
		} while (0);
	}
}

void serial_thread_function(void* arg0, void* arg1, void* arg2)
{
	LOG_INF("Starting serial thread");

	struct device* serial_device = device_get_binding(DT_NORDIC_NRF_USBD_VIRTUALCOM_LABEL);

	while (1) {
		struct message_t* message = k_fifo_get(&wireless2serial_fifo, K_FOREVER);
		uart_fifo_fill(serial_device, message->data, message->length);
		LOG_HEXDUMP_DBG(message->data, message->length, NULL);
		// k_mem_slab_free(&package_buffer_slab, (void**)&message);
	}
}

void serial_init(void)
{
	struct device* serial_device = device_get_binding(DT_NORDIC_NRF_USBD_VIRTUALCOM_LABEL);

	uart_irq_callback_set(serial_device, serial_callback);
	uart_irq_rx_enable(serial_device);

	k_thread_start(serial_thread);
}
