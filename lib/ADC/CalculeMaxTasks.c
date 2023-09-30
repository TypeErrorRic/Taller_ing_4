#include <ADC.h>

static const char *TAG = "Max Value Data";

// Definición de las Tareas:
taskDefinition taskCorrMaxI;
taskDefinition taskVoltMaxV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrMaxI;
TaskHandle_t xTaskVoltMaxV;

//Regulador de acceso:
SemaphoreHandle_t xReadCount1;
SemaphoreHandle_t xReadCount2;

static void vCorrMaxProcess(void *pvArguments)
{
    unsigned short contador = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Bucle principal
    for (;;)
    {
        ESP_LOGW(TAG, "Calculando Max I.");
        //Sección critica de lectura de datos:
        if(uxSemaphoreGetCount(xReadCount1) == 1) xSemaphoreTake(xWriteProcessMutex1, (TickType_t)5);
        xSemaphoreTake(xReadCount1, (TickType_t) FACTOR_ESPERA);
        //Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato Maximo:
            contador = (pxParameters->pxdata)->listADC_I[contador] < ((pxParameters->pxdata)->listADC_I[i]) ? i : contador;
            //Prueba:
            printf(">I:%i\t", (int)((pxParameters->pxdata)->listADC_I[i]));
            printf(">T:%i\n", (int)(pxParameters->pxdata)->listT_I[i]);
        }
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        taskYIELD();
        //Ceder llave:
        xSemaphoreGive(xReadCount1);
        //Activar la escritura de datos:
        if(uxSemaphoreGetCount(xReadCount1) == 1) xSemaphoreGive(xWriteProcessMutex1);
        //Realizar Calculos:
        /*-------------------*/
        ESP_LOGI(TAG, "Finalizado Max I");
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltMaxProcess(void *pvArguments)
{
    unsigned short contador = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Bucle principal
    for (;;)
    {
        ESP_LOGW(TAG, "Calculando Max V.");
        //Sección critica de lectura de datos:
        if(uxSemaphoreGetCount(xReadCount2) == 1) xSemaphoreTake(xWriteProcessMutex2, (TickType_t)5);
        xSemaphoreTake(xReadCount2, (TickType_t) FACTOR_ESPERA);
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato Maximo:
            contador = (pxParameters->pxdata)->listADC_V[contador] < ((pxParameters->pxdata)->listADC_V[i]) ? i : contador;
            //Prueba:
            printf(">V:%i\t", (int)((pxParameters->pxdata)->listADC_V[i]));
            printf(">T:%i\n", (int)(pxParameters->pxdata)->listT_V[i]);
        }
        taskYIELD();
        //Ceder llave:
        xSemaphoreGive(xReadCount2);
        //Activar la escritura de datos:
        if(uxSemaphoreGetCount(xReadCount2) == 1) xSemaphoreGive(xWriteProcessMutex2);
        //Realizar Calculos:
        /*-------------------*/
        ESP_LOGI(TAG, "Finalizado Max V");
        // Suspender:
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeProcess()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskCorrMaxI.taskId = vCorrMaxProcess;
    taskCorrMaxI.name = "corrienteProcess";
    taskCorrMaxI.pvParameters = pxADCParameters;
    taskCorrMaxI.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskCorrMaxI.uxPriority = 5; // Configurar la prioriodad.
    taskCorrMaxI.pvCreatedTask = &xTaskCorrMaxI;
    taskCorrMaxI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskVoltMaxV.taskId = vVoltMaxProcess;
    taskVoltMaxV.name = "voltProcess";
    taskVoltMaxV.pvParameters = pxADCParameters;
    taskVoltMaxV.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskVoltMaxV.uxPriority = 5; // Configurar la prioriodad.
    taskVoltMaxV.pvCreatedTask = &xTaskVoltMaxV;
    taskVoltMaxV.iCore = 1;

    // Crea un mutex para controlar el aceeso de lectura.
    xReadCount1 = xSemaphoreCreateCounting(1, 0);
    xReadCount2 = xSemaphoreCreateCounting(1, 0);
}