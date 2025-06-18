#include <iostream> 
#include "crow.h"


int init_server() {
    crow::SimpleApp app; 
    
    CROW_ROUTE(app,  "/video")([] () {
        return "Hello"; 
    }); 

    app.bindaddr("fc94:c600:912d:e4ee:988f:4420:a4d6:34ac")
     .port(8080)
     .multithreaded()
     .run(); 
} 
