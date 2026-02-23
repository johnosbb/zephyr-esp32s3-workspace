#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Register the module 'app' and set the visibility level to DEBUG */
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

int main(void)
{
    int counter = 0;
    LOG_INF("System Monitoring Started (Loop: 10ms)");

    while (1) {
        k_msleep(100); 
        counter++;

        /* * LOG_DBG: The High-Resolution "logging"
         * This will fill your buffer quickly.
         * Useful during active development to see every tick.
         */
        LOG_DBG("Processing cycle %d", counter);

        /* * LOG_ERR: Simulating a Critical Event
         */
        if ((counter % 100) == 0) {
            LOG_ERR("Critical: Periodic simulated error at count %d", counter);
        }
    }

    return 0;
}