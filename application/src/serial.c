#include <device.h>
#include <logging/log.h>
#include <nrf_esb.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <zephyr/types.h>

#define PRIORITY 3
#define STACKSIZE KB(2)

LOG_MODULE_REGISTER(Serial, LOG_LEVEL_DBG);

int err = 0;
extern struct k_sem led_ind_wired;

extern struct k_msgq serial_frame_q;
extern struct k_msgq esb_frame_q;

void serial_thread_function(void *arg0, void *arg1, void *arg2);

K_THREAD_DEFINE(serial_thread,
                STACKSIZE,
                serial_thread_function,
                NULL,
                NULL,
                NULL,
                PRIORITY,
                0,
                K_FOREVER);

void serial_callback(struct device *device)
{
	k_sem_give(&led_ind_wired);

	uart_irq_update(device);

	if (uart_irq_rx_ready(device)) {
		do {
			static u8_t buffer[CONFIG_NRF_ESB_MAX_PAYLOAD_LENGTH];

			u32_t received = uart_fifo_read(device, buffer, sizeof(buffer));
			__ASSERT(received >= 0, "uart_fifo_read (%d)", received);

			if (received <= 0) {
				break;
			}

			struct nrf_esb_payload payload;

			memcpy(payload.data, buffer, received);
			payload.length = received;
			payload.pipe = 0;
			err = k_msgq_put(&serial_frame_q, &payload, K_NO_WAIT); // int err = k_mem_slab_alloc(&package_buffer_slab, (void**)&message, K_NO_WAIT);

			if (err) {
				LOG_ERR("Error during serial_queue put"); /* code */
			}

			// __ASSERT(err == 0, "k_mem_slab_alloc (%d)", err);

			// if (err) {
			// 	LOG_WRN("No memory");
			// 	break;
			// }

			//memcpy(message->data, buffer, received);
			//message->length = received;

		} while (0);
	}
}

void serial_thread_function(void *arg0, void *arg1, void *arg2)
{
	LOG_DBG("Starting serial thread: %p", k_current_get());

	struct device *serial_device = device_get_binding("CDC_ACM_0");

	uart_irq_callback_set(serial_device, serial_callback);
	uart_irq_rx_enable(serial_device);

	while (1) {
		struct nrf_esb_payload payload;
		k_msgq_get(&esb_frame_q, &payload, K_FOREVER);
		uart_fifo_fill(serial_device, payload.data, payload.length);
		LOG_HEXDUMP_DBG(payload.data, payload.length, NULL);
	}
}
