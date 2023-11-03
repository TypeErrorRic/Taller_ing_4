#include <ADC.h>

static const char *TAG = "Cor Value";
#define REF_VALUE (float)1.75

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
        //Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreTake(xWriteProcessMutex1, (TickType_t)FACTOR_ESPERA);
        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = (pxParameters->pxdata)->listADC_I[i];
            preValue = (pxParameters->pxdata)->listADC_I[i - 1];
            if ((preValue >= REF_VALUE && actualValue < REF_VALUE))
            {
                actualTime = (pxParameters->pxdata)->listT_I[i];
                preTime = (pxParameters->pxdata)->listT_I[i - 1];
                corrCorTime = preTime + (actualTime - preTime) * (REF_VALUE - preValue) / (actualValue - preValue);
                break;
            }
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreGive(xWriteProcessMutex1);
        // Ceder llave:
        xSemaphoreGive(xReadCount1);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Tomar llave de escritura de datos para P.A y P.R:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        //Tiempo para actualizar datos.
        vTaskDelay(1);
        //Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        //Guardar valor del Ángulo:
        pxADCParameters->dcorteRefIt = corrCorTime;

        //Finalizar Tarea:
        ESP_LOGI(TAG, "Fin Cor I");
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xWriteAngle) == 1)
        {
            // Ceder llave:
            xSemaphoreGive(xWriteAngle);
            //Activar tarea de ángulo:
            vTaskResume(xTaskAngle);
        }
        else xSemaphoreGive(xWriteAngle);
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltCor(void *pvArguments)
{
    // Valores de captura de dato:
    double voltCorTime = 0;
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
        //Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = (pxParameters->pxdata)->listADC_V[i];
            preValue = (pxParameters->pxdata)->listADC_V[i - 1];
            if ((preValue >= REF_VALUE && actualValue < REF_VALUE))
            {
                actualTime = (pxParameters->pxdata)->listT_V[i];
                preTime = (pxParameters->pxdata)->listT_V[i - 1];
                voltCorTime = preTime + (actualTime - preTime) * (REF_VALUE - preValue) / (actualValue - preValue);
                break;
            }
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreGive(xWriteProcessMutex2);
        // Ceder llave:
        xSemaphoreGive(xReadCount2);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Tomar llave de escritura de datos para P.A y P.R:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        //Tiempo para actualizar datos.
        vTaskDelay(1);
        //Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        //Guardar valor del Ángulo:
        pxADCParameters->dcorteRefVt = voltCorTime;

        //Finalizar Tarea:
        ESP_LOGI(TAG, "Fin Cor V");
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xWriteAngle) == 1)
        {
            // Ceder llave:
            xSemaphoreGive(xWriteAngle);
            //Activar tarea de ángulo:
            vTaskResume(xTaskAngle);
        }
        else xSemaphoreGive(xWriteAngle);
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