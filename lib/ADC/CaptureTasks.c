#include <ADC.h>
#include <stdio.h>

//Implementación de la tarea de ejecución del ADC.
void corrienteProcess(void *pvParameters)
{
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *) pvParameters;
    for(;;)
    {
         uint32_t adc_value;
        
        // Espera a que llegue un mensaje a la cola
        if (xQueueReceive(adc2_queue, &adc_value, portMAX_DELAY)) {
            // Procesa el valor del ADC
            // ...
        }
    }
}


//Creación de la tarea para el ADC 2:
void voltProcess(void *pvParameters)
{
    xADCParameters *pxParameters;
    pxParameters = (xADCParameters *) pvParameters;
    for(;;)
    {
        uint32_t adc_value;
        
        // Espera a que llegue un mensaje a la cola
        if (xQueueReceive(adc2_queue, &adc_value, portMAX_DELAY)) {
            // Procesa el valor del ADC
            // ...
        }
    }
};