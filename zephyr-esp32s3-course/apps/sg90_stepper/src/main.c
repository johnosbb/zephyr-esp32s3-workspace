/*
 * SG90 hobby servo demo using a single PWM pin.
 * Button cycles a 3-state machine: off -> right -> left -> off.
 * LED mirrors "servo active" (on when driving).
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sg90, LOG_LEVEL_INF);

#define DEBOUNCE_MS 100
#define SERVO_PERIOD PWM_MSEC(20)
#define SERVO_PULSE_LEFT PWM_USEC(1000)
#define SERVO_PULSE_RIGHT PWM_USEC(2000)

/* Bindings from the overlay. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct pwm_dt_spec servo_pwm = PWM_DT_SPEC_GET(DT_ALIAS(servo0));

/* Button ISR + debounce. */
static struct gpio_callback button_cb_data;
static struct k_work_delayable debounce_work;
static bool press_latched;

enum servo_state {
    SERVO_OFF = 0,
    SERVO_RIGHT,
    SERVO_LEFT,
};

static enum servo_state state = SERVO_OFF;

static void set_servo_pulse(uint32_t pulse)
{
    int ret = pwm_set_dt(&servo_pwm, SERVO_PERIOD, pulse);
    if (ret < 0) {
        LOG_ERR("pwm_set_dt failed (%d)", ret);
    }
}

static void apply_state(enum servo_state new_state)
{
    state = new_state;

    switch (state) {
    case SERVO_RIGHT:
        set_servo_pulse(SERVO_PULSE_RIGHT);
        gpio_pin_set_dt(&led, 1);
        LOG_INF("Servo: right (%u us)", SERVO_PULSE_RIGHT / 1000U);
        break;
    case SERVO_LEFT:
        set_servo_pulse(SERVO_PULSE_LEFT);
        gpio_pin_set_dt(&led, 1);
        LOG_INF("Servo: left (%u us)", SERVO_PULSE_LEFT / 1000U);
        break;
    case SERVO_OFF:
    default:
        set_servo_pulse(0);
        gpio_pin_set_dt(&led, 0);
        LOG_INF("Servo: off");
        break;
    }
}

static void advance_state(void)
{
    switch (state) {
    case SERVO_OFF:
        apply_state(SERVO_RIGHT);
        break;
    case SERVO_RIGHT:
        apply_state(SERVO_LEFT);
        break;
    case SERVO_LEFT:
    default:
        apply_state(SERVO_OFF);
        break;
    }
}

static void debounce_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    int val = gpio_pin_get_dt(&button);
    if (val < 0) {
        LOG_ERR("Failed to read button state (%d)", val);
        return;
    }

    bool pressed_now = (val == 0); /* active-low button */

    if (pressed_now && !press_latched) {
        press_latched = true;
        advance_state();
    } else if (!pressed_now && press_latched) {
        press_latched = false;
    }
}

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    k_work_reschedule(&debounce_work, K_MSEC(DEBOUNCE_MS));
}

static int configure_gpio(const struct gpio_dt_spec *spec, gpio_flags_t flags)
{
    if (!device_is_ready(spec->port)) {
        LOG_ERR("Port not ready");
        return -ENODEV;
    }
    return gpio_pin_configure_dt(spec, flags);
}

int main(void)
{
    if (configure_gpio(&led, GPIO_OUTPUT_INACTIVE) < 0 ||
        configure_gpio(&button, GPIO_INPUT) < 0) {
        LOG_ERR("LED/button init failed");
        return 0;
    }

    if (!pwm_is_ready_dt(&servo_pwm)) {
        LOG_ERR("Servo PWM device not ready");
        return 0;
    }

    if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH) < 0) {
        LOG_ERR("Button interrupt init failed");
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    k_work_init_delayable(&debounce_work, debounce_work_handler);

    /* Start idle. */
    apply_state(SERVO_OFF);

    LOG_INF("SG90 servo demo ready. Button cycles Off -> Right -> Left.");

    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
