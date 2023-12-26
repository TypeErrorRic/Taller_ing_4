/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @author Brayan Arley Mena Trejo. (brayan.mena@correounivalle.edu.co)
 * @brief Este archivo se encarga de Calcular el punto de corte con el eje t.
 * @version 0.1
 * @date 2023-11-06
 *
 * @copyright Copyright (c) 2023
 */

#include <ADC.h>

// Definición de las Tareas:
taskDefinition taskCorrCorI;
taskDefinition taskVoltCorV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrCorI;
TaskHandle_t xTaskVoltCorV;

// Mitad del periodo para el cáculo del punto de corte:
#define CORTE (((float)1 / FRECUENCIA) / 2)

//Ganancia para mejorar el funcionamiento del Esp32:
#define GANANCIA 100000

static void vCorrCor(void *pvArguments)
{
    // Valores de captura de dato:
    double actualValue = 0;
    double preValue = 0;
    double actualTime = 0;
    double preTime = 0;
    double corCorValues[NUM_LN_ONDA];
    for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        corCorValues[NUM_LN_ONDA] = 0;
    unsigned short contador = 0;
    unsigned short muestraTomadas;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Tomar Semaforo:
        xSemaphoreTake(xReadCount1, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreTake(xWriteProcessMutex1, (TickType_t)FACTOR_ESPERA);

        // Esperar a que se hayan procesado los arreglos en ambos nucleos:
        xSemaphoreTake(xValueVolt, (TickType_t)portMAX_DELAY);
        vTaskDelay(FACTOR_ESPERA * 2);
        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            if ((pxParameters->pxdata)->listT_I[i - 1] >= ((pxParameters->pxdata)->listT_V[0] - CORTE))
            {
                actualValue = GANANCIA * ((pxParameters->pxdata)->listADC_I[i] - REF_VALUE_CORRIENTE);
                preValue = GANANCIA * ((pxParameters->pxdata)->listADC_I[i - 1] - REF_VALUE_CORRIENTE);
                // Aprovechar el cambio de vencidad entre muestras:
                if ((actualValue * preValue) < 0)
                {
                    if (contador < NUM_LN_ONDA)
                    {
                        // Valores para realizar el cálculo:
                        actualTime = (pxParameters->pxdata)->listT_I[i];
                        preTime = (pxParameters->pxdata)->listT_I[i - 1];
                        // Calcular Punto de corte:
                        corCorValues[contador] = actualTime - (actualValue * ((actualTime - preTime) / (actualValue - preValue)));
                        // Incremento del contador:
                        contador++;
                    }
                    else
                        break;
                }
                muestraTomadas++;
            }
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreGive(xWriteProcessMutex1);
        // Ceder llave:
        xSemaphoreGive(xReadCount1);

        // Activar la Modificación de los arreglos en Volt:
        xSemaphoreGive(xValueVolt);

        // Tomar llave de escritura de datos para Ángulo:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);

        // Reiniciar Varaibles de Cálculo y guardar datos:
        for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        {
            pxADCParameters->dcorteRefIt[i] = corCorValues[i];
            corCorValues[i] = 0;
        }
        pxParameters->usNumMI = contador;
        contador = 0;
        muestraTomadas = 0;
          
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xWriteAngle) == 1)
        {
            // Ceder llave:
            xSemaphoreGive(xWriteAngle);
            // Activar tarea de ángulo:
            vTaskResume(xTaskAngle);
        }
        else
            xSemaphoreGive(xWriteAngle);
        // Suspender.
        vTaskSuspend(NULL);
    }
};

static void vVoltCor(void *pvArguments)
{
    // Valores de captura de dato:
    double actualValue = 0;
    double preValue = 0;
    double actualTime = 0;
    double preTime = 0;
    double voltCorValues[NUM_LN_ONDA];
    for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        voltCorValues[NUM_LN_ONDA] = 0;
    unsigned short contador = 0;
    unsigned short muestraTomadas;
    // Inicializar Parametros:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Tomar Semaforo:
        xSemaphoreTake(xReadCount2, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Sección critica de lectura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreTake(xWriteProcessMutex2, (TickType_t)FACTOR_ESPERA);

        // Esperar a que se hayan procesado los arreglos en ambos nucleos:
        xSemaphoreTake(xValueCor, (TickType_t)portMAX_DELAY);
        vTaskDelay(FACTOR_ESPERA * 2);
        // Bloquear tareas de procesamiento en ambos nucleos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            if ((pxParameters->pxdata)->listT_V[i - 1] <= (pxParameters->pxdata)->listT_I[QUEUE_LENGTH - 1])
            {
                actualValue = GANANCIA * ((pxParameters->pxdata)->listADC_V[i] - REF_VALUE_VOLTAJE);
                preValue = GANANCIA * ((pxParameters->pxdata)->listADC_V[i - 1] - REF_VALUE_VOLTAJE);
                // Aprovechar el cambio de vencidad entre muestras:
                if ((actualValue * preValue) < 0)
                {
                    if (contador < NUM_LN_ONDA)
                    {
                        // Valores para realizar el cálculo:
                        actualTime = (pxParameters->pxdata)->listT_V[i];
                        preTime = (pxParameters->pxdata)->listT_V[i - 1];
                        // Calcular Punto de corte:
                        voltCorValues[contador] = actualTime - (actualValue * ((actualTime - preTime) / (actualValue - preValue)));
                        // Incremento del contador:
                        contador++;
                    }
                    else
                        break;
                }
                muestraTomadas++;
            }
            else
                break;
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreGive(xWriteProcessMutex2);
        // Ceder llave:
        xSemaphoreGive(xReadCount2);

        // Decidir si activar la Modificación de los arreglos de Cor:
        xSemaphoreGive(xValueCor);

        // Tomar llave de escritura de datos para Ángulo:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);
        
        // Reiniciar Variables de Cálculo y guardar datos:
        for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        {
            pxADCParameters->dcorteRefVt[i] = voltCorValues[i];
            voltCorValues[i] = 0;
        }
        pxParameters->usNumMV = contador;
        contador = 0;
        muestraTomadas = 0;

        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xWriteAngle) == 1)
        {
            // Ceder llave:
            xSemaphoreGive(xWriteAngle);
            // Activar tarea de ángulo:
            vTaskResume(xTaskAngle);
        }
        else
            xSemaphoreGive(xWriteAngle);
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