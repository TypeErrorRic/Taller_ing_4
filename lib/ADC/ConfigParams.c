#include <ADC.h>
#include <stdio.h>

//Estructura para declaración de tareas:
taskDefinition taskADC1; 
taskDefinition taskADC2;

//Declaración de las colas para la trasmición de mensajes:
QueueHandle_t adc1_queue;   // Cola con los valores del ADC de Corriente.
QueueHandle_t adc2_queue;   // Cola con los valores del ADC de Voltaje.
QueueHandle_t time1_queue;  // Cola con los valores del instante de Captura de la Corriente.
QueueHandle_t time2_queue;  // Cola con los volores del instante de Captura del Voltaje.

//Configuración del ADC:
static void initADCs()
{
    // Inicializar el ADC para el core 0 (ADC1)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_0);

    // Inicializar el ADC para el core 1 (ADC2)
    adc2_config_width(ADC_WIDTH_BIT_12);
    adc2_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_0);

    //Inicializar la cola para la trasmición de datos de los ADCs:
    adc1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
    adc2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
    time1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
    time2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
}

//Configuración de timers:
static timer_config_t timer_config = {
    .alarm_en = TIMER_ALARM_DIS, // Deshabilitar la alarma
    .counter_en = TIMER_START,   // Iniciar el contador
    .intr_type = TIMER_INTR_LEVEL, // Tipo de interrupción
    .counter_dir = TIMER_COUNT_UP, // Dirección del contador
    .auto_reload = TIMER_AUTORELOAD_DIS, // Modo de recarga automática
    .divider = 80 // Divisor para configurar la frecuencia del temporizador (80 para 1 MHz)
};

//Interrupción del timer asociado al core 0:
void IRAM_ATTR timer_isr(void* arg) {
    //Inicializar argumentos:
    xTIMERParameters *pxParameters;
    pxParameters = (xTIMERParameters *) arg;
    //Aumentar el tiempo:
    pxParameters->ulTimeCaptureI += PERIODO;
    // La conversión ha finalizado, puedes leer el valor del ADC aquí
    uint32_t adc_value = adc1_get_raw(ADC_CHANNEL1);
    // Enviar los valores a las colas de mensajes
    xQueueSendFromISR(adc1_queue, &adc_value, NULL);
    xQueueSendFromISR(time1_queue, &(pxParameters->ulTimeCaptureI), NULL);
}

//Interrupción del timer asociado al core 1:
void IRAM_ATTR timer_isr2(void* arg) {
    //Inicializar argumentos:
    xTIMERParameters *pxParameters;
    pxParameters = (xTIMERParameters *) arg;
    //Aumentar el tiempo:
    pxParameters->ulTimeCaptureV += PERIODO;
    // La conversión ha finalizado, puedes leer el valor del ADC2 aquí
    uint32_t adc_value = adc2_get_raw((adc2_channel_t)ADC_CHANNEL2, ADC_WIDTH_BIT_12, &adc_value);
    // Enviar el valor del ADC a la cola de mensajes
    xQueueSendFromISR(adc2_queue, &adc_value, NULL);
    xQueueSendFromISR(time2_queue, &(pxParameters->ulTimeCaptureV), NULL);
}

//Inicializar timers:
static void initTimers()
{
    initADCs(); //Inicializar los ADcs.

    //Argumentos de la función:
    xTIMERParameters * volatile pxTIMERparameters;
    pxTIMERparameters = (xTIMERParameters *) pvPortMalloc(sizeof( xTIMERParameters));
    pxTIMERparameters->ulTimeCaptureI = 0;
    pxTIMERparameters->ulTimeCaptureV = 0;

    //Inicializar el timer en el Core 0:
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
    // Configurar la frecuencia del temporizador
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, FRECUENCIA); // 1 segundo (1 MHz clock)
    // Establecer la función de interrupción del temporizador
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_isr, pxTIMERparameters, ESP_INTR_FLAG_IRAM, NULL);

    // Inicializar el temporizador en el CORE 1:
    timer_init(TIMER_GROUP_1, TIMER_0, &timer_config);
    // Configurar la frecuencia del temporizador
    timer_set_counter_value(TIMER_GROUP_1, TIMER_0, 0x00000000ULL);
    timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, FRECUENCIA);  // 1 segundo (1 MHz clock)
    // Establecer la función de interrupción del temporizador
    timer_isr_register(TIMER_GROUP_1, TIMER_0, timer_isr2, pxTIMERparameters, ESP_INTR_FLAG_IRAM, NULL);
}

//Inicializando las tareas:
void setupTaskADCs()
{
    //Definir un puntero a una estructura para pasa argumentos a la tarea.
    xADCParameters * pxADCParameters;

    //Pasar Argumentos:
    pxADCParameters = ( xADCParameters * ) pvPortMalloc( sizeof( xADCParameters ) ); 
    pxADCParameters->Vmax = 0;
    pxADCParameters->Imax = 0;
    pxADCParameters->corteRefVt = 0;
    pxADCParameters->corteRefIt = 0;

    //Definir la Tarea para el Procesamiento de la Señal de Corriente:
    taskADC1.taskId = corrienteProcess; 
    taskADC1.name = "corrienteProcess";
    taskADC1.usStackDepth = SIZE_TASK_ADC;
    taskADC1.pvParameters = pxADCParameters;
    taskADC1.uxPriority = tskIDLE_PRIORITY;
    taskADC1.pvCreatedTask = (TaskHandle_t *) NULL;
    taskADC1.iCore = 0;

    //Definir la Tarea para el procesamiento de la Señal de Voltaje:
    taskADC2.taskId = voltProcess; 
    taskADC2.name = "voltProcess";
    taskADC2.usStackDepth = SIZE_TASK_ADC;
    taskADC2.pvParameters = pxADCParameters;
    taskADC2.uxPriority = tskIDLE_PRIORITY;
    taskADC2.pvCreatedTask = (TaskHandle_t *) NULL;
    taskADC1.iCore = 1;
}
