#include <ADC.h>

static const char *TAG = "Angle Value";

// Definición de las Tareas:
taskDefinition taskAngle;

// Manejadores de las tareas:
TaskHandle_t xTaskAngle;

static void calculateAngle(void *pvArguments)
{
    double angle = 0;
    double auxAngle = 0;
    double contador = 0;
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Calcular el angulo:
        for (unsigned short i = 0; i < 2; i++)
        {
            auxAngle = (pxParameters->dcorteRefVt[i] - pxParameters->dcorteRefIt[i]) * 360 * FRECUENCIA_SENAL;
            if (auxAngle > 0 && auxAngle > 90)
                ESP_LOGE(TAG, "Ángulo fuera del limite Mayor.");
            else if (auxAngle < 0 && auxAngle < 90)
                ESP_LOGE(TAG, "Ángulo fuera del limite Menor.");
            else if (auxAngle == 0)
                ESP_LOGE(TAG, "No se tomo la medida.");
            else
            {
                angle += auxAngle;
                contador++;
            }
            pxParameters->dcorteRefVt[i] = 0;
            pxParameters->dcorteRefIt[i] = 0;
        }
        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);
        if (contador != 0)
            pxParameters->dAngle = (angle / contador);
        else
            pxParameters->dAngle = 0;
        contador = 0;
        angle = 0;
        xSemaphoreGive(xPower3);
        // Prueba
        printf(">A:%f\n", pxParameters->dcorteRefVt[0]);
        printf(">A:%f\n", pxParameters->dcorteRefIt[0]);
        printf(">A:%f\n", pxParameters->dAngle);
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