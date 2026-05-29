#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(hw_diag, LOG_LEVEL_INF);

#define SERVO_PERIOD PWM_MSEC(20)
#define SERVO_PULSE_LEFT PWM_USEC(1000)
#define SERVO_PULSE_CENTER PWM_USEC(1500)
#define SERVO_PULSE_RIGHT PWM_USEC(2000)
#define SERVO_SWEEP_PERIOD_MS 10000

/* Devices from devicetree. */
static const struct device *const i2c0_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *const sht40 = DEVICE_DT_GET(DT_NODELABEL(sht4x_0));
static const struct spi_dt_spec mcp3008 =
	SPI_DT_SPEC_GET(DT_NODELABEL(mcp3008), SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct pwm_dt_spec servo_pwm = PWM_DT_SPEC_GET(DT_ALIAS(servo0));

static bool cfb_ready;

static void servo_sweep(void)
{
	if (!pwm_is_ready_dt(&servo_pwm)) {
		LOG_WRN("Servo PWM not ready; skipping sweep");
		return;
	}

	LOG_INF("Servo sweep: left -> right -> center");
	(void)pwm_set_dt(&servo_pwm, SERVO_PERIOD, SERVO_PULSE_LEFT);
	k_msleep(600);
	(void)pwm_set_dt(&servo_pwm, SERVO_PERIOD, SERVO_PULSE_RIGHT);
	k_msleep(600);
	(void)pwm_set_dt(&servo_pwm, SERVO_PERIOD, SERVO_PULSE_CENTER);
	k_msleep(400);
}

static void update_i2c_status(bool *oled_seen, bool *sht_seen, int sht_ret)
{
	*oled_seen = cfb_ready && device_is_ready(display);
	*sht_seen = (sht_ret == 0);

	if (!device_is_ready(i2c0_dev)) {
		LOG_ERR("I2C0 not ready");
	}
}

static void display_status(bool oled_seen, bool sht_seen,
			   float sht_temp_c, float sht_rh,
			   uint8_t adc_channel, uint16_t adc_value,
			   bool button_pressed)
{
	if (!cfb_ready) {
		return;
	}

	char line1[32];
	char line2[32];
	char line3[32];

	snprintk(line1, sizeof(line1), "OLED %s SHT %s",
		 oled_seen ? "OK" : "--",
		 sht_seen ? "OK" : "--");
	snprintk(line2, sizeof(line2), "SHT T %.1fC RH %.1f%%",
		 (double)sht_temp_c, (double)sht_rh);
	snprintk(line3, sizeof(line3), "ADC CH%u: %u",
		 adc_channel, adc_value);

	cfb_framebuffer_clear(display, true);
	cfb_print(display, "HW Diagnostics", 2, 0);
	cfb_print(display, line1, 2, 16);
	cfb_print(display, line2, 2, 32);
	cfb_print(display, line3, 2, 48);
	if (button_pressed) {
		cfb_print(display, "BTN", 100, 0);
	}
	cfb_framebuffer_finalize(display);
}

static int read_sht40(float *temp_c, float *rh)
{
	struct sensor_value val;

	if (!device_is_ready(sht40)) {
		return -ENODEV;
	}

	if (sensor_sample_fetch(sht40) < 0) {
		return -EIO;
	}
	if (sensor_channel_get(sht40, SENSOR_CHAN_AMBIENT_TEMP, &val) < 0) {
		return -EIO;
	}
	*temp_c = sensor_value_to_double(&val);

	if (sensor_channel_get(sht40, SENSOR_CHAN_HUMIDITY, &val) < 0) {
		return -EIO;
	}
	*rh = sensor_value_to_double(&val);

	return 0;
}

static int read_mcp3008(uint8_t channel, uint16_t *value)
{
	uint8_t tx[3] = { 0x01, (uint8_t)(0x80 | (channel << 4)), 0x00 };
	uint8_t rx[3] = { 0 };
	const struct spi_buf tx_buf = { .buf = tx, .len = sizeof(tx) };
	const struct spi_buf rx_buf = { .buf = rx, .len = sizeof(rx) };
	const struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	const struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	if (channel > 7) {
		return -EINVAL;
	}
	if (!spi_is_ready_dt(&mcp3008)) {
		return -ENODEV;
	}

	int ret = spi_transceive_dt(&mcp3008, &tx_set, &rx_set);
	if (ret < 0) {
		return ret;
	}

	*value = (uint16_t)(((rx[1] & 0x03) << 8) | rx[2]);
	return 0;
}

int main(void)
{
	if (!device_is_ready(led0.port) || !device_is_ready(led1.port)) {
		LOG_ERR("LED GPIO ports not ready");
		return 0;
	}
	(void)gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	(void)gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);

	if (!device_is_ready(button.port)) {
		LOG_ERR("Button GPIO port not ready");
		return 0;
	}
	(void)gpio_pin_configure_dt(&button, GPIO_INPUT);

	if (device_is_ready(display)) {
		if (display_set_pixel_format(display, PIXEL_FORMAT_MONO01) < 0) {
			LOG_WRN("Display pixel format set failed");
		}
		if (display_blanking_off(display) < 0) {
			LOG_WRN("Display unblank failed");
		}
		if (cfb_framebuffer_init(display) == 0) {
			cfb_framebuffer_set_font(display, 0);
			cfb_ready = true;
		} else {
			LOG_WRN("CFB init failed");
		}
	} else {
		LOG_WRN("Display device not ready");
	}

	servo_sweep();

	bool oled_seen = false;
	bool sht_seen = false;

	LOG_INF("Diagnostics running: LEDs blink, button state, SHT40 readout");

	bool led_state = false;
	int64_t last_sweep_ms = k_uptime_get();
	uint8_t adc_channel = 0;
	uint16_t adc_value = 0;
	bool last_button_pressed = false;
	if (read_mcp3008(adc_channel, &adc_value) == 0) {
		LOG_INF("MCP3008: CH%u = %u", adc_channel, adc_value);
	} else {
		LOG_WRN("MCP3008 initial read failed");
	}
	while (1) {
		led_state = !led_state;
		gpio_pin_set_dt(&led0, led_state);
		gpio_pin_set_dt(&led1, !led_state);

		int btn = gpio_pin_get_dt(&button);
		bool button_pressed = (btn == 0);

		if (button_pressed && !last_button_pressed) {
			adc_channel = (adc_channel + 1) % 8;
			int adc_ret = read_mcp3008(adc_channel, &adc_value);
			if (adc_ret == 0) {
				LOG_INF("MCP3008: CH%u = %u", adc_channel, adc_value);
			} else {
				LOG_WRN("MCP3008 read failed (%d)", adc_ret);
			}
		}
		last_button_pressed = button_pressed;

		float sht_temp_c = 0.0f;
		float sht_rh = 0.0f;
		int sht_ret = read_sht40(&sht_temp_c, &sht_rh);
		if (sht_ret == 0) {
			LOG_INF("SHT40: %.2f C, %.2f %%", (double)sht_temp_c,
				(double)sht_rh);
		} else {
			LOG_WRN("SHT40 read failed");
		}

		LOG_INF("Button: %s", button_pressed ? "pressed" : "released");
		update_i2c_status(&oled_seen, &sht_seen, sht_ret);
		display_status(oled_seen, sht_seen, sht_temp_c, sht_rh,
			       adc_channel, adc_value, button_pressed);

		if (k_uptime_get() - last_sweep_ms >= SERVO_SWEEP_PERIOD_MS) {
			servo_sweep();
			last_sweep_ms = k_uptime_get();
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
