#ifndef POWER_H
#define POWER_H

#include <ADC.h>
#include <driver/dac.h>

// Indices para inidicar la potencia:
#define ACTIVE 0
#define REACTIVE 1
#define ACTIVE_POWER_CHANNEL 25
#define REACTIVE_POWER_CHANNEL 26
#define GPIO_PIN GPIO_NUM_23
#define GPIO_PIN2 GPIO_NUM_22
#define GPIO_PIN3 GPIO_NUM_21
#define GPIO_PIN4 GPIO_NUM_20
#define DAC_MAX_VALUE 48

/****************** Inicializar Power Task ******************/
void initElementsPower();
void createChannelDAC();

#endif