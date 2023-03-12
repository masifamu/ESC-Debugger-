#include <Arduino_FreeRTOS.h>

#define BOARD_LED 13

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  xTaskCreate(ledControllerTask, "led_controller_task", 50, NULL, 1, NULL);
  xTaskCreate(displayControllerTask, "display_controller_task", 50, NULL, 1, NULL);
  xTaskCreate(rpmControllerTask, "rpm_controller_task", 50, NULL, 1, NULL);
  xTaskCreate(buttonControllerTask, "button_controller_task", 50, NULL, 1, NULL);
  xTaskCreate(dataStorageControllerTask, "data_storage_controller_task", 50, NULL, 1, NULL);
  xTaskCreate(commControllerTask, "comm_controller_task", 50, NULL, 1, NULL);

}

void displayControllerTask(void *pvParameter){
  while(1){
//    Serial.println("displayControllerTask is Running");
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  
}

void rpmControllerTask(void *pvParameter){
  while(1){
//    Serial.println("rpmControllerTask is Running");
    vTaskDelay(pdMS_TO_TICKS(100));
  }  
}
void buttonControllerTask(void *pvParameter){
    while(1){
//      Serial.println("buttonControllerTask is Running");
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void dataStorageControllerTask(void *pvParameter){
    while(1){
//    Serial.println("dataStorageControllerTask is Running");
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void ledControllerTask(void *pvParameter){
  pinMode(BOARD_LED,OUTPUT);
  while(1){
    digitalWrite(BOARD_LED, digitalRead(BOARD_LED)^1);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
void commControllerTask(void *pvParameter){
  while(1){
    Serial.println("CommControllerTask is Running");
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
