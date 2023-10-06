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

// Implementación de la tarea de procesamiento delos datos del ADC:
static void vCorrienteProcess(void *pvParameters)
{
    // Información sobre la captura
    unsigned int adc_value = 0;
    unsigned long time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    // Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Tomar las LLaves para para leer / escribir en el arreglo:
        xSemaphoreTake(xMutexProcess1, portMAX_DELAY);
        xSemaphoreTake(xWriteProcessMutex1, (TickType_t)FACTOR_ESPERA);
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc1_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_I[i] = adc_value;
                if (xQueueReceive(time1_queue, &time, (TickType_t)0))
                {
                    (pxParameters->pxdata)->listT_I[i] = time;
                    printf(">I:%i\t", (int)adc_value);
                    printf(">T:%i\n", (int)time);
                }
                else
                    break;
            }
            else
                break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI y xTaskCorrCorI:
        // while (eTaskGetState(xTaskCorrMaxI) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        // while (eTaskGetState(xTaskCorrCorI) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        // Liberar llaves:
        xSemaphoreGive(xWriteProcessMutex1); // Activar la lectura de datos:
        xSemaphoreGive(xMutexProcess1);      // Activar de nuevo la captura de datos:
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura I Completa");
        // Activar tareas:
        // vTaskResume(xTaskCorrMaxI);
        // vTaskResume(xTaskCorrCorI);
        vTaskDelay(FACTOR_ESPERA);
    }
    vTaskDelete(NULL);
}

// Creación de la tarea para el ADC 2:
static void vVoltajeProcess(void *pvParameters)
{
    // Información sobre la captura
    uint32_t adc_value = 0;
    unsigned long time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    // Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Tomar las LLave:
        xSemaphoreTake(xMutexProcess2, portMAX_DELAY);
        xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc2_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_V[i] = adc_value;
                if (xQueueReceive(time2_queue, &time, (TickType_t)0))
                {
                    (pxParameters->pxdata)->listT_V[i] = time;
                    printf(">V:%i\t", (int)adc_value);
                    printf(">T:%i\n", (int)time);
                }
                else
                    break;
            }
            else
                break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI
        // while (eTaskGetState(xTaskVoltMaxV) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        // while (eTaskGetState(xTaskVoltCorV) != eSuspended) vTaskDelay(FACTOR_ESPERA);
        // Liberar llaves:
        xSemaphoreGive(xMutexProcess2);      // Activar de nuevo la captura de datos:
        xSemaphoreGive(xWriteProcessMutex2); // Activar la lectura de datos:
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura v Completa");
        // Activar Tareas:
        // vTaskResume(xTaskVoltMaxV);
        // vTaskResume(xTaskVoltCorV);
        vTaskDelay(FACTOR_ESPERA);
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
}