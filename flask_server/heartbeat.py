from flask import Flask, jsonify
from flask_cors import CORS
from datetime import datetime, timedelta 
import json
import time 
import threading 

app = Flask(__name__, static_url_path='/static') 
cors = CORS(app, resources={r"/*": {"origins": "*"}})

vehicle_status = {"last_heartbeat": None, "is_alive": False} 
HEARTBEAT_TIMEOUT = 5


def monitor_heartbeat(): 
    while True: 
        if vehicle_status["last_heartbeat"] != None: 
            time_since_last_heartbeat = datetime.now() - vehicle_status["last_heartbeat"]
            vehicle_status["is_alive"] = time_since_last_heartbeat < timedelta(seconds=HEARTBEAT_TIMEOUT) 
            vehicle_status["last_heartbeat"] = datetime.now() 
        else: 
            vehicle_status["last_heartbeat"] = datetime.now()  
            vehicle_status["is_alive"] = True
        threading.Event().wait(1) 
    


@app.route('/heartbeat') 
def heartbeat_check():
    return jsonify({
        "is_alive": vehicle_status["is_alive"],
        "last_heartbeat": vehicle_status["last_heartbeat"] 
    }) 


if __name__ == '__main__':
    heartbeat_thread = threading.Thread(target=monitor_heartbeat, daemon=True)
    heartbeat_thread.start() 

    app.run(debug=True, host="10.110.194.54")
