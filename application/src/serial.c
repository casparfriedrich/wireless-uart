#include <device.h>
#include <logging/log.h>
#include <nrf_esb.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <zephyr/types.h>

#define PRIORITY 3
#define STACKSIZE KB(10)

LOG_MODULE_REGISTER(Serial, LOG_LEVEL_DBG);

extern struct k_sem led_ind_wired;

extern struct k_msgq serial_frame_q;
extern struct k_msgq esb_frame_q;

void serial_thread_fn(void *arg0, void *arg1, void *arg2);

K_THREAD_DEFINE(serial_thread, STACKSIZE, serial_thread_fn, NULL, NULL, NULL, PRIORITY, 0, K_FOREVER);

void serial_callback(struct device *device)
{
	int err = 0;

	k_sem_give(&led_ind_wired);
	uart_irq_update(device);

	if (uart_irq_rx_ready(device)) {
		static u8_t buffer[CONFIG_NRF_ESB_MAX_PAYLOAD_LENGTH];

		u32_t received = uart_fifo_read(device, buffer, sizeof(buffer));

		if (received == 0) {
			return;
		}

		if (received < 0) {
			LOG_ERR("error during uart_fifo_read");
		}

		struct nrf_esb_payload payload;
		memcpy(payload.data, buffer, received);
		payload.length = received;
		payload.pipe = 0;

		err = k_msgq_put(&serial_frame_q, &payload, K_NO_WAIT);
		if (err) {
			LOG_ERR("Error during serial queue put");
		}
	}
}

void serial_thread_fn(void *arg0, void *arg1, void *arg2)
{
	LOG_DBG("Starting serial thread: %p", k_current_get());

	struct device *serial_device = device_get_binding("CDC_ACM_0");

	uart_irq_callback_set(serial_device, serial_callback);
	uart_irq_rx_enable(serial_device);

	while (1) {
		static struct nrf_esb_payload payload;
		k_msgq_get(&esb_frame_q, &payload, K_FOREVER);
		uart_fifo_fill(serial_device, payload.data, payload.length);
	}
}
