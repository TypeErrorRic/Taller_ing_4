/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este archivo se encarga de declarar e inicializar todas los objectos de freeRTOS usados.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

#include <ADC.h>

// Declaración de las colas para la trasmición de mensajes de los ADCs:
QueueHandle_t adc1_queue;  // Cola con los valores del ADC de Corriente.
QueueHandle_t adc2_queue;  // Cola con los valores del ADC de Voltaje.
QueueHandle_t time1_queue; // Cola con los valores del instante de Captura de la Corriente.
QueueHandle_t time1_RTOS;  // Valor del instante en que el sistema empezo a tomar muestras en relación con el reloj de Freertos.
QueueHandle_t time2_queue; // Cola con los valores del instante de Captura del Voltaje.
QueueHandle_t time2_RTOS;  // Valor del instante en que el sistema empezo a tomar muestras en relación con el reloj de Freertos.
QueueHandle_t powerData;   // Cola para la trasmición de datos de los datos de potencia activa y reactiva.

// Semaforos de control del ADCs:
SemaphoreHandle_t xMutexProcess1; // Declaración del semáforo del core 0.
SemaphoreHandle_t xMutexProcess2; // Declaración del semáforo del core 1.

// Semaforo para manejar la escritura de los datos:
SemaphoreHandle_t xWriteProcessMutex1;
SemaphoreHandle_t xWriteProcessMutex2;

// Regulador de acceso para sincronización de modificación del arreglo de process:
SemaphoreHandle_t xReadCount1;
SemaphoreHandle_t xReadCount2;

// Regular el accesso de escritura y lectura de datos sobre el ángulo:
SemaphoreHandle_t xWriteAngle;

// Regulador de modifición de los arreglos de Volt y Cor. Están cruzados:
SemaphoreHandle_t xValueCor;
SemaphoreHandle_t xValueVolt;

//Sincronizadores:
SemaphoreHandle_t xControl1;
SemaphoreHandle_t xControl2;

//Control:
unsigned short resetI; 
unsigned short resetV; 
unsigned short ban;

// Inicializar Tareas de captura:
void initTask()
{
  // Inicializar los mutex para manejar la transferencia de datos:
  xMutexProcess1 = xSemaphoreCreateMutex();
  xMutexProcess2 = xSemaphoreCreateMutex();
  // Inicializar colas de trasmición:
  // Core 1
  adc1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(unsigned int));
  time1_queue = xQueueCreate(QUEUE_LENGTH, sizeof(unsigned long long));
  time1_RTOS = xQueueCreate(1, sizeof(unsigned long));
  // Core 2
  adc2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(unsigned int));
  time2_queue = xQueueCreate(QUEUE_LENGTH, sizeof(unsigned long long));
  time2_RTOS = xQueueCreate(1, sizeof(unsigned long));
  // Crea un mutex para controlar la escritura de de datos en el arreglo:
  xWriteProcessMutex1 = xSemaphoreCreateMutex();
  xWriteProcessMutex2 = xSemaphoreCreateMutex();
  // Crea un semaforo contador para controlar el aceso de lectura.
  xReadCount1 = xSemaphoreCreateCounting(2, 2);
  xReadCount2 = xSemaphoreCreateCounting(2, 2);
  // Crea un mutex para controlar el aceeso de escritura del ángulo.
  xWriteAngle = xSemaphoreCreateCounting(2, 2);
  // Crea un semaforo contador para controlar la modificación de los arreglos Cor Y Volt:
  xValueCor = xSemaphoreCreateBinary();
  xValueVolt = xSemaphoreCreateBinary();
  //Cola para trasmición de datos de la potencia activa y reactiva:
  powerData = xQueueCreate(1, sizeof(double[2]));
  //Sincronizadores para iniciar la captura a tiempos similares:
  xControl1 = xSemaphoreCreateBinary();
  xControl2 = xSemaphoreCreateBinary();
  //Control:
  resetI = 0;
  resetV = 0;
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
  setupTaskCalculeAngle();
  // Configuración de timers:
  init_timers();
}