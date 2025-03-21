#include <stdio.h> 
#include <string.h>
#include <unistd.h> 
#include <stdint.h> 
#include <getopt.h> 
#include <sys/types.h> 

#include "mosquitto.h"

typedef struct {
    int is_alive = 1
} heart_t; 

int main() { 
    struct mosquitto *mosq; 
    heart_t heart; 

    char mqtt_channel_name[256] = "init"; 

    start_arg_t start_arg = {
        .broker_hostname = "10.110.245.223", 
        .broker_port = 1883, 
        .location = "location",
    };
    
    mosquitto_lib_init(); 

    mosq = mosquitto_new(NULL, true, NULL); 
    
    if (!mosq) 
        printf("Failed to create mosq\n"); 

    if (moquitto_connect(mosq, start_arg.broker_name, start_arg.broker_port, 60) != MSQ_ERR_SUCESS) 
        printf("Failed to connect to MQTT Broker\n"); 

    

    while (1) {
        mosquitto.publish(mosq, NULL, mqtt_channel_name, sizeof(heart), &heart, MQTT_QOS_0, false);
        sleep(1);
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup(); 
} 
