#include <ADC.h>
#include "driver/timer.h"

#define PERIODO ((int)(configTICK_RATE_HZ / FRECUENCIA) >= 1 ? (int)(configTICK_RATE_HZ / FRECUENCIA) : 1) // Periodo de Muestreo.

// Estructura para declaración de tareas:
taskDefinition taskADCCaptureI;
taskDefinition taskADCCaptureV;

// Declaración de las colas para la trasmición de mensajes:
QueueHandle_t adc1_queue;  // Cola con los valores del ADC de Corriente.
QueueHandle_t adc2_queue;  // Cola con los valores del ADC de Voltaje.
QueueHandle_t time1_queue; // Cola con los valores del instante de Captura de la Corriente.
QueueHandle_t time2_queue; // Cola con los volores del instante de Captura del Voltaje.

SemaphoreHandle_t xMutexProcess1; // Declaración del semáforo del core 0.
SemaphoreHandle_t xMutexProcess2; // Declaración del semáforo del core 1.

// Declaración de manejadores de las tarea:
TaskHandle_t xTaskCorrCaptureI;
TaskHandle_t xTaskVoltCaptureV;

// Tarea para captura de la señal de corriente:
static void vCaptureADCvalues0()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Inicializar el ADC para el core 0 (ADC1):
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_0);
    // Variabnles para la adquisicón de datos:
    uint32_t adc_value = 0;
    unsigned long time = 0;
    uint32_t promedio = 0;
    // Inicializar la cola para la trasmición de datos de los ADCs:
    adc1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(adc_value));
    time1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(time));
    // Supender tarea para sincronizar activación:
    vTaskSuspend(NULL);
    // Bucle principal:
    for (;;)
    {
        //  Capturar la llave para utilizar la cola:
        if (xSemaphoreTake(xMutexProcess1, portMAX_DELAY) == pdTRUE)
        {
            // Capturar los datos
            for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
            {
                // Detiene la tarea un intervalo de tiempo dado:
                vTaskDelayUntil(&xLastWakeTime, PERIODO);
                // Reiniciar variables:
                promedio = 0;
                // Captura los datos.
                for (short j = 0; j < PROMEDIO; j++)
                    promedio += adc1_get_raw(ADC_CHANNEL1);
                // Promedio de lecturas:
                adc_value = (uint32_t) promedio / PROMEDIO;
                xQueueSendToBack(adc1_queue, &adc_value, FACTOR_ESPERA);
                // Tiempo en que se realiza la lectura:
                time = (unsigned long)xLastWakeTime;
                xQueueSendToBack(time1_queue, &time, FACTOR_ESPERA);
            }
        }
        // Si la cola está llena ceder el procesamiento a otra tarea:
        xSemaphoreGive(xMutexProcess1); // Retornar la llave a la tarea de procesamiento:
        vTaskResume(xTaskCorrProcessI); // Volver a activar la tarea de procesamiento.
        vTaskDelay(FACTOR_ESPERA);      // Suspender la tarea durante 10 ticks para ceder al procesador.
    }
    vTaskDelete(NULL);
}

// Tarea para captura de la señal de Voltaje:
static void vCaptureADCvalues1()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Inicializar el ADC para el core 0 (ADC1):
    adc2_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_0);
    // Variables para la adquisicón de datos:
    uint32_t adc_value = 0;
    int promedio = 0;
    unsigned long time = 0;
    // Inicializar la cola para la trasmición de datos de los ADCs:
    adc2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(adc_value));
    time2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(time));
    // Supender tarea para sincronizar activación:
    vTaskSuspend(NULL);
    // Bucle Principal:
    for (;;)
    {
        // Capturar la llave para utilizar la cola:
        if (xSemaphoreTake(xMutexProcess2, portMAX_DELAY) == pdTRUE)
        {
            // Capturar los datos:
            for (unsigned short i = 0; i < QUEUE_LENGTH; i++)
            {
                // Detiene la tarea un intervalo de tiempo dado:
                vTaskDelayUntil(&xLastWakeTime, PERIODO);
                // Reiniciar variables:
                adc_value = 0;
                //Conversión de valores:
                for (short j = 0; j < PROMEDIO; j++)
                {
                    adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedio);
                    adc_value += promedio;
                }
                // Promedio de lecturas:
                adc_value = (uint32_t) adc_value / PROMEDIO;
                xQueueSendToBack(adc2_queue, &adc_value, FACTOR_ESPERA);
                // Tiempo de captura del dato:
                time = (unsigned long)xLastWakeTime;
                xQueueSendToBack(time2_queue, &time, FACTOR_ESPERA);
            }
        }
        // Si la cola está llena ceder el procesamiento a otra tarea:
        xSemaphoreGive(xMutexProcess2); // Retornar la llave a la tarea de procesamiento.
        vTaskResume(xTaskVoltProcessV); // Volver a activar la tarea de procesamiento.
        vTaskDelay(FACTOR_ESPERA);      // Suspender la tarea durante 10 ticks para que la tome el procesador.
    }
    vTaskDelete(NULL);
}

// Inicializar Tareas de captura:
void initTaskCapture()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente en core 0:
    taskADCCaptureI.taskId = vCaptureADCvalues0;
    taskADCCaptureI.name = "captureADCvalues0";
    taskADCCaptureI.pvParameters = NULL;
    taskADCCaptureI.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCCaptureI.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskADCCaptureI.pvCreatedTask = &xTaskCorrCaptureI;
    taskADCCaptureI.iCore = 0;

    // Definir la Tarea para el Procesamiento de la Señal de Corriente en core 1:
    taskADCCaptureV.taskId = vCaptureADCvalues1;
    taskADCCaptureV.name = "captureADCvalues1";
    taskADCCaptureV.pvParameters = NULL;
    taskADCCaptureV.uxPriority = configMAX_PRIORITIES; // Configurar la Maxima prioriodad.
    taskADCCaptureV.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskADCCaptureV.pvCreatedTask = &xTaskVoltCaptureV;
    taskADCCaptureV.iCore = 1;

    // Inicializar los mutex para manejar la transferencia de datos:
    xMutexProcess1 = xSemaphoreCreateMutex();
    xMutexProcess2 = xSemaphoreCreateMutex();
}

// Función con las llamadas requeridas para realizar la configuración de los ADCs.
void initElementsADCs()
{
    initTaskCapture();
    setupTaskProcessADCs();
    setupTaskCalculeProcess();
    setupTaskCalculeCorProcess();
}