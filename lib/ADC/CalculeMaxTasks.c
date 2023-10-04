#include <ADC.h>

static const char *TAG = "Max Value";

// Definición de las Tareas:
taskDefinition taskCorrMaxI;
taskDefinition taskVoltMaxV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrMaxI;
TaskHandle_t xTaskVoltMaxV;

// Regulador de acceso:
SemaphoreHandle_t xReadCount1;
SemaphoreHandle_t xReadCount2;

static void vCorrMaxProcess(void *pvArguments)
{
    unsigned short maxdirection = 0;
    double maxValue[3];
    double maxTime[3];
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 2)
            xSemaphoreTake(xWriteProcessMutex1, (TickType_t)5);
        xSemaphoreTake(xReadCount1, (TickType_t)FACTOR_ESPERA);
        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < (QUEUE_LENGTH - 1); i++)
        {
            // Capturar el dato Maximo:
            maxdirection = (pxParameters->pxdata)->listADC_I[maxdirection] < ((pxParameters->pxdata)->listADC_I[i]) ? i : maxdirection;
        }
        // Valor anterior al valor maximo:
        maxValue[0] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_I[maxdirection - 1];
        maxTime[0] = ((pxParameters->pxdata)->listT_I[maxdirection - 1]) / configTICK_RATE_HZ;
        // Valor maximo:
        maxValue[1] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_I[maxdirection];
        maxTime[1] = (double)(pxParameters->pxdata)->listT_I[maxdirection] / configTICK_RATE_HZ;
        // Valor después del valor maximo.
        maxValue[2] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_I[maxdirection + 1];
        maxTime[2] = (double)(pxParameters->pxdata)->listT_I[maxdirection + 1] / configTICK_RATE_HZ;
        // Ceder llave:
        xSemaphoreGive(xReadCount1);
        // Activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 2)
            xSemaphoreGive(xWriteProcessMutex1);

        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(3); // Minimo de 3 para que no se dañe el sistema

        // Realizar Calculos:

        // Prueba:
        printf(">I:%f\t", maxValue[1]);
        printf(">T:%f\n", maxTime[1]);

        /*-------------------*/

        ESP_LOGI(TAG, "Fin Max I");
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltMaxProcess(void *pvArguments)
{
    unsigned short maxdirection = 0;
    double maxValue[3] = {};
    double maxTime[3] = {};
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 2)
            xSemaphoreTake(xWriteProcessMutex2, (TickType_t)5);
        xSemaphoreTake(xReadCount2, (TickType_t)FACTOR_ESPERA);
        for (unsigned short i = 1; i < (QUEUE_LENGTH - 1); i++)
        {
            // Capturar el dato Maximo:
            maxdirection = (pxParameters->pxdata)->listADC_V[maxdirection] < ((pxParameters->pxdata)->listADC_V[i]) ? i : maxdirection;
        }
        // Valor anterior al valor maximo:
        maxValue[0] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_V[maxdirection - 1];
        maxTime[0] = (double)(pxParameters->pxdata)->listT_V[maxdirection - 1] / configTICK_RATE_HZ;
        // Valor maximo:
        maxValue[1] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_V[maxdirection];
        maxTime[1] = (double)(pxParameters->pxdata)->listT_V[maxdirection] / configTICK_RATE_HZ;
        // Valor después del valor maximo.
        maxValue[2] = (double)(3.3 / 4096) * (pxParameters->pxdata)->listADC_V[maxdirection + 1];
        maxTime[2] = (double)(pxParameters->pxdata)->listT_V[maxdirection + 1] / configTICK_RATE_HZ;
        // Ceder llave:
        xSemaphoreGive(xReadCount2);
        // Activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 2)
            xSemaphoreGive(xWriteProcessMutex2);

        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(3); // Minimo de 3 para que no se dañe el sistema

        // Realizar Calculos:

        // Prueba:
        printf(">V:%f\t", maxValue[1]);
        printf(">T:%f\n", maxTime[1]);

        /*-------------------*/

        ESP_LOGI(TAG, "Fin Max V");
        // Suspender:
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeProcess()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskCorrMaxI.taskId = vCorrMaxProcess;
    taskCorrMaxI.name = "corrienteMax";
    taskCorrMaxI.pvParameters = pxADCParameters;
    taskCorrMaxI.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskCorrMaxI.uxPriority = 5; // Configurar la prioriodad.
    taskCorrMaxI.pvCreatedTask = &xTaskCorrMaxI;
    taskCorrMaxI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskVoltMaxV.taskId = vVoltMaxProcess;
    taskVoltMaxV.name = "voltMax";
    taskVoltMaxV.pvParameters = pxADCParameters;
    taskVoltMaxV.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskVoltMaxV.uxPriority = 5; // Configurar la prioriodad.
    taskVoltMaxV.pvCreatedTask = &xTaskVoltMaxV;
    taskVoltMaxV.iCore = 1;

    // Crea un mutex para controlar el aceeso de lectura.
    xReadCount1 = xSemaphoreCreateCounting(2, 0);
    xReadCount2 = xSemaphoreCreateCounting(2, 0);
}