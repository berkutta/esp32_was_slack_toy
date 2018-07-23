#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/ledc.h"

#include "servo.h"

static char tag[] = "servo";

void servo_init(void) {
	int bitSize         = 15;
	int minValue        = 500;  // micro seconds (uS)
	int maxValue        = 2500; // micro seconds (uS)
	int sweepDuration   = 1500; // milliseconds (ms)
	int duty            = (1<<bitSize) * minValue / 20000 ;//1638
	int direction       = 1; // 1 = up, -1 = down
	int valueChangeRate = 20; // msecs

	ESP_LOGD(tag, ">> task_servo1");
	ledc_timer_config_t timer_conf;
	timer_conf.bit_num    = LEDC_TIMER_15_BIT;
	timer_conf.freq_hz    = 50;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = LEDC_TIMER_0;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.duty       = duty;
	ledc_conf.gpio_num   = 16;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_channel_config(&ledc_conf);

	int changesPerSweep = sweepDuration / valueChangeRate;// 1500/20 -> 75
	int changeDelta = (maxValue-minValue) / changesPerSweep;// 2000/75 -> 26
	int i;
	ESP_LOGD(tag, "sweepDuration: %d seconds", sweepDuration);
	ESP_LOGD(tag, "changesPerSweep: %d", changesPerSweep);
	ESP_LOGD(tag, "changeDelta: %d", changeDelta);
	ESP_LOGD(tag, "valueChangeRate: %d", valueChangeRate);

    servo_set(servo_low);
}

void servo_set(uint8_t position) {
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 26 * position);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void servo_run(void) {
    for(int i = 0; i <= 3; i++) {
        servo_set(servo_low);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        servo_set(servo_high);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    servo_set(servo_low);
}