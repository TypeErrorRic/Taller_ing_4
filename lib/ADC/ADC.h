#ifndef ADC_H
#define ADC_H

#include "../../include/configSistem.h"
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/adc.h>

/*--------------- Configuración del ADC ----------------*/

#define FRECUENCIA 900              // Frecuencia de Muestreo.
#define PROMEDIO 3                  // Número de muestras promediadas. No pasar de 5

#define ADC_CHANNEL1 ADC1_CHANNEL_0 // Canal de ADC1
#define ADC_CHANNEL2 ADC2_CHANNEL_0 // Canal de ADC2

// Definir el tamaño de la cola:
#define QUEUE_LENGTH 10

// Estructura de captura de datos de los ADCs:
typedef struct Capture_Parameters
{
    uint32_t listADC_I[QUEUE_LENGTH]; // Lista de valores de valores de la captura del ADC del core 0
    uint32_t listADC_V[QUEUE_LENGTH]; // Lista de valores de valores de la captura del ADC del core 1
    unsigned long listT_I[QUEUE_LENGTH];     // Instantes de captura del core 0
    unsigned long listT_V[QUEUE_LENGTH];     // Instantes de captura del core 1

} xCaptureParameters;

/****************** Definición de tareas ****************/

/*----------------------- Core 0 -----------------------*/
extern taskDefinition taskADCCaptureI;  // Tarea 1 asociado a la captura de datos.
extern taskDefinition taskADCProcessI;  // Tarea 1 asociado al procesamiento de los datos.
extern taskDefinition taskCorrMaxI;     // Tarea asociado a la obtención del Imax.
extern taskDefinition taskCorrCorI;     // Tarea absociado a la obtención del punto de corte de I.

/*----------------------- Core 1 -----------------------*/
extern taskDefinition taskADCCaptureV;  // Tarea 2 asociado a la captura de datos.
extern taskDefinition taskADCProcessV;  // Tarea 2 asociado al procesamiento de los datos.
extern taskDefinition taskVoltMaxV;     // Tarea asociado a la obtención del Vmax.
extern taskDefinition taskVoltCorV;     // Tarea absociado a la obtención del punto de corte de V.

/**Configuración de estructuras de trasmición de datos**/

extern QueueHandle_t adc1_queue;  // Cola con los valores del ADC de Corriente.
extern QueueHandle_t adc2_queue;  // Cola con los valores del ADC de Voltaje.
extern QueueHandle_t time1_queue; // Cola con los valores del instante de Captura de la Corriente.
extern QueueHandle_t time2_queue; // Cola con los volores del instante de Captura del Voltaje.

extern SemaphoreHandle_t xMutexProcess1; // Disparador de Procesamiento de datos core 0.
extern SemaphoreHandle_t xMutexProcess2; // Disparador de Procesamiento de datos core 1.

extern SemaphoreHandle_t xWriteProcessMutex1; //Semaforo de control de aceso de escritura al arreglo en el core 1.
extern SemaphoreHandle_t xWriteProcessMutex2; //Semaforo de control de aceso de escritura al arreglo en el core 2.
extern SemaphoreHandle_t xReadCount1;         //Semaforo de control de lectura de datos del arreglo en el core 1.
extern SemaphoreHandle_t xReadCount2;         //Semaforo de control de lectura de datos del arreglo en el core 2.

/**** Configuración de Parametros de tiempo del ADC ****/
typedef struct ADC_Parameters
{
    xCaptureParameters *pxdata;   // Datos de la captura del ADC.

    double dVmax;                // Valor maximo del Voltaje.
    double dImax;                // Valor Maximo de la corriente.
    double dcorteRefVt;          // Punto de corte con el nivel de referencia del voltaje.
    double dcorteRefIt;          // Punto de corte con el nivel de referencia de la corriente.
} xADCParameters;

extern xADCParameters *pxADCParameters; // Estructua creada.

/***************** Creación de Tareas *****************/

// Inicialización de tareas de captura de datos:
void initTaskCapture();
//Manejadores de las tareas:
extern TaskHandle_t xTaskCorrCaptureI;
extern TaskHandle_t xTaskVoltCaptureV;

// Inicalización de tareas de Procesamiento de datos:
void setupTaskProcessADCs();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrProcessI;
extern TaskHandle_t xTaskVoltProcessV;

//Inicialización de tareas de calculo de Max datos:
void setupTaskCalculeProcess();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrMaxI;
extern TaskHandle_t xTaskVoltMaxV;

//Inicialización de tareas de calculo de corte datos:
void setupTaskCalculeCorProcess();
// Manejadores de las tareas:
extern TaskHandle_t xTaskCorrCorI;
extern TaskHandle_t xTaskVoltCorV;

/****************** Inicializar ADCs ******************/
void initElementsADCs();

#endif