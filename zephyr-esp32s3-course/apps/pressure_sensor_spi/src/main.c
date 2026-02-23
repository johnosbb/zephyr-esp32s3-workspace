#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <math.h>
#include <stdio.h>

#define SEA_LEVEL_PRESSURE_KPA 101.325f
#define SAMPLE_PERIOD_MS 500

LOG_MODULE_REGISTER(bmp280_oled, LOG_LEVEL_INF);

/* Display and sensor devices from devicetree. */
static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *const bmp280 = DEVICE_DT_GET(DT_NODELABEL(bmp280));

static bool cfb_ready;

static float pressure_to_altitude(float pressure_kpa)
{
	if (pressure_kpa <= 0.0f) {
		return 0.0f;
	}

	return 44330.0f * (1.0f - powf(pressure_kpa / SEA_LEVEL_PRESSURE_KPA, 0.1903f));
}

static void update_display(float temp_c, float pressure_kpa, float altitude_m)
{
	if (!cfb_ready) {
		return;
	}

	char line1[32], line2[32], line3[32];
	snprintk(line1, sizeof(line1), "T: %.2f C", (double)temp_c);
	snprintk(line2, sizeof(line2), "P: %.2f kPa", (double)pressure_kpa);
	snprintk(line3, sizeof(line3), "Alt: %.1f m", (double)altitude_m);

	cfb_framebuffer_clear(display, true);
	cfb_print(display, "BMP280 (SPI)", 2, 0);
	cfb_print(display, line1, 2, 16);
	cfb_print(display, line2, 2, 32);
	cfb_print(display, line3, 2, 48);
	cfb_framebuffer_finalize(display);
}

int main(void)
{
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	if (display_set_pixel_format(display, PIXEL_FORMAT_MONO01) < 0) {
		LOG_ERR("Failed to set pixel format");
		return 0;
	}
	if (display_blanking_off(display) < 0) {
		LOG_ERR("Failed to unblank display");
		return 0;
	}
	if (cfb_framebuffer_init(display) < 0) {
		LOG_ERR("CFB init failed");
		return 0;
	}
	cfb_framebuffer_set_font(display, 0);
	cfb_ready = true;

	if (!device_is_ready(bmp280)) {
		LOG_WRN("BMP280 device not ready; showing placeholder screen");
		cfb_framebuffer_clear(display, true);
		cfb_print(display, "BMP280 not ready", 2, 16);
		cfb_print(display, "Check SPI wiring", 2, 32);
		cfb_framebuffer_finalize(display);
		goto idle;
	}

	LOG_INF("BMP280 SPI + OLED demo running (polling)");

	while (1) {
		struct sensor_value temp;
		struct sensor_value press;

		if (sensor_sample_fetch(bmp280) < 0) {
			LOG_ERR("BMP280 sample fetch failed");
			k_msleep(SAMPLE_PERIOD_MS);
			continue;
		}
		if (sensor_channel_get(bmp280, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
			LOG_ERR("BMP280 temperature read failed");
			k_msleep(SAMPLE_PERIOD_MS);
			continue;
		}
		if (sensor_channel_get(bmp280, SENSOR_CHAN_PRESS, &press) < 0) {
			LOG_ERR("BMP280 pressure read failed");
			k_msleep(SAMPLE_PERIOD_MS);
			continue;
		}

		float temp_c = sensor_value_to_double(&temp);
		float pressure_kpa = sensor_value_to_double(&press);
		float altitude_m = pressure_to_altitude(pressure_kpa);

		LOG_INF("Temp: %.2f C Pressure: %.2f kPa Alt: %.1f m",
			(double)temp_c, (double)pressure_kpa, (double)altitude_m);
		update_display(temp_c, pressure_kpa, altitude_m);
		k_msleep(SAMPLE_PERIOD_MS);
	}

idle:
	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
