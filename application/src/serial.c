#include <device.h>
#include <logging/log.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <zephyr/types.h>
#include <pwm.h>

#include "serial.h"
#include "types.h"

#define PRIORITY -3
#define STACKSIZE KB(2)

#if defined(CONFIG_PWM_NRF5_SW)
#define PWM_DRIVER CONFIG_PWM_NRF5_SW_0_DEV_NAME
#else
#define PWM_DRIVER DT_NORDIC_NRF_PWM_PWM_0_LABEL
#endif  /* CONFIG_PWM_NRF5_SW */
#define PWM_CHANNEL LED1_GPIO_PIN

#define PERIOD (USEC_PER_SEC / 50U)
/* in micro second */
#define FADESTEP	9000

LOG_MODULE_REGISTER(Serial, LOG_LEVEL_DBG);

extern struct k_sem wired_activity_alert;
extern struct k_mem_slab package_buffer_slab;
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

			int err = k_mem_slab_alloc(&package_buffer_slab, (void**)&message, K_NO_WAIT);
			__ASSERT(err == 0, "k_mem_slab_alloc (%d)", err);

			if (err) {
				LOG_WRN("No memory");
				break;
			}

			memcpy(message->data, buffer, received);
			message->length = received;
			k_fifo_put(&serial2wireless_fifo, message);
		} while (0);
	}
}

void serial_thread_function(void* arg0, void* arg1, void* arg2)
{
	LOG_INF("Starting pwm thread");

	//struct device* serial_device = device_get_binding(DT_NORDIC);
	struct device *pwm_dev;
	u32_t pulse_width = 0U;
	u8_t dir = 0U;

	pwm_dev = device_get_binding(PWM_DRIVER);
	
	while (1) {
		//struct message_t* message = k_fifo_get(&wireless2serial_fifo, K_FOREVER);
		//uart_fifo_fill(serial_device, message->data, message->length);
		//LOG_HEXDUMP_DBG(message->data, message->length, NULL);
		//LOG_DBG("haha");
		//k_mem_slab_free(&package_buffer_slab, (void**)&message);
		if (pwm_pin_set_usec(pwm_dev, PWM_CHANNEL,
					PERIOD, pulse_width)) {
			printk("pwm pin set fails\n");
			return;
		}

		if (dir) {
			if (pulse_width < FADESTEP) {
				dir = 0U;
				pulse_width = 0U;
			} else {
				pulse_width -= FADESTEP;
			}
		} else {
			pulse_width += FADESTEP;

			if (pulse_width >= PERIOD) {
				dir = 1U;
				pulse_width = PERIOD;
			}
		}
	}

}

void serial_init(void)
{
	struct device *pwm_dev; 
	pwm_dev = device_get_binding(PWM_DRIVER);
	if (!pwm_dev) {
		printk("Cannot find %s!\n", PWM_DRIVER);
		return;
	}
	//	k_sleep(MSEC_PER_SEC);
	//struct device* serial_device = device_get_binding(DT_NORDIC_NRF_USBD_VIRTUALCOM_LABEL);
	//uart_irq_callback_set(serial_device, serial_callback);
	//uart_irq_rx_enable(serial_device);
	k_thread_start(serial_thread);
}
