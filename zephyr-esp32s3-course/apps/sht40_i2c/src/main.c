#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(sht40_oled, LOG_LEVEL_INF);

/* Fetch devices from DeviceTree: display via chosen, sensor via node label. */
static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *const sht40 = DEVICE_DT_GET(DT_NODELABEL(sht4x_0));
static bool cfb_ready;

static int draw_reading(float temp_c, float rh)
{
    if (!cfb_ready)
    {
        return -EAGAIN;
    }

    char line1[32];
    char line2[32];
    snprintk(line1, sizeof(line1), "Temp: %.1f C", temp_c);
    snprintk(line2, sizeof(line2), "RH:   %.1f %%", rh);

    cfb_print(display, "SHT40 + OLED", 2, 4);
    cfb_print(display, line1, 2, 24);
    cfb_print(display, line2, 2, 40);

    /* Finalize the buffer and push to the panel. */
    return cfb_framebuffer_finalize(display);
}

static int read_sht40(float *temp_c, float *rh)
{
    struct sensor_value val;

    if (sensor_sample_fetch(sht40) < 0)
    {
        LOG_ERR("SHT40 sample fetch failed");
        return -EIO;
    }

    if (sensor_channel_get(sht40, SENSOR_CHAN_AMBIENT_TEMP, &val) < 0)
    {
        LOG_ERR("Temp read failed");
        return -EIO;
    }
    *temp_c = sensor_value_to_double(&val);

    if (sensor_channel_get(sht40, SENSOR_CHAN_HUMIDITY, &val) < 0)
    {
        LOG_ERR("Humidity read failed");
        return -EIO;
    }
    *rh = sensor_value_to_double(&val);

    return 0;
}

int main(void)
{
    if (!device_is_ready(display))
    {
        LOG_ERR("Display device not ready");
        return 0;
    }
    if (!device_is_ready(sht40))
    {
        LOG_ERR("SHT40 device not ready");
        return 0;
    }

    /* Ensure the panel is unblanked and set to a compatible format. */
    if (display_set_pixel_format(display, PIXEL_FORMAT_MONO01) < 0)
    {
        LOG_ERR("Failed to set pixel format");
        return 0;
    }
    if (display_blanking_off(display) < 0)
    {
        LOG_ERR("Failed to unblank display");
        return 0;
    }

    int ret = cfb_framebuffer_init(display);
    if (ret < 0)
    {
        LOG_ERR("CFB init failed (%d)", ret);
        return 0;
    }
    cfb_framebuffer_set_font(display, 0);
    cfb_framebuffer_clear(display, true);
    cfb_ready = true;

    LOG_INF("SHT40 + OLED demo: sampling every 1s.");

    while (1)
    {
        float temp_c = 0.0f;
        float rh = 0.0f;

        if (read_sht40(&temp_c, &rh) == 0)
        {
            LOG_INF("Temp: %.2f C, RH: %.2f %%", temp_c, rh);
            draw_reading(temp_c, rh);
        }

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
