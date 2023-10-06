// Archivos de cabecera para la operación del micro:
#include <stdio.h>
#include <string.h>

// Archivos de cabecera con la creación de las tareas:
#include "modulosConfig.h"

// Main:
static const char *TAG = "Main";

void app_main(void)
{
    // Inicializar Perifericos del sistema:
    initDrivers();
    // Arrancar las tareas.
    switch (createTask())
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Tareas creadas con exito.");
        break;
    default:
        printf(TAG, "Error!!!");
        break;
    }
    // Iniciar timers:
    ESP_ERROR_CHECK(gptimer_start(gptimer1));
    ESP_ERROR_CHECK(gptimer_start(gptimer2));
    ESP_LOGI(TAG, "Timers iniciados.");
    // Tiempo para que los timers tomen la llave.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Iniciar Tareas de procesamiento:
    vTaskResume(xTaskCorrProcessI);
    vTaskResume(xTaskVoltProcessV);
    ESP_LOGI(TAG, "Tareas de procesamiento listas.");
    // Finalizar.
    ESP_LOGI(TAG, "Inicializacion correcta.");
}
