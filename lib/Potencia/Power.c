/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Brayan Arley Mena Trejo (brayan.mena@correounivalle.edu.co)
 * @brief Este Archivo se encarga de trasmitir las respuestas por alguno de los canales del Esp32.
 * @version 0.1
 * @date 2023-12-06
 *
 * @copyright Copyright (c) 2023
 */

#include <Power.h>
#include <math.h>

static const char *TAG = "Power trasmition";

// Definición de la tarea:
taskDefinition taskPower;

void createChannelDAC()
{
    dac_output_enable(DAC_CHAN_0);
    dac_output_enable(DAC_CHAN_1);
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = 0,
        .pull_down_en = 0};
    gpio_config(&io_conf);
}

static void vPowerTrasmition(void *pvParameters)
{
    double datosPower[2] = {};
    uint8_t active = 0;
    u_int8_t reactive = 0;
    double reactivef, activef = 0;
    for (;;)
    {
        // Recivir datos: Si no están listos se queda esperando.
        xQueueReceive(powerData, &datosPower, portMAX_DELAY);
        // Valores de la potencia:
        activef = fabs(3 * datosPower[ACTIVE]);
        reactivef = fabs(3 * datosPower[REACTIVE]);
        //Trasmitir la salida por los DACs:
        if (datosPower[ACTIVE] > DAC_MAX_VALUE)
            active = (uint8_t)(255);
        else
            active = (uint8_t)((activef / DAC_MAX_VALUE) * 255);

        if (reactivef > DAC_MAX_VALUE)
            reactive = (uint8_t)(255);
        else if (datosPower[REACTIVE] >= 0)
        {
            reactive = (uint8_t)((reactivef / DAC_MAX_VALUE) * 255);
            gpio_set_level(GPIO_PIN, 0);
        }
        else if (datosPower[REACTIVE] < 0)
        {
            reactive = (uint8_t)((reactivef / DAC_MAX_VALUE) * 255);
            gpio_set_level(GPIO_PIN, 1);
        }

        // Procesamiento para la salida:
        printf("Potencia Activa : %f\n", datosPower[ACTIVE]);
        printf("Potencia Reactiva : %f\n", datosPower[REACTIVE]);
        dac_output_voltage(DAC_CHAN_0, active);
        dac_output_voltage(DAC_CHAN_1, reactive);

        // Fin de la tarea:
        ESP_LOGI(TAG, "Tarea Power");
    }
}

void initElementsPower()
{
    taskPower.taskId = vPowerTrasmition;
    taskPower.name = "Power Salidas";
    taskPower.pvParameters = NULL;
    taskPower.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskPower.uxPriority = 3;
    taskPower.pvCreatedTask = NULL;
    taskPower.iCore = 1;
    createChannelDAC();
}