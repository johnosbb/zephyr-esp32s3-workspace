#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(mcp3008_oled, LOG_LEVEL_INF);

#define POLL_DELAY_MS 50
#define UPDATE_INTERVAL_MS 200

static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct spi_dt_spec mcp3008 = SPI_DT_SPEC_GET(
	DT_NODELABEL(mcp3008),
	SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	0);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static bool cfb_ready;
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

static void update_display(uint8_t channel, uint16_t value)
{
	if (!cfb_ready) {
		return;
	}

	char line1[32];
	char line2[32];
	char line3[32];
	uint16_t y0 = 0;
	uint16_t y1 = 16;
	uint16_t y2 = 32;

	snprintk(line1, sizeof(line1), "MCP3008 ADC");
	snprintk(line2, sizeof(line2), "CH%u", channel);
	snprintk(line3, sizeof(line3), "VAL %4u", value);

	cfb_framebuffer_clear(display, true);
	cfb_print(display, line1, 2, y0);
	cfb_print(display, line2, 2, y1);
	cfb_print(display, line3, 2, y2);
	cfb_framebuffer_finalize(display);
}

int main(void)
{
	if (!device_is_ready(button.port)) {
		LOG_ERR("Button GPIO port not ready");
		return 0;
	}
	(void)gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);

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

	uint8_t channel = 0;
	uint16_t value = 0;
	uint16_t last_value = 0;
	int64_t last_update_ms = 0;
	bool last_pressed = false;

	if (read_mcp3008(channel, &value) == 0) {
		LOG_INF("MCP3008: CH%u = %u", channel, value);
		update_display(channel, value);
		last_value = value;
		last_update_ms = k_uptime_get();
	} else {
		LOG_WRN("MCP3008 initial read failed");
	}

	while (1) {
		int btn = gpio_pin_get_dt(&button);
		bool pressed = (btn == 0);

		if (pressed != last_pressed) {
			LOG_INF("Button: %s", pressed ? "pressed" : "released");
		}

		if (pressed && !last_pressed) {
			channel = (channel + 1) % 8;
		}
		last_pressed = pressed;

		int64_t now_ms = k_uptime_get();
		if ((now_ms - last_update_ms) >= UPDATE_INTERVAL_MS) {
			if (read_mcp3008(channel, &value) == 0) {
				if (value != last_value) {
					LOG_INF("MCP3008: CH%u = %u", channel, value);
					update_display(channel, value);
					last_value = value;
				}
			} else {
				LOG_WRN("MCP3008 read failed");
			}
			last_update_ms = now_ms;
		}

		k_msleep(POLL_DELAY_MS);
	}

	return 0;
}
