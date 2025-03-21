#include <stdio.h> 
#include <string.h>
#include <unistd.h> 
#include <stdint.h> 
#include <getopt.h> 
#include <sys/types.h> 

#include "mosquitto.h"
    

typedef struct {
    int is_alive; 
} heart_t; 

typedef struct {
    char hostname[128]; 
    uint16_t broker_port; 
    char location[64]; 
} start_arg_t; 

int main() { 
    struct mosquitto *mosq; 
    heart_t heart = {
        .is_alive = 1
    };   

    char mqtt_channel_name[256] = "init"; 

    start_arg_t start_arg = {
        .hostname = "10.110.245.223", 
        .broker_port = 1883, 
        .location = "location",
    };
    
    mosquitto_lib_init(); 

    mosq = mosquitto_new(NULL, true, NULL); 
    
    if (!mosq) 
        printf("Failed to create mosq\n"); 

    if (mosquitto_connect(mosq, start_arg.hostname, start_arg.broker_port, 60) != MOSQ_ERR_SUCCESS) 
        printf("Failed to connect to MQTT Broker\n"); 

    

    while (1) {
        mosquitto_publish(mosq, NULL, mqtt_channel_name, sizeof(heart), &heart, 0, false);
        sleep(1);
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup(); 
} 
