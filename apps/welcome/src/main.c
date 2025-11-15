#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(welcome, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Welcome to the Zephyr ESP32-S3 course!");
    LOG_INF("This is your first example application.");

    while (1)
    {
        k_msleep(1000);
        LOG_INF("Still running...");
    }

    return 0;
}
