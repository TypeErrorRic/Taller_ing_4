#ifndef POWER_H
#define POWER_H

#include <ADC.h>
#include <driver/dac.h>

// Indices para inidicar la potencia:
#define ACTIVE 0
#define REACTIVE 1
#define ACTIVE_POWER_CHANNEL 25
#define REACTIVE_POWER_CHANNEL 26

#define DAC_MAX_VALUE 48

/****************** Inicializar Power Task ******************/
void initElementsPower();
void createChannelDAC();

#endif