#include <ADC.h>

static const char *TAG = "Capture Data";

// Declaración de manejadores de la tarea:
TaskHandle_t xTaskCorrProcessI;
TaskHandle_t xTaskVoltProcessV;

// Derfinición de la Tarea:
taskDefinition taskADCProcessI;
taskDefinition taskADCProcessV;

// Argumentos transferidos a las tareas:
xADCParameters *pxADCParameters;

// Semaforo para manejar la escritura de los datos:
SemaphoreHandle_t xWriteProcessMutex1;
SemaphoreHandle_t xWriteProcessMutex2;

// Implementación de la tarea de procesamiento delos datos del ADC:
static void vCorrienteProcess(void *pvParameters)
{
    uint32_t adc_value = 0;
    double time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    // Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        ESP_LOGW(TAG, "Iniciando Procesamiento I.");
        // Tomar la LLave:
        xSemaphoreTake(xMutexProcess1, portMAX_DELAY);
        // Tomar la llave para escribir en el arreglo:
        xSemaphoreTake(xWriteProcessMutex1, (TickType_t) FACTOR_ESPERA); // Esperar 10 ticks para volverlo a intentar
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc1_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_I[i] = adc_value;
                if (xQueueReceive(time1_queue, &time, (TickType_t)0))
                    (pxParameters->pxdata)->listT_I[i] = time;
                else break;
            }
            else break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI y xTaskCorrCorI:
        while (eTaskGetState(xTaskCorrMaxI) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        while (eTaskGetState(xTaskCorrCorI) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        //Liberar llaves:
        xSemaphoreGive(xWriteProcessMutex1);    // Activar la lectura de datos:
        xSemaphoreGive(xMutexProcess1);         // Activar de nuevo la captura de datos:
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura I Completa");
        //Activar tareas:
        vTaskResume(xTaskCorrMaxI);
        vTaskResume(xTaskCorrCorI);
        // Suspender Tarea:
        vTaskSuspend(NULL);
    }
    vTaskDelete(NULL);
}

// Creación de la tarea para el ADC 2:
static void vVoltajeProcess(void *pvParameters)
{
    uint32_t adc_value = 0;
    double  time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    // Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        ESP_LOGW(TAG, "Iniciando Procesamiento V.");
        // Tomar las LLave:
        xSemaphoreTake(xMutexProcess2, portMAX_DELAY);
        xSemaphoreTake(xWriteProcessMutex2, (TickType_t) FACTOR_ESPERA);
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc2_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_V[i] = adc_value;
                if (xQueueReceive(time2_queue, &time, (TickType_t)0))
                    (pxParameters->pxdata)->listT_V[i] = time;
                else break;
            }
            else break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI
        while (eTaskGetState(xTaskVoltMaxV) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        while (eTaskGetState(xTaskVoltCorV) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        //Liberar llaves:
        xSemaphoreGive(xMutexProcess2);      // Activar de nuevo la captura de datos:
        xSemaphoreGive(xWriteProcessMutex2); // Activar la lectura de datos:
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura V Completa");
        // Activar Tareas:
        vTaskResume(xTaskVoltMaxV);
        vTaskResume(xTaskVoltCorV);
        vTaskSuspend(NULL);
    }
    vTaskDelete(NULL);
};

// Inicializando las tareas:
void setupTaskProcessADCs()
{
    // Definir un puntero a una estructura para pasa argumentos a la tarea.
    pxADCParameters = (xADCParameters *)pvPortMalloc(sizeof(xADCParameters));
    // Vincular un puntero a una estructura con los datos de captura.
    pxADCParameters->pxdata = (xCaptureParameters *)pvPortMalloc(sizeof(xCaptureParameters));
    // Inicializar los valores de los arreglos:
    for (short i = 0; i < QUEUE_LENGTH; i++)
    {
        (pxADCParameters->pxdata)->listADC_I[i] = (uint32_t)0;
        (pxADCParameters->pxdata)->listADC_V[i] = (uint32_t)0;
        (pxADCParameters->pxdata)->listT_I[i] = (uint32_t)0;
        (pxADCParameters->pxdata)->listT_V[i] = (uint32_t)0;
    }
    // Inicializar variables:
    pxADCParameters->dVmax = 0;
    pxADCParameters->dImax = 0;
    pxADCParameters->dcorteRefVt = 0;
    pxADCParameters->dcorteRefIt = 0;

    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskADCProcessI.taskId = vCorrienteProcess;
    taskADCProcessI.name = "corrienteProcess";
    taskADCProcessI.pvParameters = pxADCParameters;
    taskADCProcessI.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskADCProcessI.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCProcessI.pvCreatedTask = &xTaskCorrProcessI;
    taskADCProcessI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskADCProcessV.taskId = vVoltajeProcess;
    taskADCProcessV.name = "voltProcess";
    taskADCProcessV.pvParameters = pxADCParameters;
    taskADCProcessV.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskADCProcessV.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCProcessV.pvCreatedTask = &xTaskVoltProcessV;
    taskADCProcessV.iCore = 1;

    // Crea un mutex para controlar la escritura de de datos en el arreglo:
    xWriteProcessMutex1 = xSemaphoreCreateMutex();
    xWriteProcessMutex2 = xSemaphoreCreateMutex();
}