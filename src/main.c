// Archivos de cabecera para la operación del micro:
#include <stdio.h>
#include <string.h>

// Archivos de cabecera con la creación de las tareas:
#include "modulosConfig.h"

// Main:
static const char *TAG = "Main";

//Sincronizador de tareas:
SemaphoreHandle_t syncSemaphore;

void app_main(void)
{
    // Inicializar el semáforo
    syncSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(syncSemaphore); // Inicialmente, el semáforo está en estado "libre"

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
    // Finalizar.
    ESP_LOGW(TAG, "Inicializacion correcta.");

    // Iniciar tareas:
    // Deshabilitar la programación preemptiva antes de reanudar tareas
    vTaskSuspendAll();

    // Reanudar tareas
    vTaskResume(xTaskCorrCaptureI);
    vTaskResume(xTaskVoltCaptureV);

    // Habilitar nuevamente la programación preemptiva
    xTaskResumeAll();
}
