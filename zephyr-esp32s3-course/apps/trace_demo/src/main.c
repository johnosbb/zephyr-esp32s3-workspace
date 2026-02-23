#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/tracing/tracing.h>

LOG_MODULE_REGISTER(trace_demo, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)

/* Thread stack sizes */
#define STACK_SIZE 3072

/* Priorities: lower number = higher priority */
#define PRIO_FAST   1
#define PRIO_WORKER 2
#define PRIO_IO     3
#define PRIO_HEARTBEAT 4

#define FAST_PERIOD_MS 10
#define IO_PERIOD_MS   30
#define TIMER_PERIOD_MS 50
#define HEARTBEAT_PERIOD_MS 500
#define FAST_TRACE_DIVIDER 25
/* Set to 1 for periodic thread stack margin logs. */
#define TRACE_DEMO_STACK_LOG 0

static struct k_thread thread_fast;
static struct k_thread thread_worker;
static struct k_thread thread_io;
static struct k_thread thread_heartbeat;

K_THREAD_STACK_DEFINE(stack_fast, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_worker, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_io, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_heartbeat, STACK_SIZE);

static struct k_sem work_sem;
static struct k_mutex shared_mutex;
static struct k_timer wake_timer;

static volatile uint32_t fast_count;
static volatile uint32_t worker_count;
static volatile uint32_t io_count;
static volatile uint32_t isr_count;
static const struct gpio_dt_spec heartbeat_led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#if TRACE_DEMO_STACK_LOG
static void log_stack_margins(void)
{
	size_t unused = 0U;

	if (k_thread_stack_space_get(&thread_fast, &unused) == 0) {
		LOG_INF("stack thread_fast unused=%u", (uint32_t)unused);
	}
	if (k_thread_stack_space_get(&thread_worker, &unused) == 0) {
		LOG_INF("stack thread_worker unused=%u", (uint32_t)unused);
	}
	if (k_thread_stack_space_get(&thread_io, &unused) == 0) {
		LOG_INF("stack thread_io unused=%u", (uint32_t)unused);
	}
	if (k_thread_stack_space_get(&thread_heartbeat, &unused) == 0) {
		LOG_INF("stack thread_heartbeat unused=%u", (uint32_t)unused);
	}
}
#endif

static void busy_work(uint32_t loops)
{
	volatile uint32_t sink = 0;

	for (uint32_t i = 0; i < loops; i++) {
		sink += i;
	}
	ARG_UNUSED(sink);
}

static void timer_expiry(struct k_timer *timer)
{
	ARG_UNUSED(timer);

	isr_count++;

	k_sem_give(&work_sem);
}

static void thread_fast_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		fast_count++;

		/* Fast loop runs at 100 Hz: sample markers instead of tracing every loop */
		if ((fast_count % FAST_TRACE_DIVIDER) == 0U) {
			sys_trace_named_event("app.fast.loop", fast_count, 0);
		}

		busy_work(4000);
		k_msleep(FAST_PERIOD_MS);
	}
}

static void thread_worker_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		/* Marker: about to block on semaphore */
		sys_trace_named_event("app.worker.sem_take_enter", worker_count, 0);

		k_sem_take(&work_sem, K_FOREVER);

		/* Marker: semaphore acquired, worker unblocked */
		sys_trace_named_event("app.worker.sem_take_exit", worker_count, 0);

		(void)k_mutex_lock(&shared_mutex, K_FOREVER);
		worker_count++;

		/* Marker: worker entered critical section */
		sys_trace_named_event("app.worker.cs_enter", worker_count, 0);

		busy_work(25000);

		/* Marker: worker leaving critical section */
		sys_trace_named_event("app.worker.cs_exit", worker_count, 0);

		k_mutex_unlock(&shared_mutex);

		k_yield();
	}
}

static void thread_io_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		/* Marker: io wants mutex */
		sys_trace_named_event("app.io.cs_wait", io_count, 0);

		(void)k_mutex_lock(&shared_mutex, K_FOREVER);
		io_count++;

		/* Marker: io entered critical section */
		sys_trace_named_event("app.io.cs_enter", io_count, 0);

		busy_work(12000);

		/* Marker: io leaving critical section */
		sys_trace_named_event("app.io.cs_exit", io_count, 0);

		k_mutex_unlock(&shared_mutex);

		k_msleep(IO_PERIOD_MS);
	}
}

static void thread_heartbeat_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		if (gpio_is_ready_dt(&heartbeat_led)) {
			(void)gpio_pin_toggle_dt(&heartbeat_led);
		}

		k_msleep(HEARTBEAT_PERIOD_MS);
	}
}

int main(void)
{
	int ret;

	k_sem_init(&work_sem, 0, 1);
	k_mutex_init(&shared_mutex);

	if (!gpio_is_ready_dt(&heartbeat_led)) {
		LOG_WRN("Heartbeat LED device not ready");
	} else {
		ret = gpio_pin_configure_dt(&heartbeat_led, GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			LOG_WRN("Heartbeat LED configure failed: %d", ret);
		}
	}

	k_timer_init(&wake_timer, timer_expiry, NULL);
	k_timer_start(&wake_timer, K_MSEC(TIMER_PERIOD_MS), K_MSEC(TIMER_PERIOD_MS));

	k_thread_create(&thread_fast, stack_fast, STACK_SIZE,
			thread_fast_fn, NULL, NULL, NULL,
			PRIO_FAST, 0, K_NO_WAIT);
	k_thread_name_set(&thread_fast, "thread_fast");

	k_thread_create(&thread_worker, stack_worker, STACK_SIZE,
			thread_worker_fn, NULL, NULL, NULL,
			PRIO_WORKER, 0, K_NO_WAIT);
	k_thread_name_set(&thread_worker, "thread_worker");

	k_thread_create(&thread_io, stack_io, STACK_SIZE,
			thread_io_fn, NULL, NULL, NULL,
			PRIO_IO, 0, K_NO_WAIT);
	k_thread_name_set(&thread_io, "thread_io");

	k_thread_create(&thread_heartbeat, stack_heartbeat, STACK_SIZE,
			thread_heartbeat_fn, NULL, NULL, NULL,
			PRIO_HEARTBEAT, 0, K_NO_WAIT);
	k_thread_name_set(&thread_heartbeat, "thread_heartbeat");

	LOG_INF("Trace demo running.");

	while (1) {
		LOG_INF("fast=%u worker=%u io=%u isr=%u",
			fast_count, worker_count, io_count, isr_count);
#if TRACE_DEMO_STACK_LOG
		log_stack_margins();
#endif
		k_sleep(K_SECONDS(5));
	}

	return 0;
}
