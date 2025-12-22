#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(oled_demo, LOG_LEVEL_INF);

/* Fetch the display chosen in the overlay (zephyr,display = &ssd1306_oled). */
static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static void draw_banner(void)
{
    /* Initialize the character frame buffer helper. */
    if (cfb_framebuffer_init(display) < 0)
    {
        LOG_ERR("CFB init failed");
        return;
    }

    /* Optional: pick the first available font. */
    cfb_framebuffer_set_font(display, 0);

    cfb_framebuffer_clear(display, true);

    /* Text lines. */
    cfb_print(display, "Zephyr +", 2, 4);
    cfb_print(display, "SSD1306", 2, 20);
    cfb_print(display, "128x64 I2C", 2, 36);

    /* Simple box + crosshair graphic. */
    const struct cfb_position box_start = {90, 2};
    const struct cfb_position box_end = {125, 22}; /* 36x20 box area */
    const struct cfb_position h_start = {90, 12};
    const struct cfb_position h_end = {125, 12};
    const struct cfb_position v_start = {108, 2};
    const struct cfb_position v_end = {108, 22};

    cfb_draw_rect(display, &box_start, &box_end);
    cfb_draw_line(display, &h_start, &h_end);
    cfb_draw_line(display, &v_start, &v_end);

    /* Finalize the buffer and push to the panel. */
    cfb_framebuffer_finalize(display);
}

int main(void)
{
    if (!device_is_ready(display))
    {
        LOG_ERR("Display device not ready");
        return 0;
    }

    /* Ensure the panel is unblanked before drawing. */
    if (display_blanking_off(display) < 0)
    {
        LOG_ERR("Failed to unblank display");
        return 0;
    }

    draw_banner();
    LOG_INF("OLED demo ready; text + graphics drawn.");

    while (1)
    {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
