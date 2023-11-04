#include <Power.h>
#include <math.h>
#define PI 3.141592

static const char *TAG = "Power Value";

// Definición de las Tareas:
taskDefinition taskActivePower;
taskDefinition taskReactivePower;

// Manejadores de las tareas:
TaskHandle_t xTaskActivePower;
TaskHandle_t xTaskReactivePower;

static void calculateActivePower(void *pvArguments)
{
    double activePower = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        xSemaphoreTake(xPower1, (TickType_t)portMAX_DELAY);
        xSemaphoreTake(xPower2, (TickType_t)portMAX_DELAY);
        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);

        activePower = (pxParameters->dVmax) * (pxParameters->dImax) * cosh((pxParameters->dAngle) * (double)PI / 180);

        xSemaphoreGive(xPower1);
        xSemaphoreGive(xPower2);
        xSemaphoreGive(xPower3);

        printf(">PowerA:%f\n", activePower);
        ESP_LOGI(TAG, "Fin PowerA");
        // Suspender sistema
        vTaskSuspend(NULL);
    }
};

static void calculateReactivePower(void *pvArguments)
{
    double reactivePower = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        xSemaphoreTake(xPower1, (TickType_t)portMAX_DELAY);
        xSemaphoreTake(xPower2, (TickType_t)portMAX_DELAY);
        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);

        reactivePower = (pxParameters->dVmax) * (pxParameters->dImax) * sinh((pxParameters->dAngle) * (double)PI / 180);

        xSemaphoreGive(xPower1);
        xSemaphoreGive(xPower2);
        xSemaphoreGive(xPower3);

        printf(">PowerR:%f\n", reactivePower);
        ESP_LOGI(TAG, "Fin PowerR");
        // Suspender sistema
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculePower()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskActivePower.taskId = calculateActivePower;
    taskActivePower.name = "potenciaActiva";
    taskActivePower.pvParameters = pxADCParameters;
    taskActivePower.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskActivePower.uxPriority = 5; // Configurar la prioriodad.
    taskActivePower.pvCreatedTask = &xTaskActivePower;
    taskActivePower.iCore = 1;

    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskReactivePower.taskId = calculateReactivePower;
    taskReactivePower.name = "potenciaReactiva";
    taskReactivePower.pvParameters = pxADCParameters;
    taskReactivePower.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskReactivePower.uxPriority = 5; // Configurar la prioriodad.
    taskReactivePower.pvCreatedTask = &xTaskReactivePower;
    taskReactivePower.iCore = 0;
}

void initElementsPower()
{
    setupTaskCalculePower();
}