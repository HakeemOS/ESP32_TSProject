#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>


#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0; 
#else 
    static const BaseType_t app_cpu = 1; 
#endif

#define led_pin 13    

//Same colour as default esp_logi output
#define logCGreen "\033[0;"

const char msg[] = "Another Day Engineering :D";            

static TaskHandle_t T1 = NULL; 
static TaskHandle_t T2 = NULL; 

//Prints the task name, current msg index and character at index to serial stepping through msg one character at a time, delaying for one second when complete; lowest priority task 
void TaskOne(void *arg){
    int msgLen = strlen(msg); 
    char* taskName = pcTaskGetName(NULL);
    while (1){
        printf("\n"); 
        for (size_t i = 0; i < msgLen; i++){
            ESP_LOGI(taskName, "%s i = %d - %c", logCGreen, i, msg[i]); 
        }
        vTaskDelay(1000/portTICK_PERIOD_MS); 
    }
}

//prints an asterisk every .2s 
void TaskTwo (void *arg){
    while (1){
        printf("*"); 
        vTaskDelay(200/portTICK_PERIOD_MS); 
    }
    
}

//turns on/off the onboard LED every 2s; highest priority task 
void blinkFunc(void* parameter){
    char lvl = 0; 
    while(1){
        vTaskDelay(2000/portTICK_PERIOD_MS);                                        //Period = 4s; Duty cycle = 50%
        gpio_set_level(led_pin, lvl); 
        lvl = !lvl; 
    }
}

void setup(){
    vTaskDelay(1000/portTICK_PERIOD_MS); 
    printf("\n"); 
    printf("Task Demo - w/Preemption...\n");                                        //Begin commenting out from here! 
    char *taskName = pcTaskGetName(NULL);  
    ESP_LOGI(taskName, "%s cCore ID: %d   Task Priority: %d \n", logCGreen, xPortGetCoreID(),  uxTaskPriorityGet(NULL)); 
    gpio_reset_pin(led_pin); 
    gpio_set_direction(led_pin, GPIO_MODE_OUTPUT);     
    xTaskCreatePinnedToCore(TaskOne, "Task 1", 2048, NULL, 1, &T1, app_cpu);        //Task one forced to run single core; sets priority 
    xTaskCreatePinnedToCore(TaskTwo, "Task 2", 1024, NULL, 2, &T2, app_cpu);        //Task two forced to run on same core; sets priority 
    xTaskCreatePinnedToCore(blinkFunc, "Blinky", 1024, NULL, 3, NULL, app_cpu );    //blinkFunc forced to run on same core; sets priority 

}

void app_main(void){
    setup(); 
    while (1){                                                                      //Suspend Task 2 every 2s 4 times 
        for (size_t i = 0; i < 4; i++)
        {
            vTaskDelay(2000/portTICK_PERIOD_MS); 
            vTaskSuspend(T2); 
            vTaskDelay(2000/portTICK_PERIOD_MS); 
            vTaskResume(T2); 
        }
        if (NULL != T1){                                                            //Delete task one
            vTaskDelete(T1); 
            T1 = NULL;

        }
  
    }
    

}