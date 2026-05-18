#include "ap_sta_api.h"

// ─── Main ────────────────────────────────────────────────────────────────────

void app_main(void) {
    nvs_init();
    wifi_init_softap();
    start_webserver();
}
