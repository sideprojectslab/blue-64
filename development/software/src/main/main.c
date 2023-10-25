
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
}
