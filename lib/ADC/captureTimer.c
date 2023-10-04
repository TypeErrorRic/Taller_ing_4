#include <ADC.h>
#include <driver/timer.h>

// Declaracione para el funcionamiento de las interupciones:
static volatile unsigned char opcV; // Para el voltaje.
static volatile unsigned char opcI; // Para el voltaje.

// Estados:
#define TOMAR_LLAVE 0x00
#define MUESTREO 0x01
#define PROCESAMIENTO 0x02
#define PAUSA 0x03

// Configuración:
#define TIMER_DIVIDER 80                                                                      // Divisor para obtener 32 kHz (80 MHz / 80 = 1 MHz, 1 MHz / 31 = 32.25 kHz)
#define ALARMA ((unsigned int)((float)configCPU_CLOCK_HZ / (FRECUENCIA * 3)) * TIMER_DIVIDER) // Valor de la alarma

// Timer asociado al core 0:
static timer_config_t config1 = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_START,
    .auto_reload = true,
    .divider = TIMER_DIVIDER // Divisor para obtener una frecuencia de 1 MHz (80 MHz / 80 = 1 MHz)
};

// Declaraciones:
static volatile uint16_t promedioI = 0;
static volatile int adc_value1 = 0;
static volatile uint64_t tiempoI = 0;
static volatile short contadorI = 0;
static volatile short sizeI = 0;
// ISR:
void timer_callbackI(void *arg)
{
    switch (opcI)
    {
    case TOMAR_LLAVE:
        //Reinciar variables
        promedioI = 0;
        contadorI = 0;
        sizeI = 0;
        xSemaphoreTakeFromISR(xMutexProcess1, NULL);
        opcI = MUESTREO;
        break;
    case MUESTREO:
        //Guardar:
        if (contadorI < 2)
        {
            promedioI = adc1_get_raw(ADC_CHANNEL1);
            tiempoI += ALARMA;
            contadorI++;
        }
        else
        {
            promedioI = adc1_get_raw(ADC_CHANNEL1);
            adc_value1 = promedioI / 3;
            //Guardar en la cola:
            xQueueSendFromISR(adc1_queue, &adc_value1, NULL);
            xQueueSendFromISR(time1_queue, &tiempoI, NULL);
            //Determinar si activar la tarea de procesamiento:
            if (sizeI <= QUEUE_LENGTH)
            {
                contadorI = 0;
                sizeI++;
            }
            else
                opcI = PROCESAMIENTO;
        }
        break;
    case PROCESAMIENTO:
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // Liberar el semáforo desde una ISR
        xSemaphoreGiveFromISR(xMutexProcess1, &xHigherPriorityTaskWoken);
        // Si la operación en el semáforo despertó una tarea de mayor prioridad, solicita un cambio de contexto
        if (xHigherPriorityTaskWoken == pdTRUE)
            portYIELD_FROM_ISR();
        opcI = PAUSA;
        break;
    default:
        break;
    }
}

// Declaraciones:
static volatile uint16_t promedioV;
static volatile int adc_value2;
static volatile uint64_t tiempoV;
static volatile short contadorV;
static volatile short sizeV;
// ISR
void timer_callbackV(void *arg)
{
    switch (opcI)
    {
    case TOMAR_LLAVE:
        //Reinciar variables
        promedioV = 0;
        contadorV = 0;
        sizeV = 0;
        xSemaphoreTakeFromISR(xMutexProcess2, NULL);
        opcV = MUESTREO;
        break;
    case MUESTREO:
        //Guardar:
        if (contadorV < 2)
        {
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            adc_value2 += promedioV;
            tiempoV += ALARMA;
            contadorV++;
        }
        else
        {
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            adc_value2 += promedioV;
            adc_value2 = (int) adc_value2 / 3;
            //Guardar en la cola:
            xQueueSendFromISR(adc1_queue, &adc_value2, NULL);
            xQueueSendFromISR(time1_queue, &tiempoV, NULL);
            //Determinar si activar la tarea de procesamiento:
            if (sizeV <= QUEUE_LENGTH)
            {
                contadorV = 0;
                sizeV++;
            }
            else
                opcV = PROCESAMIENTO;
        }
        break;
    case PROCESAMIENTO:
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // Liberar el semáforo desde una ISR
        xSemaphoreGiveFromISR(xMutexProcess2, &xHigherPriorityTaskWoken);
        //Solicitar un cambio de contexto
        if (xHigherPriorityTaskWoken == pdTRUE)
            portYIELD_FROM_ISR();
        //Pasar a default:
        opcV = PAUSA;
        break;
    default:
        timer_disable_intr(TIMER_GROUP_0, TIMER_1);
        break;
    }
}

void init_timers()
{
    // core1:
    timer_init(TIMER_GROUP_0, TIMER_1, &config1);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, ALARMA);
    timer_enable_intr(TIMER_GROUP_0, TIMER_1);
    timer_isr_register(TIMER_GROUP_0, TIMER_1, timer_callbackI, (void *)0, 0, NULL);
    timer_start(TIMER_GROUP_0, TIMER_1);
    // Inicializar el ADC para el core 0 (ADC1):
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_0);

    // core2:
    timer_init(TIMER_GROUP_1, TIMER_1, &config1);
    timer_set_alarm_value(TIMER_GROUP_1, TIMER_1, ALARMA); 
    timer_enable_intr(TIMER_GROUP_1, TIMER_1);
    timer_isr_register(TIMER_GROUP_1, TIMER_1, timer_callbackV, (void *)0, 0, NULL);
    timer_start(TIMER_GROUP_1, TIMER_1);
    // Inicializar el ADC para el core 0 (ADC1):
    adc2_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_0);
}