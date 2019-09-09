#ifndef LEDS_H
#define LEDS_H

enum led {
	LED_0,
	LED_1,
	LED_2,
	LED_3
};

void led_init(void);
void led_flash(enum led led);

#endif // LEDS_H
