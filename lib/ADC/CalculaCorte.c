#include <ADC.h>

static const char *TAG = "Cor Value";

// Definición de las Tareas:
taskDefinition taskCorrCorI;
taskDefinition taskVoltCorV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrCorI;
TaskHandle_t xTaskVoltCorV;

static void vCorrCor(void *pvArguments)
{
    //Valores de captura de dato:
    int actualValue = 0;
    int preValue = 0;
    double corrValue[2];
    double corrTime[2];
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        ESP_LOGW(TAG, "Corte I.");
        //Sección critica de lectura de datos:
        if(uxSemaphoreGetCount(xReadCount1) == 2) xSemaphoreTake(xWriteProcessMutex1, (TickType_t)5);
        xSemaphoreTake(xReadCount1, (TickType_t) FACTOR_ESPERA);
        //Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = ((pxParameters->pxdata)->listADC_V[i] - 2048);
            preValue = ((pxParameters->pxdata)->listADC_V[i - 1] - 2048);
            if((actualValue*preValue) < 0)
            {
                corrValue[0] = ((double) (3.3 / 4096) * (pxParameters->pxdata)->listADC_I[i - 1]) - 1.65;
                corrTime[0] = (pxParameters->pxdata)->listT_I[i - 1];
                corrValue[1] = ((double) (3.3 / 4096) * (pxParameters->pxdata)->listADC_I[i]) - 1.65;
                corrTime[1] = (pxParameters->pxdata)->listT_I[i];
                break;
            }
        }
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(3); //Minimo de 3 para que no se dañe el sistema
        //Ceder llave:
        xSemaphoreGive(xReadCount1);
        //Activar la escritura de datos:
        if(uxSemaphoreGetCount(xReadCount1) == 2) xSemaphoreGive(xWriteProcessMutex1);
        //Realizar Calculos:
        /*-------------------*/
        ESP_LOGI(TAG, "Fin Cor I");
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltCor(void *pvArguments)
{
    //Valores de captura de dato:
    int actualValue = 0;
    int preValue = 0;
    double corrValue[2];
    double corrTime[2];
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        ESP_LOGW(TAG, "Corte V.");
        //Sección critica de lectura de datos:
        if(uxSemaphoreGetCount(xReadCount2) == 2) xSemaphoreTake(xWriteProcessMutex2, (TickType_t)5);
        xSemaphoreTake(xReadCount2, (TickType_t) FACTOR_ESPERA);
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            actualValue = ((pxParameters->pxdata)->listADC_V[i] - 2048);
            preValue = ((pxParameters->pxdata)->listADC_V[i - 1] - 2048);
            if((actualValue*preValue) < 0)
            {
                corrValue[0] = ((double) (3.3 / 4096) * (pxParameters->pxdata)->listADC_V[i - 1]) - 1.65;
                corrTime[0] = (pxParameters->pxdata)->listT_V[i - 1];
                corrValue[1] = ((double) (3.3 / 4096) * (pxParameters->pxdata)->listADC_V[i]) - 1.65;
                corrTime[1] = (pxParameters->pxdata)->listT_V[i];
                break;
            }
        }
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(3); //Minimo de 3 para que no se dañe el sistema
        //Ceder llave:
        xSemaphoreGive(xReadCount2);
        //Activar la escritura de datos:
        if(uxSemaphoreGetCount(xReadCount2) == 2) xSemaphoreGive(xWriteProcessMutex2);
        //Realizar Calculos:

        /*-------------------*/

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