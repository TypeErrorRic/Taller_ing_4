#include "modulosConfig.h"

esp_err_t createTask()
{
    static const char *TAG = "Create Task";
    if(sizeof(listTask) > 0)
    {
        for (short positionList = 0; positionList < sizeof(listTask); positionList++)
        {
            xTaskCreate(listTask[positionList]->taskId, listTask[positionList]->name, listTask[positionList]->usStackDepth,
                listTask[positionList]->pvParameters, listTask[positionList]->uxPriority, listTask[positionList]->pvCreatedTask);
            ESP_LOGI(TAG, "La tarea: %s. Se ha creado", listTask[positionList]->name);
        }
        return ESP_OK;
    }
    ESP_LOGW(TAG, "No se ha asignado ninguna tarea.");
    return ESP_ERR_NOT_FOUND;
}