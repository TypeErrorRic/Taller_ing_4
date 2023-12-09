#include <Power.h>

static const char *TAG = "Power trasmition";

//Definición de la tarea:
taskDefinition taskPower; 

static void vPowerTrasmition(void *pvParameters)
{
    double datosPower[2] = {};
    uint8_t active = 0;
    u_int8_t reactive = 0;
    for(;;)
    {
        //Recivir datos: Si no están listos se queda esperando.
        xQueueReceive(powerData, &datosPower, portMAX_DELAY);


        if (datosPower[0] > DAC_MAX_VALUE) active = (uint8_t)(255);
        else active = (uint8_t)((datosPower[0] / DAC_MAX_VALUE) * 255);

        if (datosPower[1] > DAC_MAX_VALUE) reactive = (uint8_t)(255);
        else reactive = (uint8_t)((datosPower[1] / DAC_MAX_VALUE) * 255);

        //Procesamiento para la salida:
        printf("Potencia Activa : %f\n", datosPower[ACTIVE]);
        printf("Potencia Reactiva : %f\n", datosPower[REACTIVE]);
        dac_output_voltage(DAC_CHAN_0, active);
        dac_output_voltage(DAC_CHAN_1, reactive);

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
    createChannelDAC();   
}

void createChannelDAC()
{
    dac_output_enable(DAC_CHAN_0);
    dac_output_enable(DAC_CHAN_1);
}