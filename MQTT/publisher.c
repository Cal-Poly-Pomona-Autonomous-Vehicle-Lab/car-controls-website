#include <stdio.h> 
#include <string.h>

#include "mosquitto.h"

int main() {
    struct mosquitto *mosq; 
    ambient_t ambient; 

    char mqtt_channel_name[256]; 

    start_arg_t start_arg = {
        .broker_hostname = "localhost", 
        .broker_port = 1883, 
        .location = "location
    } 
    
    mosquitto_lib_init(); 

    mosq = mosquitto_new(NULL, true, NULL); 
    
    if (!mosq) 
        printf("Failed to create mosq\n"); 

    if (moquitto_connect(mosq, start_arg.broker_name, start_arg.broker_port, 60) != MSQ_ERR_SUCESS) {
        printf("Failed to connect to MQTT Broker\n"); 


    while (1) {
    } 
    
    
} 
