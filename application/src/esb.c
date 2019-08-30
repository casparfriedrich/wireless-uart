#include <device.h>
#include <logging/log.h>
#include <nrf_esb.h>
#include <nrf_pwm.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <pwm.h>

#if defined(CONFIG_PWM_NRF5_SW)
#define PWM_DRIVER CONFIG_PWM_NRF5_SW_0_DEV_NAME
#else
#define PWM_DRIVER DT_NORDIC_NRF_PWM_PWM_0_LABEL
#endif  /* CONFIG_PWM_NRF5_SW */
#define PWM_CHANNEL LED2_GPIO_PIN


/*
 * 50 is flicker fusion threshold. Modulated light will be perceived
 * as steady by our eyes when blinking rate is at least 50.
 */
#define PERIOD (USEC_PER_SEC / 50U)

/* in micro second */
#define FADESTEP	2000



#define PRIORITY 3
#define STACKSIZE KB(2)
#define FRAME_LENGTH K_MSEC(50)
#define DEFAULT_BASE_ADDR_0            \
	{                              \
		0xE7, 0xE7, 0xE7, 0xE7 \
	}
#define DEFAULT_BASE_ADDR_1            \
	{                              \
		0xC2, 0xC2, 0xC2, 0xC2 \
	}
#define DEFAULT_ADDR_PREFIX                                    \
	{                                                      \
		0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 \
	}

extern struct k_sem led_ind_wireless;

static void esb_thread_fn(void* arg0, void* arg1, void* arg2);
static void my_tx_timer_expiry_fn(struct k_timer* timer);

static s64_t time_stamp = 0;

LOG_MODULE_REGISTER(ESB, LOG_LEVEL_DBG);

K_MSGQ_DEFINE(my_msgq, sizeof(struct nrf_esb_payload), 10, 4);
K_THREAD_DEFINE(esb_thread,
                STACKSIZE,
                esb_thread_fn,
                NULL,
                NULL,
                NULL,
                PRIORITY,
                0,
                K_FOREVER);
K_TIMER_DEFINE(my_tx_timer, my_tx_timer_expiry_fn, NULL);



void my_tx_timer_expiry_fn(struct k_timer* timer)
{
	//int __unused err = 0;

	//if (!nrf_esb_is_idle()) {
	//	return;
	//}

	//err = nrf_esb_start_rx();
	//if (err) {
	//	LOG_ERR("error starting RX mode: %d", err);
	//	return;
	//}

	k_timer_start(&my_tx_timer, FRAME_LENGTH, 0);
}

void esb_event_callback(struct nrf_esb_evt const* event)
{
	int __unused err = 0;
	struct nrf_esb_payload payload;

	time_stamp = k_uptime_get();

	k_sem_give(&led_ind_wireless);

	switch (event->evt_id) {
	case NRF_ESB_EVENT_TX_SUCCESS:
		LOG_DBG("NRF_ESB_EVENT_TX_SUCCESS");

		break;

	case NRF_ESB_EVENT_TX_FAILED:
		LOG_DBG("NRF_ESB_EVENT_TX_FAILED");

		err = nrf_esb_pop_tx();
		if (err) {
			LOG_ERR("error removing failed package: %d", err);
		}

		break;

	case NRF_ESB_EVENT_RX_RECEIVED:
		LOG_DBG("NRF_ESB_EVENT_RX_RECEIVED");

		err = nrf_esb_read_rx_payload(&payload);
		if (err) {
			LOG_ERR("error reading payload: %d", err);
			break;
		}

		LOG_DBG("counter: %u", payload.data[0]);

		// err = k_msgq_put(&my_msgq, &payload, K_NO_WAIT);
		// __ASSERT(err == 0, "k_msgq_put: %d", err);
		// if (err) {
		// 	LOG_WRN("k_msgq_put: %d", err);
		// }

		break;
	}
}

