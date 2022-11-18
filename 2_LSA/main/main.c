// C Header file having Standard Input Output Functions
#include <stdio.h>

// Header files for Free RTOS - Real Time Operating Systems
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// SRA's custom header file including additional functions
#include "sra_board.h"
int optimum_duty_cycle = 60;
// tested Margin Value Constants for Black and White Surfaces
#define BLACK_MARGIN 0
#define WHITE_MARGIN 4095

// targetted Mapping Value Constants for Tested Margin Values
#define CONSTRAIN_LSA_LOW 0
#define CONSTRAIN_LSA_HIGH 1000

// pointer to a character array
static const char *TAG = "LSA_READINGS";

// main driver function
void app_main(void)
{
    // enable line sensor after checking optimal working state of ESP
    ESP_ERROR_CHECK(enable_line_sensor());
    ESP_ERROR_CHECK(enable_motor_driver(a, NORMAL_MODE));

    // Union containing line sensor readings
    line_sensor_array line_sensor_readings;

#ifdef CONFIG_ENABLE_OLED
    // Declaring the required OLED struct
    u8g2_t oled_config;
    // Initialising the OLED
    ESP_ERROR_CHECK(init_oled(&oled_config));
#endif

    // infinite loop to get LSA readings continuously
    while (1)
    {
        // get line sensor readings from the LSA sensors
        line_sensor_readings = read_line_sensor();
        for (int i = 0; i < 5; i++)
        {
            // constrain lsa readings between BLACK_MARGIN and WHITE_MARGIN
            line_sensor_readings.adc_reading[i] = bound(line_sensor_readings.adc_reading[i], BLACK_MARGIN, WHITE_MARGIN);
            // map readings from (BLACK_MARGIN, WHITE_MARGIN) to (CONSTRAIN_LSA_LOW, CONSTRAIN_LSA_HIGH)
            line_sensor_readings.adc_reading[i] = map(line_sensor_readings.adc_reading[i], BLACK_MARGIN, WHITE_MARGIN, CONSTRAIN_LSA_LOW, CONSTRAIN_LSA_HIGH);
        }
        if (line_sensor_readings.adc_reading[1] && line_sensor_readings.adc_reading[2] && line_sensor_readings.adc_reading[3] < 500 && line_sensor_readings.adc_reading[0] && line_sensor_readings.adc_reading[4] > 500)
        {
            printf("straight");
            set_motor_speed(MOTOR_A_0, MOTOR_FORWARD, optimum_duty_cycle);
            set_motor_speed(MOTOR_A_1, MOTOR_FORWARD, optimum_duty_cycle+6);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else if (line_sensor_readings.adc_reading[2] && line_sensor_readings.adc_reading[3] < 500 && line_sensor_readings.adc_reading[0] && line_sensor_readings.adc_reading[4] && line_sensor_readings.adc_reading[1] > 500)
        {
            printf("left");
            set_motor_speed(MOTOR_A_0, MOTOR_FORWARD, optimum_duty_cycle+4);
            set_motor_speed(MOTOR_A_1, MOTOR_FORWARD, optimum_duty_cycle+6);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else if (line_sensor_readings.adc_reading[1] && line_sensor_readings.adc_reading[2] < 500 && line_sensor_readings.adc_reading[0] && line_sensor_readings.adc_reading[4] && line_sensor_readings.adc_reading[3] > 500)
        {
            printf("right");
            set_motor_speed(MOTOR_A_0, MOTOR_FORWARD, optimum_duty_cycle);
            set_motor_speed(MOTOR_A_1, MOTOR_FORWARD, optimum_duty_cycle+4+6);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            // stopping the MOTOR A0
            set_motor_speed(MOTOR_A_0, MOTOR_STOP, 0);

            // stopping the MOTOR A1
            set_motor_speed(MOTOR_A_1, MOTOR_STOP, 0);

            // adding delay of 100ms
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
#ifdef CONFIG_ENABLE_OLED
        // Displaying LSA Bar on OLED
        display_lsa(line_sensor_readings, &oled_config);
#endif
        // Displaying Information logs - final lsa readings
        ESP_LOGI(TAG, "LSA_1: %d \t LSA_2: %d \t LSA_3: %d \t LSA_4: %d \t LSA_5: %d", line_sensor_readings.adc_reading[0], line_sensor_readings.adc_reading[1], line_sensor_readings.adc_reading[2], line_sensor_readings.adc_reading[3], line_sensor_readings.adc_reading[4]);
        // delay of 1s after each log
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
