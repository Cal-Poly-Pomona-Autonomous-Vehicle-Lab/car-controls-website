#ifndef SERVER_H 
#define SERVER_H

#include <iostream> 
#include "crow.h"


void init_server() {
    crow::SimpleApp app; 
    
    CROW_ROUTE(app,  "/video")([] () {
        return "Hello"; 
    }); 

    server_thread = app.bindaddr("")
     .port(8080)
     .multithreaded()
     .run_async(); 

    return server_thread; 
} 

#endif
