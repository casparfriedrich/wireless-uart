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
extern struct k_mem_slab package_buffer_slab;
extern struct k_fifo serial2wireless_fifo;
extern struct k_fifo wireless2serial_fifo;

void serial_thread_function(void* arg0, void* arg1, void* arg2);

K_THREAD_DEFINE(
	serial_thread,
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
	uart_irq_update(device);

	k_sem_give(&wired_activity_alert);

	if (uart_irq_rx_ready(device))
	{
		while (1)
		{
			static u8_t buffer[32];

			u32_t received = uart_fifo_read(device, buffer, 32);
			__ASSERT(received >= 0, "uart_fifo_read (%d)", received);

			if (received <= 0)
			{
				break;
			}

			static struct message_t* message;

			int err = k_mem_slab_alloc(&package_buffer_slab, (void**)&message, K_NO_WAIT);
			__ASSERT(err == 0, "k_mem_slab_alloc (%d)", err);

			if (err)
			{
				break;
			}

			memcpy(message->data, buffer, received);
			message->length = received;
			k_fifo_put(&serial2wireless_fifo, message);
		}
	}
}

void serial_thread_function(void* arg0, void* arg1, void* arg2)
{
	LOG_INF("Starting serial thread");

	struct device* serial_device = device_get_binding(DT_NORDIC_NRF_USBD_VIRTUALCOM_LABEL);

	while (1)
	{
		struct message_t* message = k_fifo_get(&wireless2serial_fifo, K_FOREVER);
		uart_fifo_fill(serial_device, message->data, message->length);
		k_mem_slab_free(&package_buffer_slab, (void**)&message);
	}
}

void serial_init(void)
{
	struct device* serial_device = device_get_binding(DT_NORDIC_NRF_USBD_VIRTUALCOM_LABEL);

	uart_irq_callback_set(serial_device, serial_callback);
	uart_irq_rx_enable(serial_device);

	k_thread_start(serial_thread);
}
