#include <ADC.h>

static const char *TAG = "Angle Value";

// Definición de las Tareas:
taskDefinition taskAngle;

// Manejadores de las tareas:
TaskHandle_t xTaskAngle;

static void calculateAngle(void *pvArguments)
{
    double times[2] = {};
    double angle = 0;
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Sección critica de escritura de datos:
        xSemaphoreTake(xWriteCount, (TickType_t)portMAX_DELAY);
        // Adquirir los valores en la cola:
        if (uxQueueSpacesAvailable(timesAng_queue) == 0)
        {
            for (int i = 0; i < 2; i++)
            {
                if (xQueueReceive(timesAng_queue, &times[i], (TickType_t)0) == pdTRUE)
                    ;
                else
                    break;
            }
        }
        // Ceder llave;
        xSemaphoreGive(xWriteCount);
        // Calcular el angulo:
        angle = (times[0] - times[1]) * 360 * FRECUENCIA_SENAL;

        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);
        pxParameters->dAngle = angle;
        xSemaphoreGive(xPower3);

        // Prueba
        printf(">A:%f\n", times[0]);
        printf(">A:%f\n", times[1]);
        printf(">A:%f\n", angle);
        ESP_LOGI(TAG, "Fin Angle");
        // Suspender sistema
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeAngle()
{
    // Definir la Tarea para calcular el angulo de desfase:
    taskAngle.taskId = calculateAngle;
    taskAngle.name = "anguloDesfase";
    taskAngle.pvParameters = pxADCParameters;
    taskAngle.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskAngle.uxPriority = 5; // Configurar la prioriodad.
    taskAngle.pvCreatedTask = &xTaskAngle;
    taskAngle.iCore = 0;
}