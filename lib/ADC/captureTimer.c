#include <ADC.h>

static const char *TAG = "Timers";

// Declaracione para el funcionamiento de las interupciones:
static volatile unsigned char opcV = 0x00; // Para el voltaje.
static volatile unsigned char opcI = 0x00; // Para el voltaje.

// Estados:
#define TOMAR_LLAVE 0x00
#define MUESTREO 0x01
#define PROCESAMIENTO 0x02
#define PAUSA 0x03

// Configuración:
#define TIMER_DIVIDER 80                              // Divisor para obtener 32 kHz (80 MHz / 80 = 1 MHz, 1 MHz / 31 = 32.25 kHz)
#define ALARMA ((unsigned int)(1 / (FRECUENCIA * 3))) // Valor de la alarma

// Controlador de los timer.
gptimer_handle_t gptimer1;
gptimer_handle_t gptimer2;

// configuración del timmer
static gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000, // 1MHz, 1 tick = 1us
};

// Estrctura de datos para la trasmición de datos en las interrupciones:
typedef struct Parameters_Timer
{
    // Variables Requeridas:
    int adc_value;
    uint64_t tiempo;
    short contador;
    short i;
    // Colas de trasmición de datos:
    QueueHandle_t *adc_queue;
    QueueHandle_t *time_queue;
    // Semaforo de captura:
    SemaphoreHandle_t *xMutexProcess;
} xTimersParameters;

// Estructura de trasmición de datos:
xTimersParameters *pxParamsI;
xTimersParameters *pxParamsV;

// Configurar alarma:
static gptimer_alarm_config_t alarm_config1 = {
    .alarm_count = ALARMA, // period = 1s
};

// ISR:
static bool IRAM_ATTR timer_callbackI(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    // Declarar variables:
    xTimersParameters *pvParameters;
    pvParameters = (xTimersParameters *)user_data;
    switch (opcI)
    {
    case TOMAR_LLAVE:
        // Reinciar variables
        pvParameters->contador = 0;
        pvParameters->i = 0;
        xSemaphoreTakeFromISR(*(pvParameters->xMutexProcess), &high_task_awoken);
        opcI = MUESTREO;
        break;
    case MUESTREO:
        // Guardar:
        if (pvParameters->contador < 2)
        {
            pvParameters->adc_value = adc1_get_raw(ADC_CHANNEL1);
            pvParameters->tiempo += ALARMA;
            pvParameters->contador++;
        }
        else
        {
            // Guardar en la el arreglo:
            pvParameters->adc_value = adc1_get_raw(ADC_CHANNEL1);
            pvParameters->adc_value = (int)pvParameters->adc_value / 3;
            // Guardar en la cola:
            xQueueSendToBackFromISR(*(pvParameters->adc_queue), &pvParameters->adc_value, &high_task_awoken);
            xQueueSendToBackFromISR(*(pvParameters->time_queue), &pvParameters->tiempo, &high_task_awoken);
            // Determinar si activar la tarea de procesamiento:
            if (pvParameters->i <= QUEUE_LENGTH)
            {
                pvParameters->contador = 0;
                pvParameters->i++;
            }
            else
                opcI = PROCESAMIENTO;
        }
        break;
    case PROCESAMIENTO:
        // Liberar el semáforo desde una ISR
        xSemaphoreGiveFromISR(*(pvParameters->xMutexProcess), &high_task_awoken);
        opcI = PAUSA;
        break;
    default:
        gptimer_stop(timer); // Detener el timer.
        break;
    }
    return (high_task_awoken == pdTRUE);
}
static gptimer_event_callbacks_t cbs1 = {
    .on_alarm = timer_callbackI,
};

// ISR
static bool IRAM_ATTR timer_callbackV(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    xTimersParameters *pvParameters;
    pvParameters = (xTimersParameters *)user_data;
    switch (opcI)
    {
    case TOMAR_LLAVE:
        // Reinciar variables
        pvParameters->contador = 0;
        pvParameters->i = 0;
        xSemaphoreTakeFromISR(*(pvParameters->xMutexProcess), &high_task_awoken);
        opcV = MUESTREO;
        break;
    case MUESTREO:
        // Variable para la captura del dato:
        int promedioV = 0;
        // Guardar:
        if (pvParameters->contador < 2)
        {
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            pvParameters->adc_value += promedioV;
            pvParameters->tiempo += ALARMA;
            pvParameters->contador++;
        }
        else
        {
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            pvParameters->adc_value += promedioV;
            pvParameters->adc_value = (int)pvParameters->adc_value / 3;
            // Guardar en la cola:
            xQueueSendToBackFromISR(*(pvParameters->adc_queue), &pvParameters->adc_value, &high_task_awoken);
            xQueueSendToBackFromISR(*(pvParameters->time_queue), &pvParameters->tiempo, &high_task_awoken);
            // Determinar si activar la tarea de procesamiento:
            if (pvParameters->i <= QUEUE_LENGTH)
            {
                pvParameters->contador = 0;
                pvParameters->i++;
            }
            else
                opcV = PROCESAMIENTO;
        }
        break;
    case PROCESAMIENTO:
        // Liberar el semáforo desde una ISR
        xSemaphoreGiveFromISR(*(pvParameters->xMutexProcess), &high_task_awoken);
        // Pasar a default:
        opcV = PAUSA;
        break;
    default:
        gptimer_stop(timer); // Detener el timer.
        break;
    }
    return (high_task_awoken == pdTRUE);
}
static gptimer_event_callbacks_t cbs2 = {
    .on_alarm = timer_callbackV,
};

void init_timers()
{
    // Inicializar el ADC para el core 0 (ADC1):
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_0);
    // Inicializar el ADC para el core 1 (ADC2):
    adc2_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_0);
    // Inicializar controladores de timers:
    gptimer2 = NULL;
    gptimer1 = NULL;

    // Inicializar parametros:
    pxParamsI = (xTimersParameters *)pvPortMalloc(sizeof(xTimersParameters));
    pxParamsI->adc_value = 0;
    pxParamsI->tiempo = 0;
    pxParamsI->contador = 0;
    pxParamsI->i = 0;
    pxParamsI->adc_queue = &adc1_queue;
    pxParamsI->time_queue = &time1_queue;
    pxParamsI->xMutexProcess = &xMutexProcess1;
    // Timer:
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer1));
    ESP_LOGI(TAG, "Enable timer 1");
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer1, &cbs1, pxParamsI));
    ESP_ERROR_CHECK(gptimer_enable(gptimer1));
    ESP_LOGI(TAG, "Start timer, stop it at alarm event 1");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer1, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer1));

    // Inicializar parametros:
    pxParamsV = (xTimersParameters *)pvPortMalloc(sizeof(xTimersParameters));
    pxParamsV->adc_value = 0;
    pxParamsV->tiempo = 0;
    pxParamsV->contador = 0;
    pxParamsV->i = 0;
    pxParamsV->adc_queue = &adc2_queue;
    pxParamsV->time_queue = &time2_queue;
    pxParamsV->xMutexProcess = &xMutexProcess2;
    // Timer:
    gptimer2 = NULL;
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer2));
    adc1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(pxParamsI->adc_value));
    time1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(pxParamsI->tiempo));
    ESP_LOGI(TAG, "Enable timer 2");
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer2, &cbs2, pxParamsV));
    ESP_ERROR_CHECK(gptimer_enable(gptimer2));
    ESP_LOGI(TAG, "Start timer, stop it at alarm event 2");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer2, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer2));
}