#include <ADC.h>

static const char *TAG = "Cor Value";
#define REF_VALUE (float)1.75

// Definición de las Tareas:
taskDefinition taskCorrCorI;
taskDefinition taskVoltCorV;

// Manejadores de las tareas:
TaskHandle_t xTaskCorrCorI;
TaskHandle_t xTaskVoltCorV;

// Mitad del periodo para el cáculo del punto de corte:
#define CORTE (((float)1 / FRECUENCIA) / 2)

static void vCorrCor(void *pvArguments)
{
    // Valores de captura de dato:
    double actualValue = 0;
    double preValue = 0;
    double actualTime = 0;
    double preTime = 0;
    double corCorValues[2] = {0, 0};
    unsigned short contador = 0;
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
        // Leer los datos del arreglo para obtener los valores maximos:
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            if ((pxParameters->pxdata)->listT_I[i - 1] >= ((pxParameters->pxdata)->listT_V[0] - CORTE))
            {
                actualValue = (pxParameters->pxdata)->listADC_I[i] - REF_VALUE;
                preValue = (pxParameters->pxdata)->listADC_I[i - 1] - REF_VALUE;
                // Aprovechar el cambio de vencidad entre muestras:
                if ((actualValue * preValue) < 0)
                {
                    if (contador < 2)
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
                // Si en las muestras no hay 2 puntos de corte no pasa nada, se toma con solo uno.
            }
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount1) == 1)
            xSemaphoreGive(xWriteProcessMutex1);
        // Ceder llave:
        xSemaphoreGive(xReadCount1);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Tomar llave de escritura de datos para Ángulo:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);

        // Reiniciar Varaibles de Cálculo y guardar datos:
        pxADCParameters->dcorteRefIt[0] = corCorValues[0];
        pxADCParameters->dcorteRefIt[1] = corCorValues[1];
        corCorValues[0] = 0;
        corCorValues[1] = 0;
        contador = 0;

        // Finalizar Tarea:
        ESP_LOGI(TAG, "Fin Cor I");
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
    double voltCorValues[2] = {0, 0};
    unsigned short contador = 0;
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
        for (unsigned short i = 1; i < QUEUE_LENGTH; i++)
        {
            // Capturar el dato de corte:
            if ((pxParameters->pxdata)->listT_V[i - 1] <= (pxParameters->pxdata)->listT_I[QUEUE_LENGTH - 1])
            {
                actualValue = (pxParameters->pxdata)->listADC_V[i] - REF_VALUE;
                preValue = (pxParameters->pxdata)->listADC_V[i - 1] - REF_VALUE;
                // Aprovechar el cambio de vencidad entre muestras:
                if ((actualValue * preValue) < 0)
                {
                    if (contador < 2)
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
                // Si en las muestras no hay 2 puntos de corte no pasa nada, se toma con solo uno.
            }
            else
                break;
        }
        // Decidir si cerder o activar la escritura de datos:
        if (uxSemaphoreGetCount(xReadCount2) == 1)
            xSemaphoreGive(xWriteProcessMutex2);
        // Ceder llave:
        xSemaphoreGive(xReadCount2);
        // Dar oprotunidad a la tarea de ángulo ejecutarse:
        vTaskDelay(FACTOR_ESPERA);

        // Tomar llave de escritura de datos para Ángulo:
        xSemaphoreTake(xWriteAngle, (TickType_t)FACTOR_ESPERA);
        // Tiempo para actualizar datos.
        vTaskDelay(1);
        // Esperar a que la tarea de Ángulo termine de procesar los datos:
        while (eTaskGetState(xTaskAngle) != eSuspended)
            vTaskDelay(FACTOR_ESPERA);

        // Reiniciar Varaibles de Cálculo y guardar datos:
        pxADCParameters->dcorteRefVt[0] = voltCorValues[0];
        pxADCParameters->dcorteRefVt[1] = voltCorValues[1];
        voltCorValues[0] = 0;
        voltCorValues[1] = 0;
        contador = 0;

        // Finalizar Tarea:
        ESP_LOGI(TAG, "Fin Cor V");
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