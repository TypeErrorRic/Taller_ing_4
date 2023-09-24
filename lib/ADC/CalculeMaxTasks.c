#include <ADC.h>

taskDefinition taskCorrMaxI;
taskDefinition taskVoltMaxV;

//Manejadores de las tareas:
TaskHandle_t xTaskCorrMaxI;
TaskHandle_t xTaskVoltMaxV;

//Regulador de acceso:
SemaphoreHandle_t xSemaphore; //Pendiente.

//Cambio de tarea para lograr sincronización de respuesta con ángulo y valor Maximo.
#define OPORTUNIDAD 20

static void corrMaxProcess(void *pvArguments)
{
    unsigned short contador = 0;
    uint32_t max_value = 0;
    //Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    //Bucle principal
    for(;;)
    {
        for(unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            //Capturar el dato Maximo:
            contador = max_value < ((pxParameters->pxdata)->listADC_I[i]) ? i : contador;
            max_value = (pxParameters->pxdata)->listADC_I[contador];
            //Dar oprotunidad a la tarea de ángulo ejecutarse:
            if(i%OPORTUNIDAD == 0) taskYIELD();
        }
        //Suspender.
        vTaskSuspend(NULL);
    }
};

static void voltMaxProcess(void *pvArguments)
{
    unsigned short contador = 0;
    uint32_t max_value = 0;
    //Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    //Bucle principal
    for(;;)
    {
        for(unsigned short i = 0; i < QUEUE_LENGTH; i++)
        {
            //Capturar el dato Maximo:
            contador = max_value < ((pxParameters->pxdata)->listADC_V[i]) ? i : contador;
            max_value = (pxParameters->pxdata)->listADC_V[contador];
            //Dar oprotunidad a la tarea de ángulo ejecutarse:
            if(i%OPORTUNIDAD == 0) taskYIELD();
        }
        //Suspender:
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeProcess()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskCorrMaxI.taskId = corrMaxProcess;
    taskCorrMaxI.name = "corrienteProcess";
    taskCorrMaxI.usStackDepth = SIZE_TASK_ADC;
    taskCorrMaxI.usStackDepth = SIZE_TASK_ADC;
    taskCorrMaxI.pvParameters = pxADCParameters;
    taskCorrMaxI.uxPriority = 5; // Configurar la prioriodad.
    taskCorrMaxI.pvCreatedTask = xTaskCorrMaxI;
    taskCorrMaxI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskVoltMaxV.taskId = voltMaxProcess;
    taskVoltMaxV.name = "voltProcess";
    taskVoltMaxV.usStackDepth = SIZE_TASK_ADC;
    taskVoltMaxV.pvParameters = 5;
    taskVoltMaxV.uxPriority = configMAX_PRIORITIES; // Configurar la prioriodad.
    taskVoltMaxV.pvCreatedTask = xTaskVoltMaxV;
    taskVoltMaxV.iCore = 1;

    //Inicalizar un semaforo para el acceso al recurso:
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateCounting(3, 0);
}