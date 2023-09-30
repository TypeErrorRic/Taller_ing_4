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
    uint32_t adc_value = 0;
    double time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    //Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Tomar la LLave:
        if (xSemaphoreTake(xMutexProcess1, (TickType_t)5) == pdTRUE)
        {
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
                    else break;
                }
                else break;
            }
            ESP_LOGI(TAG, "Captura I Completa");
            // Suspender tarea hasta que la cola vuelva a estar lista.
            xSemaphoreGive(xMutexProcess1);
            // Activar nuevas Tareas:
            //vTaskResume(xTaskCorrMaxI);
            vTaskSuspend(NULL);
        }
    }
    vTaskDelete(NULL);
}

// Creación de la tarea para el ADC 2:
static void vVoltajeProcess(void *pvParameters)
{
    uint32_t adc_value = 0;
    double time = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    //Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Tomar la LLave:
        if (xSemaphoreTake(xMutexProcess2, portMAX_DELAY) == pdTRUE)
        {
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
                    else break;
                }
                else break;
            }
            ESP_LOGI(TAG, "Captura V Completa");
            // Suspender tarea hasta que la cola vuelva a estar lista.
            xSemaphoreGive(xMutexProcess2);
            // Activar nuevas Tareas:
            //vTaskResume(xTaskVoltMaxV);
            vTaskSuspend(NULL);
        }
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