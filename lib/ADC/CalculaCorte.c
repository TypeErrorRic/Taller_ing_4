#include <ADC.h>

static const char *TAG = "Cor Value";
#define REF_VALUE 0.45

// Definición de las Tareas:
taskDefinition taskCorrCorI;
taskDefinition taskVoltCorV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrCorI;
TaskHandle_t xTaskVoltCorV;

static void vCorrCor(void *pvArguments)
{
    // Valores de captura de dato:
    double corrCorTime = 0;
    float refValue = REF_VALUE;
    double actualValue = 0;
    double preValue = 0;
    double actualTime = 0;
    double preTime = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        corrCorTime = 0;
        // Tomar Semaforo:
        xSemaphoreTake(xReadCount1, (TickType_t)FACTOR_ESPERA);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreTake(xWriteProcessMutex1, (TickType_t)FACTOR_ESPERA);
        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = (pxParameters->pxdata)->listADC_I[i];
            preValue = (pxParameters->pxdata)->listADC_I[i - 1];
            if ((preValue >= refValue && actualValue < refValue))
            {
                actualTime = (pxParameters->pxdata)->listT_I[i];
                preTime = (pxParameters->pxdata)->listT_I[i - 1];
                corrCorTime = preTime + (actualTime - preTime) * (refValue - preValue) / (actualValue - preValue);
                break;
            }
            if ((i + 1 == QUEUE_LENGTH) && (pxParameters->pxdata)->listADC_I[QUEUE_LENGTH - 1] == refValue)
                corrCorTime = (pxParameters->pxdata)->listT_I[QUEUE_LENGTH - 1];
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreGive(xWriteProcessMutex1);
        // Ceder llave:
        xSemaphoreGive(xReadCount1);
        // Sección critica de escritura de datos:
        xSemaphoreTake(xWriteCount, (TickType_t)FACTOR_ESPERA);
        // Guardar en la cola el valor calculado:
        xQueueSendToBack(timesAng_queue, &corrCorTime, portMAX_DELAY);
        // Ceder llave;
        xSemaphoreGive(xWriteCount);
        // Verificar si la cola esta llena
        if (uxQueueSpacesAvailable(timesAng_queue) == 0)
            // Activar la tarea de calcular el angulo
            vTaskResume(xTaskAngle);
        // Prueba:
        /*if (corrCorTime != 0)
            printf(">TIc:%f\n", corrCorTime);*/
        ESP_LOGI(TAG, "Fin Cor I");
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltCor(void *pvArguments)
{
    // Valores de captura de dato:
    double voltCorTime = 0;
    float refValue = REF_VALUE;
    double actualValue = 0;
    double preValue = 0;
    double actualTime = 0;
    double preTime = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        voltCorTime = 0;
        // Tomar Semaforo:
        xSemaphoreTake(xReadCount2, (TickType_t)FACTOR_ESPERA);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = (pxParameters->pxdata)->listADC_V[i];
            preValue = (pxParameters->pxdata)->listADC_V[i - 1];
            if ((preValue >= refValue && actualValue < refValue))
            {
                actualTime = (pxParameters->pxdata)->listT_V[i];
                preTime = (pxParameters->pxdata)->listT_V[i - 1];
                voltCorTime = preTime + (actualTime - preTime) * (refValue - preValue) / (actualValue - preValue);
                break;
            }
            if ((i + 1 == QUEUE_LENGTH) && (pxParameters->pxdata)->listADC_V[QUEUE_LENGTH - 1] == refValue)
                voltCorTime = (pxParameters->pxdata)->listT_V[QUEUE_LENGTH - 1];
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreGive(xWriteProcessMutex2);
        // Ceder llave:
        xSemaphoreGive(xReadCount2);
        // Sección critica de escritura de datos:
        xSemaphoreTake(xWriteCount, (TickType_t)FACTOR_ESPERA);
        // Guardar en la cola el valor calculado:
        xQueueSendToBack(timesAng_queue, &voltCorTime, portMAX_DELAY);
        // Ceder llave:
        xSemaphoreGive(xWriteCount);
        // Verificar si la cola esta llena
        if (uxQueueSpacesAvailable(timesAng_queue) == 0)
            // Activar la tarea de calcular el angulo
            vTaskResume(xTaskAngle);
        // Prueba
        /*if (voltCorTime != 0)
            printf(">TVc:%f\n", voltCorTime);*/
        ESP_LOGI(TAG, "Fin Cor V");
        // Suspender:
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeCorProcess()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskCorrCorI.taskId = vCorrCor;
    taskCorrCorI.name = "corrienteCor";
    taskCorrCorI.pvParameters = pxADCParameters;
    taskCorrCorI.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskCorrCorI.uxPriority = 5; // Configurar la prioriodad.
    taskCorrCorI.pvCreatedTask = &xTaskCorrCorI;
    taskCorrCorI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskVoltCorV.taskId = vVoltCor;
    taskVoltCorV.name = "voltCor";
    taskVoltCorV.pvParameters = pxADCParameters;
    taskVoltCorV.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskVoltCorV.uxPriority = 5; // Configurar la prioriodad.
    taskVoltCorV.pvCreatedTask = &xTaskVoltCorV;
    taskVoltCorV.iCore = 1;
}