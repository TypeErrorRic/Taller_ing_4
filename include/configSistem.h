#ifndef CONFIG_SISTEM_H
#define CONFIG_SISTEM_H

// Librerias para la declaración de objectos RTOS:
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#define configTICK_RATE_HZ 1000  // Esto define un tick rate de 1000 ticks por segundo (1 milisegundo por tick)

typedef void (*function)(void *);

// Estructura de definicion de tareas:
typedef struct
{
    function taskId;
    char * name;
    configSTACK_DEPTH_TYPE usStackDepth;
    void *pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t *pvCreatedTask;
    BaseType_t iCore;
    
} taskDefinition;

#endif