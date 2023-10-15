/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este Archivo se encarga de Inicializar y Ejecutar el Muestreo de las Señales.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

#include <ADC.h>

static const char *TAG = "Timers";

// Declaraciones para el funcionamiento de las interupciones:
static volatile unsigned char opcV = 0x00; // Para el voltaje.
static volatile unsigned char opcI = 0x00; // Para el voltaje.

// Estados:
#define TOMAR_LLAVE 0x00
#define MUESTREO 0x01
#define CEDER 0x02

// Configuración:
#define RESOLUTION 1000000 // 1MHz, 1 tick = 1us
#define ALARMA ((unsigned int)((((double)1) / (FRECUENCIA * 3)) * 1000000))

// Controlador de los timer.
gptimer_handle_t gptimer1;
gptimer_handle_t gptimer2;
// TIMER WALL CLOCK:
static gptimer_handle_t gptimer3;

// configuración del timmer
static gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = RESOLUTION,
};

// Estructura de datos para la trasmición de datos en las interrupciones:
typedef struct Parameters_Timer
{
    // Variables Requeridas:
    unsigned int adc_value;
    unsigned long long tiempo;
    unsigned long tiempoSeconds;
    unsigned short contador;
    unsigned short i;
    // Colas de trasmición de datos:
    QueueHandle_t *adc_queue;
    QueueHandle_t *time_queue;
    QueueHandle_t *time_RTOS;
    // Semaforo de captura:
    SemaphoreHandle_t *xMutexProcess;
    // Tiempo transcurrido desde el segundo anterior:
    unsigned long long usTime;
    // Control del Wall Clock:
    gptimer_handle_t *clock;
} xTimersParameters;

// Estructura de trasmición de datos:
static xTimersParameters *pxParamsI;
static xTimersParameters *pxParamsV;

// Configurar alarma:
static gptimer_alarm_config_t alarm_config1 = {
    .reload_count = 0,
    .alarm_count = ALARMA,
    .flags.auto_reload_on_alarm = true,
};

// Configurar led de muestreo:
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .intr_type = GPIO_INTR_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
};

// Variable para almacenar el estado del LED (0: apagado, 1: encendido)
static volatile int led_state = 0;

// Variable para almacenar la cantidad de Segundos Transcurridos:
static volatile unsigned long segundos = 0;

// Configurar alarma:
static gptimer_alarm_config_t alarm_configClock = {
    .reload_count = 0,
    .alarm_count = 1000000, // Cada Segundo:
    .flags.auto_reload_on_alarm = true,
};

// Interrupción para el contador de segundos:
static bool IRAM_ATTR wallClock(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    segundos++;
    return false;
}
static gptimer_event_callbacks_t cbs3 = {
    .on_alarm = wallClock,
};

