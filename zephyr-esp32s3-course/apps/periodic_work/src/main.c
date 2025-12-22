/*
 * Copyright (c) 2025 John O'Sullivan
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(workdemo, LOG_LEVEL_INF);

/* LED bindings pulled from the overlay aliases. */
static const struct gpio_dt_spec led_fast = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_slow = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/* Work items to offload "heavy" tasks from ISR-like callbacks. */
static void fast_work_handler(struct k_work *work);
static void slow_work_handler(struct k_work *work);
K_WORK_DEFINE(fast_work, fast_work_handler);
K_WORK_DEFINE(slow_work, slow_work_handler);

/* Timers fire in interrupt context and simply submit work to the system queue. */
static void fast_timer_handler(struct k_timer *timer);
static void slow_timer_handler(struct k_timer *timer);
K_TIMER_DEFINE(fast_timer, fast_timer_handler, NULL);
K_TIMER_DEFINE(slow_timer, slow_timer_handler, NULL);

static int setup_led(const struct gpio_dt_spec *led)
{
    if (!device_is_ready(led->port)) {
        LOG_ERR("LED port not ready");
        return -ENODEV;
    }
    return gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
}

static void pulse_led(const struct gpio_dt_spec *led, uint32_t on_ms)
{
    gpio_pin_set_dt(led, 1);
    k_msleep(on_ms);
    gpio_pin_set_dt(led, 0);
}

static void fast_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    pulse_led(&led_fast, 20);
    LOG_INF("Fast work executed");
    /* Simulate a modest processing chunk offloaded from ISR. */
    k_msleep(30);
}

static void slow_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    pulse_led(&led_slow, 50);
    LOG_INF("Slow work executed");
    /* Simulate heavier processing off the ISR path. */
    k_msleep(80);
}

static void fast_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&fast_work);
}

static void slow_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&slow_work);
}

int main(void)
{
    if (setup_led(&led_fast) < 0 || setup_led(&led_slow) < 0) {
        return 0;
    }

    /* Start periodic timers: fast every 200 ms, slow every 700 ms. */
    k_timer_start(&fast_timer, K_MSEC(200), K_MSEC(200));
    k_timer_start(&slow_timer, K_MSEC(700), K_MSEC(700));

    LOG_INF("Periodic workqueue demo running (fast=200ms, slow=700ms)");

    /* Nothing else to do in main; work runs on the system workqueue. */
    return 0;
}
