
#include "sdkconfig.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "uni_esp32.h"

void bluepad32(void *arg)
{
	uni_esp32_main();
}

void app_main(void) {
	uni_esp32_main();
	//xTaskCreatePinnedToCore(bluepad32, "bluepad32", 1024 * 64, NULL, 3, NULL, tskNO_AFFINITY);
}
