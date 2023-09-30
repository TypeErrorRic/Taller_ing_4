//Archivos de cabecera para la operación del micro:
#include <stdio.h>
#include <string.h>

//Archivos de cabecera con la creación de las tareas:
#include "modulosConfig.h"

// Main:
static const char *TAG = "Main";

void app_main(void) {
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
    //Finalizar.
    ESP_LOGW(TAG, "Inicializacion correcta.");
}
