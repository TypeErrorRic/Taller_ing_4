#include <ADC.h>
#include <math.h>

// Macros de control y de cálculo:
#define PI 3.141592
#define SIZE_VER 1

// Indices para inidicar la potencia:
#define ACTIVE 0
#define REACTIVE 1

static const char *TAG = "Angle Value";

// Definición de las Tareas:
taskDefinition taskAngle;

// Manejadores de las tareas:
TaskHandle_t xTaskAngle;

// Macros de control de cálculo del ángulo:
#define DESFASE_NUM 6
#define BALANCE 90 + DESFASE_NUM

static void calculateAngle(void *pvArguments)
{
    // Variables para calcular el ángulo:
    double angle = 0;
    double volt = 0;
    double cor = 0;
    double auxAngle = 0;
    // Contador de ángulos:
    double contador = 0;
    // Valores para controlar el flujo de acceso:
    double preValue = 0;
    double currentValue = 0;
    unsigned char flagCorrect = 0x00;
    // Calculo de la potencias:
    double power[2] = {};
    // Verificador de Valores:
    double angleVerific[SIZE_VER] = {};
    double maxCor[SIZE_VER] = {};
    double maxVolt[SIZE_VER] = {};
    short contadorVer = 0;
    // Parametros por default:
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *)pvArguments;
    // Suspender sistema
    vTaskSuspend(NULL);
    // Bucle principal
    for (;;)
    {
        // Calcular el ángulo:
        for (unsigned short i = 0; i < NUM_LN_ONDA; i++)
        {
            if ((pxParameters->dcorteRefIt[i] == 0) || (pxParameters->dcorteRefVt[i] == 0))
            {
                ESP_LOGW(TAG, "No se tomo la medida.");
            }
            else
            {
                auxAngle = (pxParameters->dcorteRefVt[i] - pxParameters->dcorteRefIt[i]) * 360 * FRECUENCIA_SENAL;
                if (auxAngle > 90 || auxAngle < -90)
                {
                    if (((auxAngle > BALANCE) || (auxAngle < -BALANCE)) && (contador == 0) && (pxParameters->usNumMI == pxParameters->usNumMV))
                    {
                        if (pxParameters->usNumMI == NUM_LN_ONDA)
                        {
                            ESP_LOGW(TAG, "Corregir angulo Mayor.");
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
                        else if ((pxParameters->usNumMI == pxParameters->usNumMV) && (auxAngle > -BALANCE))
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
        // Guardar valores para cuando contador es diferente a 0:
        if (contador != 0)
        {
            currentValue = (angle / contador);
            // Guardar el valor y activar la correción de ángulo.
            if (flagCorrect == 0x00)
            {
                angle = currentValue;
                preValue = currentValue;
            }
            else
            {
                // Verificar la correción de ángulo.
                if ((((preValue + DESFASE_NUM) > currentValue) && ((preValue - DESFASE_NUM) < currentValue)))
                {
                    angle = currentValue;
                    preValue = currentValue;
                }
                else
                {
                    // Salir con error si no son iguales:
                    ESP_LOGE(TAG, "No Coincide");
                    angle = 0;
                }
                flagCorrect = 0x00;
            }
        }
        // Salir con error:
        else
        {
            angle = 0;
            ESP_LOGE(TAG, "No Tomada");
        }
        // Reiniciar variables:
        contador = 0;
        auxAngle = 0;

        // Corrector de Valores:
        if (angle != 0)
        {
            angleVerific[contadorVer] = angle;
            maxCor[contadorVer] = pxParameters->dImax;
            maxVolt[contadorVer] = pxParameters->dVmax;
            contadorVer++;
        }

        // Cálculo de la potencia:
        if (contadorVer == SIZE_VER)
        {
            //Tiempo de espera para la realización de la tarea IDLE:
            vTaskDelay(FACTOR_ESPERA);
            // Aplicando media a los datos:
            for (unsigned short i = 0; i < SIZE_VER; i++)
            {
                angle += angleVerific[i];
                volt += maxVolt[i];
                cor += maxCor[i];
            }
            angle /= SIZE_VER;
            volt /= SIZE_VER;
            cor /= SIZE_VER;
            // Cálculo de la potencia Activa y reativa.
            power[ACTIVE] = volt * cor * FACTOR_ESCALA_VOLTAJE * FACTOR_ESCALA_CORRIENTE * cosh(angle * (double)PI / 180);
            power[REACTIVE] = volt * cor * FACTOR_ESCALA_VOLTAJE * FACTOR_ESCALA_CORRIENTE * sinh(angle * (double)PI / 180);
            // Trasmición de datos a la tarea de potencia:
            if (xQueueSend(powerData, &power, portMAX_DELAY) != pdPASS)
                ESP_LOGE(TAG, "Error de Trasmicion.");
            // Reiniciar Cuenta de Verificacion:
            contadorVer = 0;
            volt = 0;
            cor = 0;
        }

        // Finalizar tarea:
        ESP_LOGI(TAG, "Fin Anngle %.2f", angle);
        // Reiniciar ángulo:
        angle = 0;
        //  Suspender sistema
        vTaskSuspend(NULL);
    }
};

// Inicializando las tareas:
void setupTaskCalculeAngle()
{
    // Definir la Tarea para calcular el angulo de desfase:
    taskAngle.taskId = calculateAngle;
    taskAngle.name = "anguloDesfasePotencia";
    taskAngle.pvParameters = pxADCParameters;
    taskAngle.sizeTask = 3 * configMINIMAL_STACK_SIZE;
    taskAngle.uxPriority = 3;
    taskAngle.pvCreatedTask = &xTaskAngle;
    taskAngle.iCore = 0;
}