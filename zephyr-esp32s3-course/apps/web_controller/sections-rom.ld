#include <zephyr/linker/iterable_sections.h>

/* Registers HTTP resource descriptors for the "web_controller_service" service. */
ITERABLE_SECTION_ROM(http_resource_desc_web_controller_service, Z_LINK_ITERABLE_SUBALIGN)
