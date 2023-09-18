#include "configSistem.h"

#define SIZE_TASK_ADC 1024

//Parametros del ADC:
typedef struct ADC_Parameters
{
    unsigned short usADC1;			    // Identificador del puerto del ADC 0.
    unsigned short usADC2;			    // Identificador del puerto del ADC 1.
    unsigned int uiADCRate;		        // La frecuencia de operación.
} xADCParameters;

//Definición de tareas:
extern taskDefinition taskADC1;
extern taskDefinition taskADC2;

//Configuración del ADC:
esp_err_t setupTaskADCs();

//Creación de la tarea para el ADC 1:
void ADCtask1(void *pvParameters);

//Creación de la tarea para el ADC 2:
void ADCtask2(void *pvParameters);