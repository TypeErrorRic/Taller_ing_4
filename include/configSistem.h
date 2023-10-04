#ifndef CONFIG_SISTEM_H
#define CONFIG_SISTEM_H

// Librerias para la declaración de objectos RTOS:
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>
#include <esp_log.h>

#define configTICK_RATE_HZ 1800 //Tick del sistema.

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

#define FACTOR_ESPERA 10

#endif