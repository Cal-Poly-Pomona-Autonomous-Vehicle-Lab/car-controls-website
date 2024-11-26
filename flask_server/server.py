from flask import Flask, render_template, Response, request, jsonify 
from flask_cors import CORS
from aiortc import RTCPeerConnection, RTCSessionDescription 
from datetime import datetime, timedelta
import ssl
import threading
import cv2 
import json 
import uuid
import asyncio
import logging 
import time 

app = Flask(__name__, static_url_path='/static') 
cors = CORS(app, resources={r"/*": {"origins": "*"}})

context = ssl.SSLContext() 
context.load_cert_chain('cert.pem','key.pem')

vehicle_status = {"last_heartbeat": None, "is_alive": False}
HEARTBEAT_TIMEOUT = 5

def generate_frames():
    camera = cv2.VideoCapture(0) 
    
    while True: 
        start_time = time.time()
        success, frame = camera.read()
        if not success: 
            break
            

        ret, buffer = cv2.imencode('.jpg', frame) 
        frame = buffer.tobytes() 
        yield(b'--frame\r\n'
            b'Content-type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        elasped_time = time.time() - start_time
        

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
    
 
@app.route('/')
def video_feed(): 
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame') 

@app.route('/heartbeat') 
def heartbeat_check():
    return jsonify({
        "is_alive": vehicle_status["is_alive"],
        "last_heartbeat": vehicle_status["last_heartbeat"] 
    }) 
     

if __name__ == '__main__':
    heartbeat_thread = threading.Thread(target=monitor_heartbeat, daemon=True)
    heartbeat_thread.start() 

    app.run(debug=True, ssl_context=context)
