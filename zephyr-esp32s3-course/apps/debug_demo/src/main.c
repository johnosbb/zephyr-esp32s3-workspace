#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(debug_demo, LOG_LEVEL_DBG);

/* Global/static data to poke at with GDB watchpoints and memory reads. */
static uint32_t global_counter = 0x1234ABCD;
static uint8_t sensor_ring[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
static int static_seed = 7;

struct sample_state {
    int32_t accumulator;
    int16_t last_step;
    bool toggled;
};

static struct sample_state state = {
    .accumulator = 10,
    .last_step = -1,
    .toggled = false,
};

/*
 * Small helper you can breakpoint on:
 *   (gdb) break compute_next_step
 *   (gdb) break update_state
 */
static int compute_next_step(void)
{
    /* Create a small varying step based on CPU cycles and a static seed. */
    uint32_t tick = k_cycle_get_32();
    int step = (int)((tick & 0xFF) % 7) + static_seed;
    static_seed = (static_seed * 3 + 1) & 0xFF;
    return step;
}

/* Summation helper so you can inspect locals and return values. */
static uint32_t checksum8(const uint8_t *buf, size_t len)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

/* Touch a buffer location to give you a predictable memory slot to examine. */
static void touch_memory(void)
{
    uint32_t idx = global_counter % ARRAY_SIZE(sensor_ring);
    sensor_ring[idx] ^= (uint8_t)(state.accumulator & 0xFF);
}

/* Central point to step through in GDB; toggles state and logs progress. */
static void update_state(void)
{
    int step = compute_next_step();
    state.accumulator += step;
    state.last_step = (int16_t)step;
    state.toggled = !state.toggled;
    global_counter++;

    uint32_t sum = checksum8(sensor_ring, ARRAY_SIZE(sensor_ring));
    touch_memory();

    LOG_DBG("step=%d acc=%d sum=0x%x counter=%u toggled=%d seed=%d",
            step, state.accumulator, sum, global_counter, state.toggled, static_seed);
}

int main(void)
{
    LOG_INF("Debug demo ready. Attach GDB and break on update_state().");
    LOG_INF("Example: west debug -d build/debug_demo -- -ex \"break update_state\"");

    while (1) {
        update_state();
        k_msleep(700);
    }

    return 0;
}
