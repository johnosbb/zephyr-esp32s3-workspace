#include <zephyr/linker/iterable_sections.h>

/* Registers HTTP resource descriptors for the "zephyr_web_service" service.
 * Without this, HTTP_RESOURCE_DEFINE entries won't be discoverable at runtime.
 */

ITERABLE_SECTION_ROM(http_resource_desc_zephyr_web_service, Z_LINK_ITERABLE_SUBALIGN)
