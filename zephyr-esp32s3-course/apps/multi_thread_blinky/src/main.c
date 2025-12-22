/*
 * Copyright (c) 2025 John O'Sullivan
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mtblink, LOG_LEVEL_INF);

/* Two LED bindings pulled from the overlay aliases. */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

struct blink_ctx {
    const struct gpio_dt_spec *led;
    uint32_t interval_ms;
    const char *name;
};

/* Thread stacks and priorities (lower number = higher priority). */
K_THREAD_STACK_DEFINE(led0_stack, 1024);
K_THREAD_STACK_DEFINE(led1_stack, 1024);
static struct k_thread led0_thread;
static struct k_thread led1_thread;

static void blink_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    struct blink_ctx *ctx = p1;
    bool state = false;

    while (1) {
        gpio_pin_set_dt(ctx->led, state);
        LOG_INF("%s toggled %s", ctx->name, state ? "ON" : "OFF");
        state = !state;
        k_msleep(ctx->interval_ms);
    }
}

static int setup_led(const struct gpio_dt_spec *led)
{
    if (!device_is_ready(led->port)) {
        LOG_ERR("LED port not ready");
        return -ENODEV;
    }
    return gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
}

int main(void)
{
    if (setup_led(&led0) < 0 || setup_led(&led1) < 0) {
        return 0;
    }

    static struct blink_ctx ctx0 = {
        .led = &led0,
        .interval_ms = 300,
        .name = "LED0",
    };
    static struct blink_ctx ctx1 = {
        .led = &led1,
        .interval_ms = 700,
        .name = "LED1",
    };

    k_thread_create(&led0_thread, led0_stack, K_THREAD_STACK_SIZEOF(led0_stack),
                    blink_thread, &ctx0, NULL, NULL,
                    K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
    k_thread_name_set(&led0_thread, "led0_blink");

    k_thread_create(&led1_thread, led1_stack, K_THREAD_STACK_SIZEOF(led1_stack),
                    blink_thread, &ctx1, NULL, NULL,
                    K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
    k_thread_name_set(&led1_thread, "led1_blink");

    LOG_INF("Multi-thread blinky started (LED0=%u ms, LED1=%u ms)",
            ctx0.interval_ms, ctx1.interval_ms);

    return 0;
}
