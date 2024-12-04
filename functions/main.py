from firebase_functions import https_fn
from firebase_admin import initialize_app
from flask import jsonify
import flask
import requests

initialize_app()
app = flask.Flask(__name__)

@app.route("/heartbeat")
def get_heartbeat(): 
    try: 
        response = requests.get("http://10.110.194.54:5000/heartbeat", timeout=3)
    except requests.exceptions.Timeout: 
        return "Timeout, car is not alive"


    if response.status_code != 200: 
        return "Failed to get heartbeat"
    
    return jsonify(response.json())

@app.route("/camera/stream")
def is_video_stream_live(): 
    try: 
        response = requests.get("http://10.110.194.54:5002/camera/stream", timeout=5) 
    except request.exceptions.Timeout:
        return "Timeout, stream is not active"

    if response.status_code != 200: 
        return "Failed to get stream"
    
    return Response(response.content(), mimetype='multipart/x-mixed-replace; boundary=frame') 


@https_fn.on_request()
def httpsflaskexample(req: https_fn.Request) -> https_fn.Response:
    with app.request_context(req.environ):
        return app.full_dispatch_request()