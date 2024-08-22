#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

#include "esp_adc/adc_cali.h"

void app_main(void)
{
    // 1. Config ADC oneshot variables
    int adc_read0;
    int mv_output;
    int previous_adc_read = -1; // เก็บค่า ADC ที่อ่านได้ก่อนหน้า

    adc_oneshot_unit_handle_t handle = NULL;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &handle));

    // 2. Config ADC channel (ใช้ ADC_CHANNEL_4 สำหรับ GPIO32)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, ADC_CHANNEL_4, &config));

    // 3. Calibrate ADC
    adc_cali_handle_t cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));

    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(handle, ADC_CHANNEL_4, &adc_read0));
        
        if (adc_read0 != previous_adc_read) {
            printf("ADC Channel-4 raw read result: %d \n", adc_read0);
            adc_cali_raw_to_voltage(cali_handle, adc_read0, &mv_output);
            printf("ADC millivolt output: %d \n", mv_output);
            previous_adc_read = adc_read0; // เก็บค่าที่อ่านได้ล่าสุด
        } else {
            printf("No change in ADC value, still: %d \n", adc_read0);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    } // end of while loop
} // end of app_main()
