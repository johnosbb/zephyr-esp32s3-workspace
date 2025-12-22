/*
 * Copyright (c) 2025 John O'Sullivan
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(prod_cons, LOG_LEVEL_INF);

/* LED bindings pulled from the overlay aliases. */
static const struct gpio_dt_spec led_producer = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_consumer = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/* Simple message type passed through the queue. */
struct msg {
    uint32_t seq;
    uint32_t value;
};

#define MSGQ_LEN 8
K_MSGQ_DEFINE(msgq, sizeof(struct msg), MSGQ_LEN, 4);

K_THREAD_STACK_DEFINE(prod_stack, 1024);
K_THREAD_STACK_DEFINE(cons_stack, 1024);
static struct k_thread prod_thread;
static struct k_thread cons_thread;

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

static void producer(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    uint32_t seq = 0;

    while (1) {
        struct msg m = {
            .seq = seq++,
            .value = k_cycle_get_32(),
        };

        if (k_msgq_put(&msgq, &m, K_NO_WAIT) == 0) {
            pulse_led(&led_producer, 30);
            LOG_INF("Produced seq=%u val=0x%08x", m.seq, m.value);
        } else {
            LOG_WRN("Queue full; dropping seq=%u", m.seq);
        }
        k_msleep(200);
    }
}

static void consumer(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    struct msg m;

    while (1) {
        k_msgq_get(&msgq, &m, K_FOREVER);
        pulse_led(&led_consumer, 60);
        LOG_INF("Consumed seq=%u val=0x%08x", m.seq, m.value);
        k_msleep(50);
    }
}

int main(void)
{
    if (setup_led(&led_producer) < 0 || setup_led(&led_consumer) < 0) {
        return 0;
    }

    k_thread_create(&prod_thread, prod_stack, K_THREAD_STACK_SIZEOF(prod_stack),
                    producer, NULL, NULL, NULL,
                    K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
    k_thread_name_set(&prod_thread, "producer");

    k_thread_create(&cons_thread, cons_stack, K_THREAD_STACK_SIZEOF(cons_stack),
                    consumer, NULL, NULL, NULL,
                    K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
    k_thread_name_set(&cons_thread, "consumer");

    LOG_INF("Producer-consumer demo started (msgq depth=%d)", MSGQ_LEN);
    return 0;
}
