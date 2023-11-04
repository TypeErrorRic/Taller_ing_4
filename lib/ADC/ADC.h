/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este es el Header del Funcionamiento del Procesamiento de las Señales.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

#ifndef ADC_H
#define ADC_H

#include "../../include/configSistem.h"
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/adc.h>
#include <driver/gptimer.h>
#include "driver/gpio.h"

/*--------------- Configuración del ADC ----------------*/

#define FRECUENCIA 900      // Frecuencia de Muestreo 900/600/300
#define FRECUENCIA_SENAL 60 // Frecuencia original de la señal a muestrear en hz.

#define ADC_CHANNEL1 ADC1_CHANNEL_0 // Canal de ADC1
#define ADC_CHANNEL2 ADC2_CHANNEL_0 // Canal de ADC2

#define RESOLUTION 1000000 // 1MHz, 1 tick = 1us Resolución del timer de WALL-CLOCK

#define NUM_LN_ONDA 3 // Número de longitudes de onda Tomadas.

/*--------------- Definiciones del ADC ----------------*/

// Definir el tamaño de la cola:
#define QUEUE_LENGTH (FRECUENCIA >= 300 ? (((unsigned int)FRECUENCIA / FRECUENCIA_SENAL) * NUM_LN_ONDA) : 10)

// Pin GPIO para el LED incorporado en el ESP32 DevKit
#define LED_PIN GPIO_NUM_2

// Estructura de captura de datos de los ADCs:
typedef struct Capture_Parameters
{
    double listADC_I[QUEUE_LENGTH]; // Lista de valores de valores de la captura del ADC del core 0
    double listADC_V[QUEUE_LENGTH]; // Lista de valores de valores de la captura del ADC del core 1
    double listT_I[QUEUE_LENGTH];   // Instantes de captura del core 0
    double listT_V[QUEUE_LENGTH];   // Instantes de captura del core 1
} xCaptureParameters;

/****************** Definición de tareas ****************/

/*----------------------- Core 0 -----------------------*/
////Interrupción Asociado a la captura del valor de la Corriente////.
extern taskDefinition taskADCProcessI;   // Tarea 1 asociada al procesamiento de los datos.
extern taskDefinition taskCorrMaxI;      // Tarea asociada a la obtención del Imax.
extern taskDefinition taskCorrCorI;      // Tarea asociada a la obtención del punto de corte de I.
extern taskDefinition taskAngle;         // Tarea asociada a la obtención del angulo de desfase entre I y V.
extern taskDefinition taskReactivePower; // Tarea asociada al calculo de la potencia reactiva.

/*----------------------- Core 1 -----------------------*/
////Interrupción Asociado a la captura del valor del Voltaje////.
extern taskDefinition taskADCProcessV; // Tarea 2 asociada al procesamiento de los datos.
extern taskDefinition taskVoltMaxV;    // Tarea asociada a la obtención del Vmax.
extern taskDefinition taskVoltCorV;    // Tarea asociada a la obtención del punto de corte de V.
extern taskDefinition taskActivePower; // Tarea asociada al calculo de la potencia activa.

/**Configuración de estructuras de trasmición de datos**/

extern QueueHandle_t adc1_queue;     // Cola con los valores del ADC de Corriente.
extern QueueHandle_t adc2_queue;     // Cola con los valores del ADC de Voltaje.
extern QueueHandle_t time1_queue;    // Cola con los valores del instante de Captura de la Corriente.
extern QueueHandle_t time1_RTOS;     // Valor del instante en que el sistema empezo a tomar muestras en relación con el reloj de Freertos.
extern QueueHandle_t time2_queue;    // Cola con los volores del instante de Captura del Voltaje.
extern QueueHandle_t time2_RTOS;     // Valor del instante en que el sistema empezo a tomar muestras en relación con el reloj de Freertos.
extern QueueHandle_t timesAng_queue; // Valores de instantes de tiempos del cruce por referencia del voltaje y la corriente.

extern SemaphoreHandle_t xMutexProcess1; // Disparador de Procesamiento de datos core 0.
extern SemaphoreHandle_t xMutexProcess2; // Disparador de Procesamiento de datos core 1.

extern SemaphoreHandle_t xWriteProcessMutex1; // Semaforo de control de aceso de escritura al arreglo en el core 1.
extern SemaphoreHandle_t xWriteProcessMutex2; // Semaforo de control de aceso de escritura al arreglo en el core 2.
extern SemaphoreHandle_t xReadCount1;         // Semaforo de control de lectura de datos del arreglo en el core 1.
extern SemaphoreHandle_t xReadCount2;         // Semaforo de control de lectura de datos del arreglo en el core 2.

extern SemaphoreHandle_t xWriteAngle; // Semaforo de control de escritura de datos sobre los valores de tiempo de corte.

extern SemaphoreHandle_t xPower1; // Semaforo de control para acceder al valor del voltaje maximo.
extern SemaphoreHandle_t xPower2; // Semaforo de control para acceder al valor de la corriente maxima.
extern SemaphoreHandle_t xPower3; // Semaforo de control para acceder al valor del angulo.

extern SemaphoreHandle_t xValueCor;  // Semaforo para controlar la modificación del tiempo en Cor.
extern SemaphoreHandle_t xValueVolt; // Semaforo para controlar la modificación del tiempo en Volt.

/**** Configuración de Parametros de tiempo del ADC ****/
typedef struct ADC_Parameters
{
    xCaptureParameters *pxdata; // Datos de la captura del ADC.

    double dVmax;                    // Valor maximo del Voltaje.
    double dImax;                    // Valor Maximo de la corriente.
    double dcorteRefVt[NUM_LN_ONDA]; // Punto de corte con el nivel de referencia del voltaje.
    double dcorteRefIt[NUM_LN_ONDA]; // Punto de corte con el nivel de referencia de la corriente.
    double dAngle;                   // Angulo desfase entre la corriente y el voltaje.
} xADCParameters;

extern xADCParameters *pxADCParameters; // Estructua creada.

/***************** Creación de Tareas *****************/

// Inicialización de estructura de datos:
void initTask();

// Inicialización de captura de datos por Interrupciones:
void init_timers();
// Manejadores de las interrupciones:
extern gptimer_handle_t gptimer1;
extern gptimer_handle_t gptimer2;

// Inicalización de tareas de Procesamiento de datos:
void setupTaskProcessADCs();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrProcessI;
extern TaskHandle_t xTaskVoltProcessV;

// Inicialización de tareas de calculo de Max datos:
void setupTaskCalculeProcess();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrMaxI;
extern TaskHandle_t xTaskVoltMaxV;

// Inicialización de tareas de calculo de corte datos:
void setupTaskCalculeCorProcess();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrCorI;
extern TaskHandle_t xTaskVoltCorV;

// Inicialización de la tarea de calculo del angulo de desfase:
void setupTaskCalculeAngle();
// Manejador de la tarea:
extern TaskHandle_t xTaskAngle;

/****************** Inicializar ADCs ******************/
void initElementsADCs();

#endif