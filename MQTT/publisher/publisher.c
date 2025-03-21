#include <stdio.h> 
#include <stdint.h> 
#include <unistd.h> 
#include <getopt.h> 
#include <sys/types.h> 

#include "mosquitto.h"

typedef struct {
    int is_alive; 
} heart_t; 

typedef struct {
    char hostname[128]; 
    uint16_t port; 
    char location[64];  
} start_arg_t; 

int main() { 
    struct mosquitto *mosq; 
    heart_t heart = {
        .is_alive = 1
    }; 

    char mqtt_channel_name[256] = "run"; 

    start_arg_t start_arg = {
        .hostname = "10.110.245.223", 
        .port = 1883, 
        .location = "location", 
    };  

    mosquitto_lib_init(); 
    mosq = mosquitto_new(NULL, true, NULL); 
    
    if (!mosq)
        printf("Failed to create mosq\n"); 

    if (mosquitto_connect(mosq, start_arg.hostname, start_arg.port, 60) != MOSQ_ERR_SUCCESS) {
        printf("Failed to connect to MQTT\n"); 
    } 

    while (1) {
        mosquitto_publish(mosq, NULL, mqtt_channel_name, sizeof(heart), &heart, 0, false); 
        sleep(1); 
    } 

    mosquitto_destroy(mosq); 
    mosquitto_lib_cleanup(); 
} 
