//Archivos de cabecera para la operación del micro:
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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

    // Mensaje que deseas enviar
    const char *mensaje = "Hola Mundo.\n";

    while (1) {
        // Envía el mensaje al puerto COM (serial)
        printf("%s", mensaje);

        // Espera unos segundos antes de enviar otro mensaje (ajusta según tus necesidades)
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    return 0;
}
