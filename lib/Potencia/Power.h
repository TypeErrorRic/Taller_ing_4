#ifndef POWER_H
#define POWER_H

#include <ADC.h>

// Inicialización de la tarea de calculo de la potencia activa:
void setupTaskCalculePower();
// Manejadores de la tarea:
extern TaskHandle_t xTaskActivePower;
extern TaskHandle_t xTaskReactivePower;

/****************** Inicializar POWER Tasks ******************/
void initElementsPower();

#endif