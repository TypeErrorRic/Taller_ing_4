#include <ADC.h>

// Declaración de las colas para la trasmición de mensajes de los ADCs:
QueueHandle_t adc1_queue;  // Cola con los valores del ADC de Corriente.
QueueHandle_t adc2_queue;  // Cola con los valores del ADC de Voltaje.
QueueHandle_t time1_queue; // Cola con los valores del instante de Captura de la Corriente.
QueueHandle_t time2_queue; // Cola con los volores del instante de Captura del Voltaje.

// Semaforos de control del ADCs:
SemaphoreHandle_t xMutexProcess1; // Declaración del semáforo del core 0.
SemaphoreHandle_t xMutexProcess2; // Declaración del semáforo del core 1.

// Semaforo para manejar la escritura de los datos:
SemaphoreHandle_t xWriteProcessMutex1;
SemaphoreHandle_t xWriteProcessMutex2;

// Regulador de acceso para sincronización de modificación del arreglo de process:
SemaphoreHandle_t xReadCount1;
SemaphoreHandle_t xReadCount2;

// Inicializar Tareas de captura:
void initTask()
{
    // Inicializar los mutex para manejar la transferencia de datos:
    xMutexProcess1 = xSemaphoreCreateMutex();
    xMutexProcess2 = xSemaphoreCreateMutex();
    // Inicializar colas de trasmición:
    adc1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint16_t));
    time1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint64_t));
    adc2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint16_t));
    time2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(uint64_t));
    // Crea un mutex para controlar la escritura de de datos en el arreglo:
    xWriteProcessMutex1 = xSemaphoreCreateMutex();
    xWriteProcessMutex2 = xSemaphoreCreateMutex();
    // Crea un mutex para controlar el aceeso de lectura.
    xReadCount1 = xSemaphoreCreateCounting(2, 0);
    xReadCount2 = xSemaphoreCreateCounting(2, 0);
}

// Función con las llamadas requeridas para realizar la configuración de los ADCs.
void initElementsADCs()
{
    // Configuración de estrucutras de trasmición de datos:
    initTask();
    // Configuración de tareas:
    setupTaskProcessADCs();
    setupTaskCalculeProcess();
    setupTaskCalculeCorProcess();
    // Configuración de timers:
    init_timers();
}