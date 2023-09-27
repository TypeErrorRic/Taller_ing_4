#include "..\..\include\modulosConfig.h"

//Lista de tareas a ejecutar:
const taskDefinition * listTask[] = {&taskADCCaptureI, &taskADCCaptureV, &taskADCProcessI, &taskADCProcessV, 
    &taskCorrMaxI, &taskVoltMaxV /*Aquí se alistan las tareas*/};

esp_err_t createTask()
{
    static const char *TAG = "Create Task";
    if(sizeof(listTask) > 0)
    {
        //Inicialización de tareas:
        for (short positionList = 0; positionList < (short)(sizeof(listTask)/sizeof(listTask[0])); positionList++)
        {
            //Creación de tareas en relación a la manera de como están enlistadas en taskDefinition:
            xTaskCreatePinnedToCore(listTask[positionList]->taskId, listTask[positionList]->name, listTask[positionList]->usStackDepth,
                listTask[positionList]->pvParameters, listTask[positionList]->uxPriority, listTask[positionList]->pvCreatedTask,
                listTask[positionList]->iCore);
            ESP_LOGI(TAG, "La tarea: %s. Se ha creado", listTask[positionList]->name);
        }
        return ESP_OK;
    }
    //Retornanr error si no se manda a inicializar ninguna Tarea:
    ESP_LOGW(TAG, "No se ha asignado ninguna tarea.");
    return ESP_ERR_NOT_FOUND;
}

//Inicialización de variables: 
esp_err_t initDrivers()
{
    initElementsADCs();
    return ESP_OK;
}