// ISR:
static bool IRAM_ATTR timer_callbackI(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    // Declarar variables:
    xTimersParameters *pvParameters;
    pvParameters = (xTimersParameters *)user_data;
    // Procesamiento:
    switch (opcI)
    {
    case TOMAR_LLAVE:
        // Reinciar variables
        pvParameters->contador = 0;
        pvParameters->i = 0;
        pvParameters->adc_value = 0;
        pvParameters->usTime = 0;
        // Capturar el instante en el que se empezara a capturar datos:
        if (xSemaphoreTakeFromISR(*(pvParameters->xMutexProcess), &high_task_awoken) == pdTRUE)
        {
            // Capturar instante del tiempo:
            gptimer_get_raw_count(*(pvParameters->clock), &pvParameters->usTime);
            pvParameters->tiempo = pvParameters->usTime;
            pvParameters->tiempoSeconds = segundos;
            // Guardar instante en que el sistema entra a la interrupción:
            xQueueSendToBackFromISR(*(pvParameters->time_RTOS), &pvParameters->tiempoSeconds, &high_task_awoken);
            opcI = MUESTREO;
        }
        break;
    case MUESTREO:
        // Guardar:
        if (pvParameters->contador < 2)
        {
            pvParameters->adc_value += adc1_get_raw(ADC_CHANNEL1);
            pvParameters->tiempo += edata->alarm_value;
            pvParameters->contador++;
        }
        else
        {
            // Guardar en la el arreglo:
            pvParameters->adc_value += adc1_get_raw(ADC_CHANNEL1);
            pvParameters->adc_value = (unsigned int)pvParameters->adc_value / 3;
            // Guardar en la cola:
            xQueueSendToBackFromISR(*(pvParameters->adc_queue), &pvParameters->adc_value, &high_task_awoken);
            xQueueSendToBackFromISR(*(pvParameters->time_queue), &pvParameters->tiempo, &high_task_awoken);
            // Determinar si activar la tarea de procesamiento:
            if (pvParameters->i <= QUEUE_LENGTH)
            {
                // Incrementar tiempo:
                pvParameters->tiempo += edata->alarm_value;
                //Reincicar variables:
                pvParameters->contador = 0;
                pvParameters->adc_value = 0;
                pvParameters->i++;
            }
            else
                opcI = CEDER; // Default:
        }
        break;
    default:
        // Liberar el semáforo desde una ISR
        if (xSemaphoreGiveFromISR(*(pvParameters->xMutexProcess), &high_task_awoken) == pdTRUE)
        {
            opcI = TOMAR_LLAVE;
            gptimer_stop(timer); // Detener el timer.
        }
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
    // Declarar variables:
    xTimersParameters *pvParameters;
    pvParameters = (xTimersParameters *)user_data;
    // Procesamiento:
    switch (opcV)
    {
    case TOMAR_LLAVE:
        // Encender led:
        led_state = !led_state;
        gpio_set_level(LED_PIN, led_state);
        // Reinciar variables
        pvParameters->contador = 0;
        pvParameters->i = 0;
        pvParameters->adc_value = 0;
        pvParameters->usTime = 0;
        if (xSemaphoreTakeFromISR(*(pvParameters->xMutexProcess), &high_task_awoken) == pdTRUE)
        {
            // Capturar instante del tiempo:
            gptimer_get_raw_count(*(pvParameters->clock), &pvParameters->usTime);
            pvParameters->tiempo = pvParameters->usTime;
            pvParameters->tiempoSeconds = segundos;
            // Guardar instante en que el sistema entra a la interrupción:
            xQueueSendToBackFromISR(*(pvParameters->time_RTOS), &pvParameters->tiempoSeconds, &high_task_awoken);
            opcV = MUESTREO;
        }
        break;
    case MUESTREO:
        // Guardar:
        int promedioV = 0;
        if (pvParameters->contador < 2)
        {
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            pvParameters->adc_value += promedioV;
            pvParameters->tiempo += edata->alarm_value;
            pvParameters->contador++;
        }
        else
        {
            // Guardar en la el arreglo:
            adc2_get_raw(ADC_CHANNEL2, ADC_WIDTH_BIT_12, &promedioV);
            pvParameters->adc_value += promedioV;
            pvParameters->adc_value = (unsigned int)pvParameters->adc_value / 3;
            // Guardar en la cola:
            xQueueSendToBackFromISR(*(pvParameters->adc_queue), &pvParameters->adc_value, &high_task_awoken);
            xQueueSendToBackFromISR(*(pvParameters->time_queue), &pvParameters->tiempo, &high_task_awoken);
            // Determinar si activar la tarea de procesamiento:
            if (pvParameters->i <= QUEUE_LENGTH)
            {
                // Incrementar tiempo:
                pvParameters->tiempo += edata->alarm_value;
                //Reincicar variables:
                pvParameters->contador = 0;
                pvParameters->adc_value = 0;
                pvParameters->i++;
            }
            else
                opcV = CEDER; // Default:
        }
        break;
    default:
        // Liberar el semáforo desde una ISR
        if (xSemaphoreGiveFromISR(*(pvParameters->xMutexProcess), &high_task_awoken) == pdTRUE)
        {
            opcV = TOMAR_LLAVE;
            gptimer_stop(timer); // Detener el timer.
        }
        break;
    }
    return (high_task_awoken == pdTRUE);
}
static gptimer_event_callbacks_t cbs2 = {
    .on_alarm = timer_callbackV,
};

void init_timers()
{
    ESP_LOGW(TAG, "Periodo de Muestreo: %i us", ALARMA);
    ESP_LOGW(TAG, "Longitud de Muestra: %i", QUEUE_LENGTH);
    // Inicializar el ADC para el core 0 (ADC1):
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_11);
    // Inicializar el ADC para el core 1 (ADC2):
    adc2_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_11);
    // Inicializar controladores de timers:
    gptimer2 = NULL;
    gptimer1 = NULL;
    // Inicializar led de prueba:
    gpio_config(&io_conf);

    // Inicializar parametros:
    pxParamsI = (xTimersParameters *)pvPortMalloc(sizeof(xTimersParameters));
    pxParamsI->adc_value = 0;
    pxParamsI->tiempo = 0;
    pxParamsI->contador = 0;
    pxParamsI->i = 0;
    pxParamsI->adc_queue = &adc1_queue;
    pxParamsI->time_queue = &time1_queue;
    pxParamsI->xMutexProcess = &xMutexProcess1;
    pxParamsI->time_RTOS = &time1_RTOS;
    pxParamsI->tiempoSeconds = 0;
    pxParamsI->usTime = 0;
    pxParamsI->clock = &gptimer3;
    // Timer:
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer1));
    ESP_LOGI(TAG, "Enable timer 1");
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer1, &cbs1, pxParamsI));
    ESP_ERROR_CHECK(gptimer_enable(gptimer1));
    ESP_LOGI(TAG, "Start timer, stop it at alarm event 1");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer1, &alarm_config1));

    // Inicializar parametros:
    pxParamsV = (xTimersParameters *)pvPortMalloc(sizeof(xTimersParameters));
    pxParamsV->adc_value = 0;
    pxParamsV->tiempo = 0;
    pxParamsV->contador = 0;
    pxParamsV->i = 0;
    pxParamsV->adc_queue = &adc2_queue;
    pxParamsV->time_queue = &time2_queue;
    pxParamsV->xMutexProcess = &xMutexProcess2;
    pxParamsV->time_RTOS = &time2_RTOS;
    pxParamsV->tiempoSeconds = 0;
    pxParamsV->usTime = 0;
    pxParamsV->clock = &gptimer3;
    // Timer:
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer2));
    ESP_LOGI(TAG, "Enable timer 2");
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer2, &cbs2, pxParamsV));
    ESP_ERROR_CHECK(gptimer_enable(gptimer2));
    ESP_LOGI(TAG, "Start timer 2, stop it at alarm event");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer2, &alarm_config1));

    // TIMER WALL CLOCK:
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer3));
    ESP_LOGI(TAG, "Enable timer Clock");
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer3, &cbs3, ((void *)0)));
    ESP_ERROR_CHECK(gptimer_enable(gptimer3));
    ESP_LOGI(TAG, "Start timer Clock, stop it at alarm event");
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer3, &alarm_configClock));
    // Iniciar el reloj:
    ESP_ERROR_CHECK(gptimer_start(gptimer3));
}