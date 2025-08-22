# cam_server.py
# Stream webcam frames as JPEG over WebSocket
# Requires: pip install websockets opencv-python

import asyncio
import cv2
import websockets

HOST = "0.0.0.0"
PORT = 5002 
FPS = 10  # target frames per second

async def stream_camera(ws):
    cap = cv2.VideoCapture(0)  # 0 = default front camera (on laptops usually built-in)
    if not cap.isOpened():
        await ws.close(code=1011, reason="Camera not available")
        return

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            # Encode frame as JPEG
            success, buffer = cv2.imencode(".jpg", frame)
            if not success:
                continue

            await ws.send(buffer.tobytes())
            await asyncio.sleep(1 / FPS)
    except websockets.ConnectionClosed:
        pass
    finally:
        cap.release()

async def main():
    async with websockets.serve(stream_camera, HOST, PORT, max_size=None):
        print(f"Serving webcam at ws://{HOST}:{PORT}")
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    asyncio.run(main())