void esb_thread_fn(void* arg0, void* arg1, void* arg2)
{
	int __unused err = 0;


//
//	u8_t base_addr_0[4] = DEFAULT_BASE_ADDR_0;
//	u8_t base_addr_1[4] = DEFAULT_BASE_ADDR_1;
//	u8_t addr_prefix[8] = DEFAULT_ADDR_PREFIX;
//
//	struct nrf_esb_config config = NRF_ESB_DEFAULT_CONFIG;
//	config.event_handler = esb_event_callback;
//	config.tx_mode = NRF_ESB_TXMODE_MANUAL_START;
//
//	err = nrf_esb_init(&config);
//	__ASSERT(err == 0, "nrf_esb_init: %d", err);
//
//	err = nrf_esb_set_base_address_0(base_addr_0);
//	__ASSERT(err == 0, "nrf_esb_set_base_address_0: %d", err);
//
//	err = nrf_esb_set_base_address_1(base_addr_1);
//	__ASSERT(err == 0, "nrf_esb_set_base_address_1: %d", err);
//
//	err = nrf_esb_set_prefixes(addr_prefix, ARRAY_SIZE(addr_prefix));
//	__ASSERT(err == 0, "nrf_esb_set_prefixes: %d", err);
//
//	err = nrf_esb_start_rx();
//	__ASSERT(err == 0, "nrf_esb_start_rx: %d", err);
//
	k_timer_start(&my_tx_timer, FRAME_LENGTH, 0);

// struct device *pwm_dev;
// 	u32_t pulse_width = 0U;
// 	u8_t dir = 0U;

// 	printk("PWM demo app-fade LED\n");

// 	pwm_dev = device_get_binding(PWM_DRIVER);
// 	if (!pwm_dev) {
// 		printk("Cannot find %s!\n", PWM_DRIVER);
// 		return;
// 	}



	while (1) {

	// struct device *dev;

	// dev = device_get_binding(LED_PORT);
	// if (dev == 0) {
	// 	printk("Nordic nRF GPIO driver was not found!\n");
	// 	return 1;
	// }

	// gpio_pin_configure(dev, LED4_GPIO_PIN, GPIO_DIR_OUT);
	// gpio_pin_write(dev, LED4_GPIO_PIN, 8);
	// 	if (pwm_pin_set_usec(pwm_dev, PWM_CHANNEL,
	// 				PERIOD, pulse_width)) {
	// 		printk("pwm pin set fails\n");
	// 		return;
	// 	}

	// 	if (dir) {
	// 		if (pulse_width < FADESTEP) {
	// 			dir = 0U;
	// 			pulse_width = 0U;
	// 		} else {
	// 			pulse_width -= FADESTEP;
	// 		}
	// 	} else {
	// 		pulse_width += FADESTEP;

	// 		if (pulse_width >= PERIOD) {
	// 			dir = 1U;
	// 			pulse_width = PERIOD;
	// 		}
	// 	}
		k_sleep(10);



//		struct nrf_esb_payload payload = NRF_ESB_CREATE_PAYLOAD(0, counter++);

		// err = k_msgq_get(&my_msgq, &payload1, K_FOREVER);
		// if (err) {
		// 	LOG_WRN("k_msgq_get: %d", err);
		// 	continue;
		// }

		// err = nrf_esb_write_payload(&payload1);
		// if (err) {
		// 	LOG_ERR("nrf_esb_write_payload1: %d", err);
		// }

		//err = nrf_esb_write_payload(&payload);
		//if (err) {
		//	LOG_ERR("error writing payload for transmission: %d", err);
		//}

		//if (k_uptime_delta(&time_stamp) < FRAME_LENGTH) {
		//	continue;
		//}

		//err = nrf_esb_stop_rx();
		//if (err) {
		//	LOG_ERR("error stopping RX mode: %d", err);
		//	continue;
		//}

		//err = nrf_esb_start_tx();
		//if (err) {
		//	LOG_ERR("error starting TX mode: %d", err);
		//	continue;
		//}

		
	}
}
