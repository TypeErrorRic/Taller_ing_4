#ifndef CONFIG_SISTEM_H
#define CONFIG_SISTEM_H

// Librerias para la declaración de objectos RTOS:
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>
#include <esp_log.h>

typedef void (*function)(void *);

// Estructura de definicion de tareas:
typedef struct
{
    function taskId;
    char * name;
    void *pvParameters;
    UBaseType_t uxPriority;
    uint32_t sizeTask;
    TaskHandle_t *pvCreatedTask;
    BaseType_t iCore;
    
} taskDefinition;

// El valor de factor de espera puede estar entre 3 y 10.
#define FACTOR_ESPERA 3

#endif