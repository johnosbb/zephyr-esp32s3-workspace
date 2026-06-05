/*
 * Copyright (c) 2025 John O'Sullivan
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 2000 msec = 2 sec */
#define SLEEP_TIME_MS 2000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means the board has no "led0" alias.
 * Define one in boards/esp32s3_devkitc.overlay (see this app's overlay).
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;
    bool led_state = true;

    if (!gpio_is_ready_dt(&led))
    {
        printf("LED GPIO device not ready\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        printf("Failed to configure LED GPIO (%d)\n", ret);
        return 0;
    }

    while (1)
    {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0)
        {
            printf("Failed to toggle LED GPIO (%d)\n", ret);
            return 0;
        }

        led_state = !led_state;
        printf("LED state: %s\n", led_state ? "ON" : "OFF");
        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}
