#include <ADC.h>

static const char *TAG = "Angle Value";

// Definición de las Tareas:
taskDefinition taskAngle;

// Manejadores de las tareas:
TaskHandle_t xTaskAngle;

// Macros de control de cálculo del ángulo:
#define DESFASE_NUM 3
#define BALANCE 90 + DESFASE_NUM

static void calculateAngle(void *pvArguments)
{
    // Variables para calcular el ángulo:
    double angle = 0;
    double auxAngle = 0;
    double contador = 0;
    // Valores para controlar el flujo de acceso:
    double preValue = 0;
    double currentValue = 0;
    unsigned char flagCorrect = 0x00;
    unsigned char ready = 0x00;
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        if (flagCorrect == 0x01)
        {
            flagCorrect = 0x00;
            //  Suspender sistema
            vTaskSuspend(NULL);
        }
        // Calcular el angulo:
        for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        {
            if ((pxParameters->dcorteRefIt[i] == 0) || (pxParameters->dcorteRefVt[i] == 0))
            {
                ESP_LOGW(TAG, "No se tomo la medida.");
            }
            else
            {
                auxAngle = (pxParameters->dcorteRefVt[i] - pxParameters->dcorteRefIt[i]) * 360 * FRECUENCIA_SENAL;
                if (auxAngle > 90)
                {
                    if ((auxAngle > BALANCE) && (contador == 0) && (pxParameters->usNumMI == pxParameters->usNumMV))
                    {
                        ESP_LOGW(TAG, "Corregir angulo Mayor.");
                        if ((ready == 0x01) && (pxParameters->usNumMI == NUM_LN_ONDA))
                        {
                            flagCorrect = 0x01;
                            // Se corre para que de negativo: desfase cercano a - 180 grados.
                            for (unsigned short j = 0; j < (NUM_LN_ONDA - 1); j++)
                            {
                                angle += (pxParameters->dcorteRefVt[i] - pxParameters->dcorteRefIt[i + 1]) * 360 * FRECUENCIA_SENAL;
                                contador++;
                            }
                            break;
                        }
                        else
                            break;
                    }
                    else
                    {
                        // Verificar que hayan la misma cantidad de cortes y que el ángulo sea menor a BALANCE.
                        if ((pxParameters->usNumMI == pxParameters->usNumMV) && (auxAngle < BALANCE))
                            angle += 90;
                        else
                            break;
                    }
                }
                else if (auxAngle < -90)
                {
                    if ((auxAngle < -BALANCE) && (contador == 0) && (pxParameters->usNumMI == pxParameters->usNumMV))
                    {
                        ESP_LOGW(TAG, "Corregir angulo Menor");
                        if ((ready == 0x01) && (pxParameters->usNumMI == NUM_LN_ONDA))
                        {
                            flagCorrect = 0x01;
                            // Se corre para que de Positivo: desfase cercano a + 180 grados.
                            for (unsigned short j = 0; j < (NUM_LN_ONDA - 1); j++)
                            {
                                angle += (pxParameters->dcorteRefVt[i + 1] - pxParameters->dcorteRefIt[i]) * 360 * FRECUENCIA_SENAL;
                                contador++;
                            }
                            break;
                        }
                        else
                            break;
                    }
                    else
                    {
                        // Verificar que hayan la misma cantidad de cortes y que el ángulo sea menor a BALANCE.
                        if ((pxParameters->usNumMI == pxParameters->usNumMV) && (auxAngle > -BALANCE))
                            angle -= 90;
                        else
                            break;
                    }
                }
                else
                {
                    angle += auxAngle;
                }
                contador++;
            }
        }
        // Guardar el Valor:
        xSemaphoreTake(xPower3, (TickType_t)portMAX_DELAY);
        // Guardar valores para cuando contador es diferente a 0:
        if (contador != 0)
        {
            currentValue = (angle / contador);
            // Guardar el valor y activar la correción de ángulo.
            if (flagCorrect == 0x00)
            {
                pxParameters->dAngle = currentValue;
                preValue = currentValue;
                ready = 0x01;
            }
            else
            {
                // Verificar la correción de ángulo.
                if ((((preValue + DESFASE_NUM) > currentValue) && ((preValue - DESFASE_NUM) < currentValue)))
                {
                    pxParameters->dAngle = currentValue;
                    preValue = currentValue;
                }
                else
                {
                    // Salir con error si no son iguales:
                    if (ready == 0x01)
                        ESP_LOGE(TAG, "Medida no valida. IDK");
                    pxParameters->dAngle = 0;
                }
            }
        }
        // Salir con error:
        else
        {
            pxParameters->dAngle = 0;
            if (ready == 0x01)
                ESP_LOGE(TAG, "Medida no valida. NoT");
        }
        // Reiniciar variables:
        contador = 0;
        angle = 0;
        currentValue = 0;
        auxAngle = 0;
        // Entregar llave:
        xSemaphoreGive(xPower3);

        // Prueba
        printf(">A:%f\n", pxParameters->dAngle);
        ESP_LOGI(TAG, "Fin Angle");
        //  Suspender sistema
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeAngle()
{
    // Definir la Tarea para calcular el angulo de desfase:
    taskAngle.taskId = calculateAngle;
    taskAngle.name = "anguloDesfase";
    taskAngle.pvParameters = pxADCParameters;
    taskAngle.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskAngle.uxPriority = 10;
    taskAngle.pvCreatedTask = &xTaskAngle;
    taskAngle.iCore = 0;
}