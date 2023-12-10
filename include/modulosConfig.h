/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este es el Header para la instanciación de tareas e inicialización de objectos FreeRTOS.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

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