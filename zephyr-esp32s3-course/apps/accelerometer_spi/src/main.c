#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(adxl_oled, LOG_LEVEL_INF);

/* Display and sensor devices from devicetree. */
static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *const adxl = DEVICE_DT_GET_ONE(adi_adxl345);

static struct k_work display_work;
static struct sensor_trigger data_ready_trig;
static bool cfb_ready;
static bool adxl_ready;

static void update_display(float x_g, float y_g, float z_g)
{
    if (!cfb_ready) {
        return;
    }

    char line1[32], line2[32], line3[32];
    snprintk(line1, sizeof(line1), "X: %.2f g", x_g);
    snprintk(line2, sizeof(line2), "Y: %.2f g", y_g);
    snprintk(line3, sizeof(line3), "Z: %.2f g", z_g);

    cfb_framebuffer_clear(display, true);
    cfb_print(display, "ADXL345 (SPI)", 2, 4);
    cfb_print(display, line1, 2, 20);
    cfb_print(display, line2, 2, 36);
    cfb_print(display, line3, 2, 52);
    cfb_framebuffer_finalize(display);
}

static void display_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    if (!adxl_ready) {
        return;
    }

    struct sensor_value accel[3];
    if (sensor_sample_fetch(adxl) < 0) {
        LOG_ERR("ADXL sample fetch failed");
        return;
    }
    if (sensor_channel_get(adxl, SENSOR_CHAN_ACCEL_XYZ, accel) < 0) {
        LOG_ERR("ADXL read failed");
        return;
    }

    float x = sensor_value_to_double(&accel[0]);
    float y = sensor_value_to_double(&accel[1]);
    float z = sensor_value_to_double(&accel[2]);

    LOG_INF("Accel: X=%.3f g Y=%.3f g Z=%.3f g", x, y, z);
    update_display(x, y, z);
}

static void adxl_trigger_handler(const struct device *dev,
                                 const struct sensor_trigger *trig)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(trig);
    /* Offload the read/display work to the system workqueue. */
    k_work_submit(&display_work);
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

    k_work_init(&display_work, display_work_handler);

    adxl_ready = device_is_ready(adxl);
    if (!adxl_ready) {
        LOG_WRN("ADXL device not ready; showing placeholder screen");
        cfb_framebuffer_clear(display, true);
        cfb_print(display, "ADXL345 not ready", 2, 20);
        cfb_print(display, "Check wiring/CS", 2, 36);
        cfb_framebuffer_finalize(display);
        goto idle;
    }

    data_ready_trig.type = SENSOR_TRIG_DATA_READY;
    data_ready_trig.chan = SENSOR_CHAN_ACCEL_XYZ;
    if (sensor_trigger_set(adxl, &data_ready_trig, adxl_trigger_handler) < 0) {
        LOG_WRN("Failed to set data ready trigger; falling back to polling");
        while (1) {
            k_work_submit(&display_work);
            k_msleep(200);
        }
    }

    LOG_INF("ADXL345 SPI + OLED demo running (data ready interrupt -> workqueue)");

idle:
    /* Main thread idles; work is driven by sensor triggers. */
    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
