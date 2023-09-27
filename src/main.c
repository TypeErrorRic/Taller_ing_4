//Archivos de cabecera para la operación del micro:
#include <stdio.h>
#include <string.h>

//Archivos de cabecera con la creación de las tareas:
#include "modulosConfig.h"

void app_main(void) {
    // Inicializar el micro
    initDrivers();
    // Arrancar las tareas.
    switch (createTask())
    {
    case ESP_OK:
        printf("Inicializado Correctamente");
        break;
    default:
        printf("Error!!!");
        break;
    }
    // Iniciar el scheduler de FreeRTOS
    vTaskStartScheduler();

    while (1);
}
