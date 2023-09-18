#include <ADC.h>
#include <stdio.h>

//Estructura para declaración de tareas:
taskDefinition taskADC1; 
taskDefinition taskADC2;

void setupTaskADCs()
{
    //Definir un puntero a una estructura para pasa argumentos a la tarea.
    xADCParameters * pxADCParameters;

    //Pasar Argumentos:
    pxADCParameters = ( xADCParameters * ) pvPortMalloc( sizeof( xADCParameters ) ); 
    pxADCParameters->usADC1 = 3;
    pxADCParameters->usADC2 = 9;
    pxADCParameters->uiADCRate = 1000;

    //Definir La tarea ADC 1:
    taskADC1.taskId = ADCtask1; 
    taskADC1.name = "ADC 1";
    taskADC1.usStackDepth = SIZE_TASK_ADC;
    taskADC1.pvParameters = pxADCParameters;
    taskADC1.uxPriority = tskIDLE_PRIORITY;
    taskADC1.pvCreatedTask = (TaskHandle_t *) NULL;

    //Definir la tarea ADC 2
    taskADC2.taskId = ADCtask2; 
    taskADC2.name = "ADC 2";
    taskADC2.usStackDepth = SIZE_TASK_ADC;
    taskADC2.pvParameters = pxADCParameters;
    taskADC2.uxPriority = tskIDLE_PRIORITY;
    taskADC2.pvCreatedTask = (TaskHandle_t *) NULL;
}

//Implementación de la tarea de ejecución del ADC.
void ADCtask1(void *pvParameters)
{
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *) pvParameters;
    for(;;);
}


//Creación de la tarea para el ADC 2:
void ADCtask2(void *pvParameters)
{
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *) pvParameters;
    for(;;);
};