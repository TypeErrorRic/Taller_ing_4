#ifndef MODULOS_CONFIG_H
#define MODULOS_CONFIG_H

// Elementos FreeRTOS:
#include <stdio.h>
#include <string.h>

//Configuracion del sistema:
#include "configSistem.h"

//Librerías privadas creadas:
#include <ADC.h>
#include <Power.h>

// Función para inicializar el microcontrolador.
esp_err_t initDrivers(void);

// Función para arrancar las tareas.
esp_err_t createTask();

#endif