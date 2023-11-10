/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este archivo se encarga de procesar los resultados obtenidos del muestreo.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

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

// Definir retardo:
#define DELAY_TIME (FRECUENCIA > 10 ? FACTOR_ESPERA : ((int)1000 / portTICK_PERIOD_MS))

// Definir la Compensación entre los dos Canales.
#define COMPENSACION (float)0.009133

// Implementación de la tarea de procesamiento de los datos del ADC:
static void vCorrienteProcess(void *pvParameters)
{
    // Información sobre la captura
    unsigned int adc_value = 0;
    unsigned long long time = 0;
    unsigned long timeSeconds = 0;
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
        // Tomar LLave para modificar el arreglo de tiempo:
        xSemaphoreTake(xValueCor, (TickType_t)FACTOR_ESPERA);
        // Capturar el instante en el que se empezo a realizar la captura:
        xQueueReceive(time1_RTOS, &timeSeconds, (TickType_t)0);
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc1_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_I[i] = ((double)(3.3 / 4095) * adc_value) + COMPENSACION;
                if (xQueueReceive(time1_queue, &time, (TickType_t)0))
                    (pxParameters->pxdata)->listT_I[i] = (timeSeconds + ((double)time / RESOLUTION));
                else
                    break;
            }
            else
                break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI y xTaskCorrCorI:
        while (eTaskGetState(xTaskCorrMaxI) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        while (eTaskGetState(xTaskCorrCorI) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        // Liberar llave para inicializar Captura:
        xSemaphoreGive(xMutexProcess1);      // Habilitar de nuevo la captura de datos.
        xSemaphoreGive(xWriteProcessMutex1); // Habilitar la lectura de datos.
        xSemaphoreGive(xValueCor);           // Habilitar lectura de arreglos de tiempo.
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura I Completa");
        // Activar tareas:
        vTaskResume(xTaskCorrMaxI);
        vTaskResume(xTaskCorrCorI);
        // Activar timer:
        ESP_ERROR_CHECK(gptimer_start(gptimer1));
        vTaskDelay(DELAY_TIME);
    }
    vTaskDelete(NULL);
}

// Creación de la tarea para el ADC 2:
static void vVoltajeProcess(void *pvParameters)
{
    // Información sobre la captura
    unsigned int adc_value = 0;
    unsigned long long time = 0;
    unsigned long timeSeconds = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvParameters;
    // Suspender tarea:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Tomar las LLaves para para leer / escribir en el arreglo:
        xSemaphoreTake(xMutexProcess2, portMAX_DELAY);
        xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);
        // Tomar LLave para modificar el arreglo de tiempo:
        xSemaphoreTake(xValueVolt, (TickType_t)FACTOR_ESPERA);
        // Capturar el instante en el que se empezo a realizar la captura:
        xQueueReceive(time2_RTOS, &timeSeconds, (TickType_t)0);
        // Copiar los datos a un arreglo para trasnferirlo a las tareas de Calculo.
        for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            if (xQueueReceive(adc2_queue, &adc_value, (TickType_t)0))
            {
                (pxParameters->pxdata)->listADC_V[i] = (double)(3.3 / 4095) * adc_value;
                if (xQueueReceive(time2_queue, &time, (TickType_t)0))
                    (pxParameters->pxdata)->listT_V[i] = (timeSeconds + ((double)time / RESOLUTION));
                else
                    break;
            }
            else
                break;
        }
        // Procesar el estado de la tarea: xTaskCorrMaxI
        while (eTaskGetState(xTaskVoltMaxV) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        while (eTaskGetState(xTaskVoltCorV) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        // Liberar llave para inicializar Captura:
        xSemaphoreGive(xMutexProcess2);      // Habilitar de nuevo la captura de datos.
        xSemaphoreGive(xWriteProcessMutex2); // Habilitar la lectura de datos.
        xSemaphoreGive(xValueVolt);          // Habilitar lectura de arreglos de tiempo.
        // Finalizar procesamiento de datos:
        ESP_LOGI(TAG, "Captura v Completa");
        // Activar Tareas:
        vTaskResume(xTaskVoltMaxV);
        vTaskResume(xTaskVoltCorV);
        // Activar timer:
        ESP_ERROR_CHECK(gptimer_start(gptimer2));
        vTaskDelay(DELAY_TIME);
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
        (pxADCParameters->pxdata)->listT_I[i] = (double)0;
        (pxADCParameters->pxdata)->listT_V[i] = (double)0;
    }
    // Inicializar variables:
    pxADCParameters->dVmax = 0;
    pxADCParameters->dImax = 0;
    for (short i = 0; i < 2; i++)
    {
        pxADCParameters->dcorteRefVt[i] = 0;
        pxADCParameters->dcorteRefIt[i] = 0;
    }
    pxADCParameters->dAngle = 0;
    pxADCParameters->usNumMI = 0;
    pxADCParameters->usNumMV = 0;

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