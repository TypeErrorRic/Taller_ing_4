#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

void app_main(void) {
    // Mensaje que deseas enviar
    const char *mensaje = "Hola Mundo.\n";

    while (1) {
        // Envía el mensaje al puerto COM (serial)
        printf("%s", mensaje);

        // Espera unos segundos antes de enviar otro mensaje (ajusta según tus necesidades)
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
