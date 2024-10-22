#include "lora_init.h"
#include "lora_gateway.h"
#include "lora_file_transfer.h"
#include "lora_peer.h"
#include "SD.h"
#include "SPIFFS.h"

TimerHandle_t watchdogTimer;
TaskHandle_t scheduled_poll_handle;
volatile bool functionRunning = false;
bool poll_success = false;
int rssi = 0;

void scheduled_poll(void *parameter);

/******************************************************************
 *                                                                *
 *                        Receive Control                         *
 *                                                                *
 ******************************************************************/

// *************************************
// * OnReceive Handlers Registration
// *************************************
void OnDataRecvGateway(const uint8_t *incomingData, int len) { 
  
  uint8_t type = incomingData[0];       // first message byte is the type of message 

  switch (type) {
    case PAIRING:                            // the message is a pairing request 
      handle_pairing(incomingData);
      break;
    case FILE_BODY:
      // Serial.println("Received FILE_BODY");
        handle_file_body(incomingData);
      break;
    case FILE_ENTIRE:
      // Serial.println("Received FILE_ENTIRE");
      handle_file_entire(incomingData);
      break;
    case POLL_COMPLETE:
      // Serial.println("Received POLL_COMPLETE");
      rssi = LoRa.packetRssi();
      poll_success = true;
      break;
    default:
      Serial.println("Unkown message type.");
  }
}

/******************************************************************
 *                                                                *
 *                         Send Control                           *
 *                                                                *
 ******************************************************************/


int waitForPollAck() {
  unsigned long startTime = millis();
  while (millis() - startTime < POLL_TIMEOUT) {
    if(poll_success){
      poll_success = false;
      return true;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS); // Delay for 1 second
  }
  return false;
}

/******************************************************************
 *                         Control Tasks                          *
 ******************************************************************/

// Schedule Watchdog
void watchdogCallback(TimerHandle_t xTimer) {
    // This function gets called when the timer expires
    logErrorToSPIFFS("Error: watchdogCallback, time expired");
    if (functionRunning) {
        logErrorToSPIFFS("scheduled_poll task timed out. Restarting task...");
        Serial.println("scheduled_poll task timed out. Restarting task...");
        vTaskDelete(scheduled_poll_handle); // Terminate the stuck task
        functionRunning = false;
        
        // Create the function task again
        xTaskCreate(scheduled_poll, "FunctionTask", 2048, NULL, 1, &scheduled_poll_handle);
    }
}

void scheduled_poll(void *parameter){

  while(true){


    unsigned long currentTime = millis();

    if(peerCount == 0){
      vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
      continue;
    }

    for (int i = 0; i < lora_config.scheduleCount; i++){
      LoRaPollSchedule schedule = lora_config.schedules[i];
      LoRaPollFunc func = schedule.func;
      unsigned long lastPoll = schedule.lastPoll;
      unsigned long interval = schedule.interval;
      int isBroadcast = schedule.isBroadcast;

      if ((currentTime - lastPoll) >= interval) {

        Serial.println();Serial.println("================================");
        Serial.print("Beging Schedule: "); Serial.print(i);
        Serial.print(", lastPoll: "); Serial.print(lastPoll);
        Serial.print(", interval: "); Serial.print(interval);
        Serial.print(", currentTime: "); Serial.println(currentTime);
        
        lora_config.schedules[i].lastPoll = currentTime;
        for(int j = 0; j < peerCount; j++){
          
          rej_switch = 0; // turn on file transfer

          if (xSemaphoreTake(xMutex_DataPoll, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
            poll_success = false;

            // Start the watchdog timer
            functionRunning = true;
            xTimerStart(watchdogTimer, 0);
            func(j);
            // If the function, stop the watchdog timer
            xTimerStop(watchdogTimer, 0);
            functionRunning = false;

            if (!xSemaphoreGive(xMutex_DataPoll)) {
              Serial.println("Error: Failed to release xMutex_DataPoll");
              logErrorToSPIFFS("Error: Schedule: "); logErrorToSPIFFS(String(i));
              logErrorToSPIFFS("Error: Failed to release xMutex_DataPoll");

              // Attempt recovery actions, such as reinitializing resources
              // This example just logs the attempt; real recovery logic would be more complex
              vSemaphoreDelete(xMutex_DataPoll);
              Serial.println("Deleted xMutex_DataPoll");
              xMutex_DataPoll = xSemaphoreCreateMutex();
              Serial.println("Recreated xMutex_DataPoll");
              if (xMutex_DataPoll == NULL) {
                Serial.println("Error: Failed to reinitialize xMutex_DataPoll");
                logErrorToSPIFFS("Error: Failed to reinitialize xMutex_DataPoll");
                ESP.restart();  // Reset the system as a last resort
              }

            }
          } else {
            Serial.println("Error: Failed to obtain xMutex_DataPoll");
          }

          if(isBroadcast){
            Serial.println("Is broadcast, no need to wait for confirmation.");
            continue;
          }

          if(waitForPollAck()){ // check for ack before proceeding to next one
            struct tm timeinfo;
            if(!getLocalTime(&timeinfo)){
              Serial.println("Failed to obtain time.");
              logErrorToSPIFFS("Error: Failed to obtain time.");
            }
            peers[j].lastCommTime = timeinfo;
            peers[j].status = ONLINE;
            peers[j].SignalStrength = rssi;
            Serial.println("Received poll success.");
            Serial.println("Updated LoRa communication stats.");
          }
          else{ // still in transmission or slave is non responsive

            rej_switch = true;
            Serial.println("rej_switch = true");
            vTaskDelay(1000 / portTICK_PERIOD_MS); // delay to allow the gateway handlers to send REJ to slaves

          }
        }
        Serial.print("Completed Schedule: "); Serial.println(i);
      }
      
    }

    // Sleep for a short interval before next check (if needed)
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
  }
}

/******************************************************************
 *                                                                *
 *                        Initialization                          *
 *                                                                *
 ******************************************************************/

void lora_gateway_init() {

  LoRa.onReceive(onReceive);
  LoRa.receive();

  loadPeers();

  // Ensure the "data" directory exists
  if (!SD.exists("/node")) {
    Serial.print("Creating 'node' directory - ");
    if (SD.mkdir("/node")) {
      Serial.println("OK");
    } else {
      Serial.println("Failed to create 'node' directory");
    }
  }
  
  Serial.println("Finished checking node dir");

  // Create the task for the receive loop
  xTaskCreate(
    taskReceive,
    "Data Receive Handler",
    10000,
    (void*)OnDataRecvGateway,
    3,
    NULL
  );

  Serial.println("Added Data receieve handler");

  // Create a watchdog timer with a timeout of 5 seconds (5000 ms)
  watchdogTimer = xTimerCreate("WatchdogTimer", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, watchdogCallback);

  // Create the task for the control loop
  xTaskCreate(
    scheduled_poll,    // Task function
    "Control Task",     // Name of the task
    10000,              // Stack size in words
    NULL,               // Task input parameter
    1,                  // Priority of the task
    &scheduled_poll_handle // Task handle
  );
  Serial.println("Added Data send handler");

  // sync_folder_request(peers[0].mac);

  
}