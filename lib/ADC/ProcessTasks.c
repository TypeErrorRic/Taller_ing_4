#include <ADC.h>

//Declaración de manejadores de la tarea:
TaskHandle_t xTaskCorrProcessI;
TaskHandle_t xTaskVoltProcessV;

//Argumentos transferidos a las tareas:
xADCParameters *pxADCParameters;

// Implementación de la tarea de procesamiento delos datos del ADC:
static void corrienteProcess(void *pvParameters)
{
    unsigned short contador = 0;
    //Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    //Bucle Principal:
    for (;;)
    {
        //Tomar la LLave:
        if (xSemaphoreTake(xMutexProcess1, (TickType_t)5) == pdTRUE)
        {
            uint32_t adc_value;
            if (xQueueReceive(adc1_queue, &adc_value, (TickType_t)0))
            {
                //Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
                (pxParameters->pxdata)->listADC_I[contador] = adc_value;
                if (xQueueReceive(time1_queue, &adc_value, (TickType_t)0))
                {
                    (pxParameters->pxdata)->listT_I[contador] = adc_value;
                }
            }
            else
            {
                contador = 0;
                xSemaphoreGive(xMutexProcess1);
                vTaskResume(xTaskCorrMaxI);
                vTaskSuspend(NULL);
            }
        }
    }
}

// Creación de la tarea para el ADC 2:
static void voltProcess(void *pvParameters)
{
    unsigned short contador = 0;
    //Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    //Bucle Principal:
    for (;;)
    {
        //Tomar la LLave:
        if (xSemaphoreTake(xMutexProcess2, portMAX_DELAY) == pdTRUE)
        {
            uint32_t adc_value;
            if (xQueueReceive(adc2_queue, &adc_value, (TickType_t)0))
            {
                //Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
                (pxParameters->pxdata)->listADC_V[contador] = adc_value;
                if (xQueueReceive(time1_queue, &adc_value, (TickType_t)0))
                {
                    (pxParameters->pxdata)->listT_V[contador] = adc_value;
                }
            }
            else
            {
                //Reiniciar contador y suspender tarea:
                contador = 0;
                xSemaphoreGive(xMutexProcess2);
                //Activar nuevas Tareas:
                vTaskResume(xTaskVoltMaxV);
                vTaskSuspend(NULL);
            }
        }
    }
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
    taskADCProcessI.taskId = corrienteProcess;
    taskADCProcessI.name = "corrienteProcess";
    taskADCProcessI.usStackDepth = SIZE_TASK_ADC;
    taskADCProcessI.pvParameters = pxADCParameters;
    taskADCProcessI.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCProcessI.pvCreatedTask = xTaskCorrProcessI;
    taskADCProcessI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskADCProcessV.taskId = voltProcess;
    taskADCProcessV.name = "voltProcess";
    taskADCProcessV.usStackDepth = SIZE_TASK_ADC;
    taskADCProcessV.pvParameters = pxADCParameters;
    taskADCProcessV.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCProcessV.pvCreatedTask = xTaskVoltProcessV;
    taskADCProcessV.iCore = 1;
}