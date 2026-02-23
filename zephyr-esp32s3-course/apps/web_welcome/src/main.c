#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <string.h>

LOG_MODULE_REGISTER(web_welcome, LOG_LEVEL_INF);

#define HTTP_PORT 80

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_ready, 0, 1);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define READY_BLINK_COUNT 3
#define READY_BLINK_ON_MS 300
#define READY_BLINK_OFF_MS 300

static const uint8_t index_html[] =
"<!doctype html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"  <meta charset=\"utf-8\" />\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
"  <title>Zephyr Networking</title>\n"
"  <style>\n"
"    :root {\n"
"      --bg1: #0b1020;\n"
"      --bg2: #0f172a;\n"
"      --accent: #38bdf8;\n"
"      --accent2: #22c55e;\n"
"      --text: #e2e8f0;\n"
"      --muted: #94a3b8;\n"
"    }\n"
"    * { box-sizing: border-box; }\n"
"    body {\n"
"      margin: 0;\n"
"      min-height: 100vh;\n"
"      font-family: \"Space Grotesk\", \"Segoe UI\", system-ui, sans-serif;\n"
"      color: var(--text);\n"
"      background:\n"
"        radial-gradient(1200px 600px at 10% -10%, rgba(56, 189, 248, 0.25), transparent 60%),\n"
"        radial-gradient(1000px 500px at 110% 10%, rgba(34, 197, 94, 0.18), transparent 55%),\n"
"        linear-gradient(135deg, var(--bg1), var(--bg2));\n"
"      display: grid;\n"
"      place-items: center;\n"
"      padding: 24px;\n"
"    }\n"
"    .card {\n"
"      max-width: 720px;\n"
"      width: 100%;\n"
"      background: rgba(15, 23, 42, 0.7);\n"
"      border: 1px solid rgba(148, 163, 184, 0.2);\n"
"      border-radius: 20px;\n"
"      padding: 28px;\n"
"      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.35);\n"
"      backdrop-filter: blur(6px);\n"
"    }\n"
"    .badge {\n"
"      display: inline-block;\n"
"      padding: 6px 12px;\n"
"      border-radius: 999px;\n"
"      font-size: 12px;\n"
"      letter-spacing: 0.08em;\n"
"      text-transform: uppercase;\n"
"      color: #0b1020;\n"
"      background: linear-gradient(90deg, var(--accent), var(--accent2));\n"
"    }\n"
"    h1 {\n"
"      margin: 16px 0 10px;\n"
"      font-size: 32px;\n"
"      line-height: 1.1;\n"
"    }\n"
"    p {\n"
"      margin: 0;\n"
"      color: var(--muted);\n"
"      font-size: 16px;\n"
"      line-height: 1.6;\n"
"    }\n"
"    .pill {\n"
"      margin-top: 18px;\n"
"      display: inline-flex;\n"
"      align-items: center;\n"
"      gap: 10px;\n"
"      padding: 10px 14px;\n"
"      border-radius: 12px;\n"
"      background: rgba(56, 189, 248, 0.12);\n"
"      border: 1px solid rgba(56, 189, 248, 0.25);\n"
"      font-size: 14px;\n"
"      color: var(--text);\n"
"    }\n"
"    .dot {\n"
"      width: 10px;\n"
"      height: 10px;\n"
"      border-radius: 50%;\n"
"      background: var(--accent2);\n"
"      box-shadow: 0 0 16px rgba(34, 197, 94, 0.8);\n"
"    }\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"  <div class=\"card\">\n"
"    <span class=\"badge\">Zephyr RTOS</span>\n"
"    <h1>Welcome to Zephyr networking</h1>\n"
"    <p>\n"
"      Your device is serving this page over Wi-Fi using the Zephyr HTTP server.\n"
"      This first example is intentionally minimal: connect, get an IP, and serve a single page.\n"
"      Future lessons will add control and live status updates.\n"
"    </p>\n"
"    <div class=\"pill\"><span class=\"dot\"></span>HTTP server is running</div>\n"
"  </div>\n"
"</body>\n"
"</html>\n";

