#include <Power.h>

static const char *TAG = "Power trasmition";

//Definición de la tarea:
taskDefinition taskPower; 

static void vPowerTrasmition(void *pvParameters)
{
    double datosPower[2] = {};
    for(;;)
    {
        //Recivir datos: Si no están listos se queda esperando.
        xQueueReceive(powerData, &datosPower, portMAX_DELAY);

        //Procesamiento para la salida:
        printf("Potencia Activa : %f\n", datosPower[ACTIVE]);
        printf("Potencia Reactiva : %f\n", datosPower[REACTIVE]);

        //Fin de la tarea:
        ESP_LOGI(TAG, "Tarea Power");
    }
}

void initElementsPower()
{
    taskPower.taskId = vPowerTrasmition;
    taskPower.name = "Power Salidas";
    taskPower.pvParameters = NULL;
    taskPower.sizeTask = 2 * configMINIMAL_STACK_SIZE;
    taskPower.uxPriority = 3; 
    taskPower.pvCreatedTask = NULL;
    taskPower.iCore = 1;   
}