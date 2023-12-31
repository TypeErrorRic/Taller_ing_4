/**
 * @file Proyecto de Taller de Ingeniería IV.
 * @author Ricardo Pabón Serna.(ricardo.pabon@correounivalle.edu.co)
 * @brief Este archivo contiene las funciones para inicialización y creación de tareas.
 * @version 0.1
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 */

#include "..\..\include\modulosConfig.h"

// Lista de tareas a ejecutar:
const taskDefinition *listTask[] = {&taskADCProcessI, &taskADCProcessV, &taskVoltMaxV, &taskCorrMaxI,
                                    &taskCorrCorI, &taskVoltCorV, &taskAngle, &taskPower};

esp_err_t createTask()
{
    // Identificador de la creación de la tarea:
    static const char *TAG = "Create Task";

    if (sizeof(listTask) > 0)
    {
        // Inicialización de tareas:
        for (short positionList = 0; positionList < (short)(sizeof(listTask) / sizeof(listTask[0])); positionList++)
        {
            // Creación de tareas en relación a la manera de como están enlistadas en taskDefinition:
            xTaskCreatePinnedToCore(listTask[positionList]->taskId, listTask[positionList]->name, listTask[positionList]->sizeTask,
                                    listTask[positionList]->pvParameters, listTask[positionList]->uxPriority, listTask[positionList]->pvCreatedTask,
                                    listTask[positionList]->iCore);
            ESP_LOGI(TAG, "La tarea: %s. Se ha creado", listTask[positionList]->name);
        }
        return ESP_OK;
    }
    // Retornanr error si no se manda a inicializar ninguna Tarea:
    ESP_LOGW(TAG, "No se ha asignado ninguna tarea.");
    return ESP_ERR_NOT_FOUND;
}

// Inicialización de variables:
esp_err_t initDrivers()
{
    // Identificador de inicialización de elementos:
    static const char *TAG = "Init";
    initElementsADCs();
    ESP_LOGI(TAG, "ADC Inicializado.");
    initElementsPower();
    ESP_LOGI(TAG, "Power elements Inicializados.");
    return ESP_OK;
}
