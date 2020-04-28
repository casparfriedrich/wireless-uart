#ifndef LEDS_H
#define LEDS_H

enum led { LED_RED, LED_GREEN, LED_BLUE };

void led_flash(enum led led);

#endif // LEDS_H
