#include <device.h>
#include <drivers/uart.h>
#include <esb.h>
#include <logging/log.h>
#include <stdio.h>
#include <string.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "led.h"

#define PRIORITY 3
#define STACKSIZE KB(10)
#define FRAME_TIME K_MSEC(10)
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

extern struct k_msgq esb_frame_q;
extern struct k_msgq serial_frame_q;

static int tx_frame_counter = 0;

static void esb_thread_fn(void *arg0, void *arg1, void *arg2);
static void frame_timer_expiry_fn(struct k_timer *timer);

LOG_MODULE_REGISTER(ESB, LOG_LEVEL_DBG);

K_THREAD_DEFINE(esb_thread, STACKSIZE, esb_thread_fn, NULL, NULL, NULL, PRIORITY, 0, K_FOREVER);
K_TIMER_DEFINE(frame_timer, frame_timer_expiry_fn, frame_timer_expiry_fn);

void frame_timer_expiry_fn(struct k_timer *timer)
{
	int err = 0;

	if (esb_is_idle()) {
		err = esb_start_rx();
		if (err) {
			LOG_ERR("error starting RX mode");
		}
	} else if (esb_is_rx() && (tx_frame_counter > 0)) {
		err = esb_stop_rx();
		if (err) {
			LOG_ERR("error stopping RX mode: %d", err);
		}

		err = esb_start_tx();
		if (err) {
			LOG_ERR("error starting TX mode: %d", err);
		}
	}
}

void esb_event_callback(struct esb_evt const *event)
{
	int err = 0;
	struct esb_payload payload;

	led_flash(LED_BLUE);

	switch (event->evt_id) {
	case ESB_EVENT_TX_SUCCESS:
		LOG_DBG("ESB_EVENT_TX_SUCCESS");
		tx_frame_counter--;
		break;

	case ESB_EVENT_TX_FAILED:
		LOG_DBG("ESB_EVENT_TX_FAILED");
		err = esb_pop_tx();
		if (err) {
			LOG_ERR("error removing package");
		}
		tx_frame_counter--;
		break;

	case ESB_EVENT_RX_RECEIVED:
		err = esb_read_rx_payload(&payload);
		if (err) {
			LOG_ERR("error reading payload: %d", err);
			break;
		}

		err = k_msgq_put(&esb_frame_q, &payload, K_NO_WAIT);
		if (err) {
			LOG_WRN("error storing message: %d", err);
		}

		break;
	}

	k_timer_start(&frame_timer, 0, FRAME_TIME);
}

void esb_thread_fn(void *arg0, void *arg1, void *arg2)
{
	LOG_INF("Starting thread: %p", k_current_get());

	int err = 0;

	u8_t base_addr_0[4] = DEFAULT_BASE_ADDR_0;
	u8_t base_addr_1[4] = DEFAULT_BASE_ADDR_1;
	u8_t addr_prefix[8] = DEFAULT_ADDR_PREFIX;

	struct esb_config config = ESB_DEFAULT_CONFIG;
	config.event_handler = esb_event_callback;
	config.tx_mode = ESB_TXMODE_MANUAL_START;

	err = esb_init(&config);
	__ASSERT(err == 0, "esb_init: %d", err);

	err = esb_set_base_address_0(base_addr_0);
	__ASSERT(err == 0, "esb_set_base_address_0: %d", err);

	err = esb_set_base_address_1(base_addr_1);
	__ASSERT(err == 0, "esb_set_base_address_1: %d", err);

	err = esb_set_prefixes(addr_prefix, ARRAY_SIZE(addr_prefix));
	__ASSERT(err == 0, "esb_set_prefixes: %d", err);

	err = esb_start_rx();
	__ASSERT(err == 0, "esb_start_rx: %d", err);

	k_timer_start(&frame_timer, 0, FRAME_TIME);

	while (1) {
		static struct esb_payload payload = ESB_CREATE_PAYLOAD(0);

		if (tx_frame_counter >= CONFIG_ESB_TX_FIFO_SIZE) {
			k_sleep(FRAME_TIME);
			continue;
		}

		err = k_msgq_get(&serial_frame_q, &payload, K_FOREVER);
		if (err) {
			LOG_WRN("k_msgq_get: %d", err);
			continue;
		}

		err = esb_write_payload(&payload);
		if (err) {
			LOG_ERR("esb_write_payload: %d", err);
			continue;
		}

		tx_frame_counter++;
	}
}
