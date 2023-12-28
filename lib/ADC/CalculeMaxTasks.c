/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @author Brayan Arley Mena Trejo. (brayan.mena@correounivalle.edu.co)
 * @brief Este archivo se encarga de Calcular el Valor Máximo de la corriente y el voltaje.
 * @version 0.1
 * @date 2023-11-06
 *
 * @copyright Copyright (c) 2023
 */

#include <ADC.h>
#include <math.h>

// Definición de las Tareas:
taskDefinition taskCorrMaxI;
taskDefinition taskVoltMaxV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrMaxI;
TaskHandle_t xTaskVoltMaxV;

// Compensación de voltajes con respecto al Osciloscopio:

static void vCorrMaxProcess(void *pvArguments)
{
    unsigned short maxdirection = 0;
    double maxValue[3];
    const unsigned int maxTime[3] = {0, 1, 2};
    double maxCor = 0;
    // Coeficicentes polinomio:
    double a = 0;
    double b = 0;
    double c = 0;
    double aux1 = 0;
    double aux2 = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Tomar Semaforo para detener el procesamiento de las tareas de Corriente:
        xSemaphoreTake(xReadCount1, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreTake(xWriteProcessMutex1, (TickType_t)FACTOR_ESPERA);

        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < (QUEUE_LENGTH - 1); i++)
        {
            // Capturar el dato Maximo:
            maxdirection = (pxParameters->pxdata)->listADC_I[maxdirection] < ((pxParameters->pxdata)->listADC_I[i]) ? i : maxdirection;
        }
        // Valor anterior al valor maximo:
        maxValue[0] = (pxParameters->pxdata)->listADC_I[maxdirection - 1];
        // Valor maximo:
        maxValue[1] = (pxParameters->pxdata)->listADC_I[maxdirection];
        // Valor después del valor maximo.
        maxValue[2] = (pxParameters->pxdata)->listADC_I[maxdirection + 1];

        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreGive(xWriteProcessMutex1);
        // Ceder llave:
        xSemaphoreGive(xReadCount1);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Relacionado con la interpolacion
        aux1 = (maxValue[1] - maxValue[0]) / (maxTime[1] - maxTime[0]);
        aux2 = (maxValue[2] - maxValue[1]) / (maxTime[2] - maxTime[1]);
        // Coeficientes del polinomio hallados con la interpolacion cuadratica
        a = (aux2 - aux1) / (maxTime[2] - maxTime[0]);
        b = aux1 - a * (maxTime[1] - maxTime[0]);
        c = maxValue[0] - aux1 * maxTime[0] + a * maxTime[1] * maxTime[0];
        // Valor maximo de corriente:
        maxCor = (c - ((b * b) / (4 * a)));

        // Esperar a que la tarea de Ángulo se detenga:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        //Guardar Datos:
        pxParameters->dImax = (isnan(maxCor) || isinf(maxCor) || (maxCor == 0)) ? maxValue[1] : maxCor;

        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltMaxProcess(void *pvArguments)
{
    unsigned short maxdirection = 0;
    double maxValue[3] = {};
    const unsigned int maxTime[3] = {1, 2, 3};
    double maxVolt = 0;
    // Coeficicentes polinomio:
    double a = 0;
    double b = 0;
    double c = 0;
    double aux1 = 0;
    double aux2 = 0;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Tomar Semaforo para detener el procesamiento de las tareas de Voltaje:
        xSemaphoreTake(xReadCount2, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);

        for (unsigned short i = 1; i < (QUEUE_LENGTH - 1); i++)
        {
            // Capturar el dato Maximo:
            maxdirection = (pxParameters->pxdata)->listADC_V[maxdirection] < ((pxParameters->pxdata)->listADC_V[i]) ? i : maxdirection;
        }
        // Valor anterior al valor maximo:
        maxValue[0] = (pxParameters->pxdata)->listADC_V[maxdirection - 1];
        // Valor maximo:
        maxValue[1] = (pxParameters->pxdata)->listADC_V[maxdirection];
        // Valor después del valor maximo.
        maxValue[2] = (pxParameters->pxdata)->listADC_V[maxdirection + 1];

        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreGive(xWriteProcessMutex2);
        // Ceder llave:
        xSemaphoreGive(xReadCount2);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Relacionado con la interpolacion
        aux1 = (maxValue[1] - maxValue[0]) / (maxTime[1] - maxTime[0]);
        aux2 = (maxValue[2] - maxValue[1]) / (maxTime[2] - maxTime[1]);
        // Coeficientes del polinomio hallados con la interpolacion cuadratica
        a = (aux2 - aux1) / (maxTime[2] - maxTime[0]);
        b = aux1 - a * (maxTime[1] - maxTime[0]);
        c = maxValue[0] - aux1 * maxTime[0] + a * maxTime[1] * maxTime[0];
        // Valor maximo de corriente:
        maxVolt = (c - ((b * b) / (4 * a)));

        // Esperar a que la tarea de Ángulo se detenga:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        //Guardar Datos:
        pxParameters->dVmax = (isnan(maxVolt) || isinf(maxVolt) || (maxVolt == 0)) ? maxValue[1] : maxVolt;

        // Suspender:
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeProcess()
{
    // Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskCorrMaxI.taskId = vCorrMaxProcess;
    taskCorrMaxI.name = "corrienteMax";
    taskCorrMaxI.pvParameters = pxADCParameters;
    taskCorrMaxI.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskCorrMaxI.uxPriority = 5; // Configurar la prioriodad.
    taskCorrMaxI.pvCreatedTask = &xTaskCorrMaxI;
    taskCorrMaxI.iCore = 0;

    // Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskVoltMaxV.taskId = vVoltMaxProcess;
    taskVoltMaxV.name = "voltMax";
    taskVoltMaxV.pvParameters = pxADCParameters;
    taskVoltMaxV.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskVoltMaxV.uxPriority = 5; // Configurar la prioriodad.
    taskVoltMaxV.pvCreatedTask = &xTaskVoltMaxV;
    taskVoltMaxV.iCore = 1;
}