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
#define FRAME_TIME K_MSEC(20)
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

extern struct k_msgq esb_frame_q;
extern struct k_msgq serial_frame_q;

static s64_t time_stamp = 0;

static void esb_thread_fn(void *arg0, void *arg1, void *arg2);
static void frame_timer_expiry_fn(struct k_timer *timer);

LOG_MODULE_REGISTER(ESB, LOG_LEVEL_DBG);

K_THREAD_DEFINE(esb_thread, STACKSIZE, esb_thread_fn, NULL, NULL, NULL, PRIORITY, 0, K_FOREVER);
K_TIMER_DEFINE(frame_timer, frame_timer_expiry_fn, NULL);

void frame_timer_expiry_fn(struct k_timer *timer)
{
	if (nrf_esb_is_idle()) {
		if (nrf_esb_start_rx()) {
			LOG_ERR("error starting RX mode");
		} else {
			LOG_DBG("starting RX mode");
		}
	}

	k_timer_start(&frame_timer, FRAME_TIME, 0);
}

void esb_event_callback(struct nrf_esb_evt const *event)
{
	int err = 0;
	struct nrf_esb_payload payload;

	k_sem_give(&led_ind_wireless);
	time_stamp = k_uptime_get();

	switch (event->evt_id) {
	case NRF_ESB_EVENT_TX_SUCCESS:
		break;

	case NRF_ESB_EVENT_TX_FAILED:
		LOG_DBG("NRF_ESB_EVENT_TX_FAILED");
		if (nrf_esb_pop_tx()) {
			LOG_ERR("error removing package");
		}
		break;

	case NRF_ESB_EVENT_RX_RECEIVED:
		err = nrf_esb_read_rx_payload(&payload);
		if (err) {
			LOG_ERR("error reading payload: %d", err);
			break;
		}

		err = k_msgq_put(&esb_frame_q, &payload, K_NO_WAIT);
		if (err) {
			LOG_WRN("k_msgq_put: %d", err);
		}

		break;
	}
}

void esb_thread_fn(void *arg0, void *arg1, void *arg2)
{
	LOG_DBG("Starting esb thread: %p", k_current_get());

	int err = 0;

	u8_t base_addr_0[4] = DEFAULT_BASE_ADDR_0;
	u8_t base_addr_1[4] = DEFAULT_BASE_ADDR_1;
	u8_t addr_prefix[8] = DEFAULT_ADDR_PREFIX;

	struct nrf_esb_config config = NRF_ESB_DEFAULT_CONFIG;
	config.event_handler = esb_event_callback;
	config.tx_mode = NRF_ESB_TXMODE_MANUAL_START;

	err = nrf_esb_init(&config);
	__ASSERT(err == 0, "nrf_esb_init: %d", err);

	err = nrf_esb_set_base_address_0(base_addr_0);
	__ASSERT(err == 0, "nrf_esb_set_base_address_0: %d", err);

	err = nrf_esb_set_base_address_1(base_addr_1);
	__ASSERT(err == 0, "nrf_esb_set_base_address_1: %d", err);

	err = nrf_esb_set_prefixes(addr_prefix, ARRAY_SIZE(addr_prefix));
	__ASSERT(err == 0, "nrf_esb_set_prefixes: %d", err);

	err = nrf_esb_start_rx();
	__ASSERT(err == 0, "nrf_esb_start_rx: %d", err);

	time_stamp = k_uptime_get();
	k_timer_start(&frame_timer, FRAME_TIME, 0);

	while (1) {
		struct nrf_esb_payload payload;

		err = k_msgq_get(&serial_frame_q, &payload, K_FOREVER);
		if (err) {
			LOG_WRN("k_msgq_get: %d", err);
			continue;
		}

		err = nrf_esb_write_payload(&payload);
		if (err) {
			LOG_ERR("nrf_esb_write_payload: %d", err);
			continue;
		}

		if (k_uptime_delta(&time_stamp) < FRAME_TIME) {
			continue;
		}

		err = nrf_esb_stop_rx();
		if (err) {
			LOG_ERR("error stopping RX mode: %d", err);
			continue;
		}

		err = nrf_esb_start_tx();
		if (err) {
			LOG_ERR("error starting TX mode: %d", err);
			continue;
		}
	}
}
