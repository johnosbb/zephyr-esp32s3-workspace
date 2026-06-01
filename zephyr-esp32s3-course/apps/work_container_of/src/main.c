/*
 * Copyright (c) 2025 John O'Sullivan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Demonstrates the canonical Zephyr pattern for giving a work-queue handler
 * its own per-instance context.
 *
 * A work handler only ever receives "struct k_work *work" -- there is no
 * void *user_data and struct k_work has no context field of its own. So when
 * you have MANY independent things that each need to defer work (here we
 * pretend they are sensor "devices"), you embed a struct k_work inside your
 * own per-instance struct and recover that struct in the handler with
 * CONTAINER_OF(). One handler function then serves every instance.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>   /* CONTAINER_OF, ARRAY_SIZE */

LOG_MODULE_REGISTER(work_container_of, LOG_LEVEL_INF);

/* LEDs pulled from the overlay aliases; some devices below use them. */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/*
 * Our per-instance "super-struct". The k_work member is what the kernel
 * sees; everything after it is private context that belongs to this device.
 */
struct sensor_device {
    struct k_work work;             /* embedded kernel work item        */
    const char *name;               /* per-instance context             */
    int id;                         /* per-instance context             */
    uint32_t jobs_handled;          /* per-instance MUTABLE state        */
    const struct gpio_dt_spec *led; /* optional LED, NULL if none        */
};

/*
 * A small fixed pool of devices. In a real "devices come and go" scenario
 * (USB, hot-plugged sensors, ...) this could be a dynamically managed pool,
 * but the work/CONTAINER_OF mechanism is identical.
 */
static struct sensor_device devices[] = {
    { .name = "temp-sensor",   .id = 1, .led = &led0 },
    { .name = "humidity",      .id = 2, .led = &led1 },
    { .name = "accelerometer", .id = 3, .led = NULL  },
};

/*
 * ONE handler for ALL devices. It is handed a plain struct k_work *, with no
 * idea which device it belongs to -- until CONTAINER_OF walks back from the
 * embedded "work" member to the enclosing struct sensor_device.
 */
static void sensor_work_handler(struct k_work *work)
{
    struct sensor_device *dev = CONTAINER_OF(work, struct sensor_device, work);

    /* Per-instance state proves each device is tracked independently. */
    dev->jobs_handled++;

    LOG_INF("Handler ran for device '%s' (id=%d) -- job #%u",
            dev->name, dev->id, dev->jobs_handled);

    /*
     * We are in work-queue THREAD context here (not the timer's ISR), so we
     * are allowed to sleep, take locks, talk to drivers, etc.
     */
    if (dev->led != NULL) {
        gpio_pin_set_dt(dev->led, 1);
        k_msleep(40);
        gpio_pin_set_dt(dev->led, 0);
    }
}

/*
 * For the purpose of this demo I am using a timer to stand in for "an event arriving from some device". It runs in ISR
 * context and only submits the work item -- the same fast-top-half /
 * deferred-bottom-half pattern from the interrupts lecture.
 */
static void event_timer_handler(struct k_timer *timer)
{
    static size_t next;

    ARG_UNUSED(timer);

    /* Round-robin: pretend each tick is an event for a different device. */
    k_work_submit(&devices[next].work);
    next = (next + 1) % ARRAY_SIZE(devices);
}
K_TIMER_DEFINE(event_timer, event_timer_handler, NULL);

static void setup_led(const struct gpio_dt_spec *led)
{
    if (device_is_ready(led->port)) {
        gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
    }
}

int main(void)
{
    setup_led(&led0);
    setup_led(&led1);

    /*
     * Initialise one work item per device, ALL pointing at the same handler.
     * The handler tells the instances apart purely via CONTAINER_OF.
     */
    for (size_t i = 0; i < ARRAY_SIZE(devices); i++) {
        k_work_init(&devices[i].work, sensor_work_handler);
    }

    LOG_INF("CONTAINER_OF workqueue demo: %u devices share one handler",
            (unsigned int)ARRAY_SIZE(devices));

    /* One event every 500 ms, cycling through the devices. */
    k_timer_start(&event_timer, K_MSEC(500), K_MSEC(500));

    /* All the real work happens on the system workqueue thread. */
    return 0;
}
