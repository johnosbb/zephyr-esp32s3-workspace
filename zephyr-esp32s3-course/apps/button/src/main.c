#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(button, LOG_LEVEL_INF);

/* Debounce window; increase if your button is especially noisy. */
#define DEBOUNCE_MS 100

/* Pull LED/button GPIO binding from the board's DeviceTree aliases. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

/* ISR bookkeeping and delayed work for debouncing. */
static struct gpio_callback button_cb_data;
static struct k_work_delayable debounce_work;
/* Tracks whether the current press has already toggled the LED (prevents repeat toggles while held). */
static bool press_latched;

/* Runs in a system work queue context after the debounce delay expires. */
static void debounce_work_handler(struct k_work *work)
{
    int val = gpio_pin_get_dt(&button);
    if (val < 0)
    {
        LOG_ERR("Failed to read button state (%d)", val);
        return;
    }

    /* Active-low input: 0 means the button is held. */
    bool pressed_now = (val == 0);

    if (pressed_now && !press_latched)
    {
        gpio_pin_toggle_dt(&led);
        LOG_INF("Button pressed; toggling LED");
        press_latched = true;
    }
    else if (!pressed_now && press_latched)
    {
        /* Release detected; unlock for the next press. */
        press_latched = false;
    }
}

/* Interrupt callback: schedule debounced processing on the work queue. */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    k_work_reschedule(&debounce_work, K_MSEC(DEBOUNCE_MS));
}

int main(void)
{
    /* Validate that the GPIO controllers backing the bindings are ready. */
    if (!device_is_ready(led.port) || !device_is_ready(button.port))
    {
        LOG_ERR("LED or button device not ready");
        return 0;
    }

    /* Start with LED off; use the flags baked into the binding. */
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0)
    {
        LOG_ERR("Failed to configure LED");
        return 0;
    }

    if (gpio_pin_configure_dt(&button, GPIO_INPUT) < 0)
    {
        LOG_ERR("Failed to configure button");
        return 0;
    }

    /* Use both edges: press starts debounce, release clears the latch. */
    if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH) < 0)
    {
        LOG_ERR("Failed to enable button interrupt");
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    if (gpio_add_callback(button.port, &button_cb_data) < 0)
    {
        LOG_ERR("Failed to add button callback");
        return 0;
    }

    /* Initialize after the callback is registered to avoid races. */
    k_work_init_delayable(&debounce_work, debounce_work_handler);

    LOG_INF("Button demo ready; press the button to toggle the LED.");

    while (1)
    {
        /* Sleep forever; all work happens via the interrupt + work queue. */
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
