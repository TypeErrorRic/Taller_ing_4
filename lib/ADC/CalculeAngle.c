#include <ADC.h>

static const char *TAG = "Angle Value";

// Definición de las Tareas:
taskDefinition taskAngle;

// Manejadores de las tareas:
TaskHandle_t xTaskAngle;

static void calculateAngle(void *pvArguments)
{
    double angle = 0;
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Calcular el angulo:
        angle = (pxParameters->dcorteRefVt - pxParameters->dcorteRefIt) * 360 * FRECUENCIA_SENAL;
        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);
        pxParameters->dAngle = angle;
        xSemaphoreGive(xPower3);
        // Prueba
        printf(">A:%f\n", pxParameters->dcorteRefVt);
        printf(">A:%f\n", pxParameters->dcorteRefIt);
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
    taskAngle.uxPriority = 10;
    taskAngle.pvCreatedTask = &xTaskAngle;
    taskAngle.iCore = 0;
}