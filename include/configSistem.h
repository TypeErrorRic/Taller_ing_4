#ifndef CONFIG_SISTEM_H
#define CONFIG_SISTEM_H

// Librerias para la declaración de tareas:
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

typedef void (*function)(void);

// Estructura de definicion de tareas:
typedef struct
{
    function taskId;
    char * name;
    configSTACK_DEPTH_TYPE usStackDepth;
    void *pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t *pvCreatedTask;

} taskDefinition;

#endif