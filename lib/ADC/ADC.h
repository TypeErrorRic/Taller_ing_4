#include "../../include/configSistem.h"
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/adc.h>
#include <driver/timer.h>

//Configuración del ADC:
#define SIZE_TASK_ADC 1024

//Configuración de las Interrupciones:

#define TIMER_INTERVAL1         1000   // Intervalo del temporizador para ADC1 (en milisegundos)
#define TIMER_INTERVAL2         500    // Intervalo del temporizador para ADC2 (en milisegundos)

#define ADC_CHANNEL1            ADC1_CHANNEL_0  // Canal de ADC1
#define ADC_CHANNEL2            ADC2_CHANNEL_0  // Canal de ADC2

// Parametros del ADC:
typedef struct ADC_Parameters
{
    double Vmax;        // Valor maximo del Voltaje.
    double Imax;        // Valor Maximo de la corriente.
    double corteRefVt;  // Punto de corte con el nivel de referencia del voltaje.
    double corteRefIt;  // Punto de corte con el nivel de referencia de la corriente.
} xADCParameters;

// Definir el tamaño de la cola:
#define QUEUE_LENGTH 10

/****************** Definición de tareas ****************/

// Tarea 1 asociado al procesamiento del ADC 0:
extern taskDefinition taskADC1;
// Tarea 2 asociado al procesamiento del ADC 1:
extern taskDefinition taskADC2;

/*********** Funciones para el funcionamiento ***********/

/*--------------- Configuración del ADC ----------------*/
#define FRECUENCIA  1000000
#define PERIODO     ((double) 1/FRECUENCIA)

void initADCs();        // Inicializar los ADCs.
void setupTaskADCs();   // Configurar la Tareas.

/*-Configuración de estructuras de trasmición de datos-*/
extern QueueHandle_t adc1_queue;   // Cola con los valores del ADC de Corriente.
extern QueueHandle_t adc2_queue;   // Cola con los valores del ADC de Voltaje.
extern QueueHandle_t time1_queue;  // Cola con los valores del instante de Captura de la Corriente.
extern QueueHandle_t time2_queue;  // Cola con los volores del instante de Captura del Voltaje.

/**** Configuración de Parametros de tiempo del ADC ****/

typedef struct TIMER_Parameters
{
    long double ulTimeCaptureI;       // Tiempo transcurrido core 0.
    long double ulTimeCaptureV;       // Tiempo transcurido core 1.
} xTIMERParameters;

/***************** Creación de Tareas *****************/

// Creación de la tarea para el ADC 1:
void corrienteProcess(void *pvParameters);
// Creación de la tarea para el ADC 2:
void voltProcess(void *pvParameters);