static struct http_resource_detail_static index_html_resource_detail = {
	.common = {
			.type = HTTP_RESOURCE_TYPE_STATIC,
			.bitmask_of_supported_http_methods = BIT(HTTP_GET),
			.content_type = "text/html; charset=utf-8",
		},
	.static_data = index_html,
	.static_data_len = sizeof(index_html) - 1,
};

static uint16_t web_port = HTTP_PORT;
HTTP_SERVICE_DEFINE(zephyr_web_service, NULL, &web_port, 1, 10, NULL);

HTTP_RESOURCE_DEFINE(index_html_resource, zephyr_web_service, "/",
		     &index_html_resource_detail);
HTTP_RESOURCE_DEFINE(index_html_resource_alias, zephyr_web_service, "/index.html",
		     &index_html_resource_detail);

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
			       uint32_t mgmt_event, struct net_if *iface)
{
	const struct wifi_status *status = cb->info;

	ARG_UNUSED(iface);

	if (mgmt_event != NET_EVENT_WIFI_CONNECT_RESULT) {
		return;
	}

	if (status->status) {
		LOG_ERR("Wi-Fi connect failed (%d)", status->status);
		return;
	}

	LOG_INF("Wi-Fi connected");
	k_sem_give(&wifi_connected);
}

static void ipv4_event_handler(struct net_mgmt_event_callback *cb,
			       uint32_t mgmt_event, struct net_if *iface)
{
	char addr_buf[NET_IPV4_ADDR_LEN];

	ARG_UNUSED(cb);

	if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
		return;
	}

	for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
		if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type != NET_ADDR_DHCP) {
			continue;
		}

		LOG_INF("IPv4 address: %s",
			net_addr_ntop(AF_INET,
				      &iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr,
				      addr_buf, sizeof(addr_buf)));
		LOG_INF("Browser: http://%s/", addr_buf);
		k_sem_give(&ipv4_ready);
		break;
	}
}

static int connect_wifi_and_wait_for_ip(void)
{
	struct net_if *iface = net_if_get_default();
	struct wifi_connect_req_params params = { 0 };
	int ret;

	if (iface == NULL) {
		LOG_ERR("No default network interface");
		return -ENODEV;
	}

	net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
				     NET_EVENT_WIFI_CONNECT_RESULT);
	net_mgmt_add_event_callback(&wifi_cb);

	net_mgmt_init_event_callback(&ipv4_cb, ipv4_event_handler,
				     NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&ipv4_cb);

	params.ssid = CONFIG_WIFI_CREDENTIALS_STATIC_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_CREDENTIALS_STATIC_SSID);
	params.psk = CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD;
	params.psk_length = strlen(CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD);
	params.security = WIFI_SECURITY_TYPE_PSK;

	LOG_INF("Connecting to Wi-Fi SSID \"%s\"...", CONFIG_WIFI_CREDENTIALS_STATIC_SSID);

	net_if_up(iface);
	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
	if (ret < 0) {
		LOG_ERR("Wi-Fi connect request failed (%d)", ret);
		return ret;
	}

	k_sem_take(&wifi_connected, K_FOREVER);

	LOG_INF("Starting DHCPv4...");
	net_dhcpv4_start(iface);
	k_sem_take(&ipv4_ready, K_FOREVER);

	return 0;
}

static void blink_ready(void)
{
	if (!gpio_is_ready_dt(&led0)) {
		LOG_WRN("LED gpio not ready; skip blink");
		return;
	}

	if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_WRN("LED gpio config failed; skip blink");
		return;
	}

	LOG_INF("Blinking LED on GPIO%u", (unsigned)led0.pin);
	for (int i = 0; i < READY_BLINK_COUNT; i++) {
		(void)gpio_pin_set_dt(&led0, 1);
		k_msleep(READY_BLINK_ON_MS);
		(void)gpio_pin_set_dt(&led0, 0);
		k_msleep(READY_BLINK_OFF_MS);
	}
}

int main(void)
{
	if (connect_wifi_and_wait_for_ip() == 0) {
		blink_ready();
		LOG_INF("Starting HTTP server on port %u", web_port);
		http_server_start();
	}

	return 0;
}
