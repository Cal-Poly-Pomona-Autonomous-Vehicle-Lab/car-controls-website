#include <iostream> 
#include <opencv2/core.hpp> 
#include <opencv2/opencv.hpp>
#include <thread> 
#include <csignal> 
#include <mutex>
#include <queue>
#include "crow.h"

void signal_handler(int signum) {
  std::cout << "Caught signal " << signum << ". Exiting gracefully. \n"; 
  exit(signum);
}

void manage_camera(cv::VideoCapture cap, 
  std::queue<std::string> &frames, std::mutex &mutex, bool &isLive) {

  signal(SIGINT, signal_handler); 
  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80;

  cv::Mat frame; 

  while ( cap.isOpened() && isLive ) {
    cap >> frame; 

    if ( frame.empty() ) {
      CROW_LOG_INFO << "Error: Unable to obtain frame \n"; 
      return; 
    } 
    
    bool is_success = cv::imencode(".jpg", frame, buff, param); 
    if (!is_success) {
      CROW_LOG_INFO << "Failed to encode image";
      return; 
    }
    
    std::string result(buff.begin(), buff.end());

    /* If frame queue is full, continue updating the queue with latest frames */
    mutex.lock();
    if (frames.size() >= 10) {
      frames.pop(); 
      frames.push( result );   
      CROW_LOG_INFO << "Waiting for update..."; 
    } else {
      frames.push( result ); 
      CROW_LOG_INFO << "Pushing Frame Into Queue"; 
    }
    mutex.unlock(); 

    
    if ( cv::waitKey(1) == 27 ) {
      break; 
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void send_frames(crow::websocket::connection& conn, 
  std::queue<std::string> &frames_queue, std::mutex& mutex, bool& isLive) {
  
  signal(SIGINT, signal_handler); 
  while (isLive) {
    if (frames_queue.empty()) continue; 

    mutex.lock(); 
    conn.send_binary(frames_queue.front()); 
    frames_queue.pop(); 
    mutex.unlock(); 

    CROW_LOG_INFO << "Sending image..."; 
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
  }

}

int main() {
  cv::Mat *frame = new cv::Mat();
  cv::VideoCapture cap(0);
  std::queue<std::string> frames_queue; 
  std::mutex mutex; 
  bool isLive = false; 

  CROW_LOG_INFO  << "Server is Starting...\n"; 

  crow::SimpleApp app; 

  CROW_WEBSOCKET_ROUTE(app, "/")
    .onopen([&](crow::websocket::connection& conn) {
      CROW_LOG_INFO << "Opening Socket";

      if (isLive) {
        CROW_LOG_INFO << "Livestream thread already exists";
        return; 
      }
        
      isLive = true; 
      std::thread camera_thread(manage_camera, cap, 
        std::ref(frames_queue), std::ref(mutex), std::ref(isLive)); 
      camera_thread.detach(); 

      std::thread queue_thread(send_frames, std::ref(conn), 
        std::ref(frames_queue), std::ref(mutex), std::ref(isLive));
      queue_thread.detach(); 
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason,
    uint16_t status_code) {
      CROW_LOG_INFO << "Closing socket"; 
      isLive = false; 
    })
    .onaccept([&](const crow::request& req, void **userdata) {
      return true;
    }) 
    .onerror([&](const crow::websocket::connection& conn, const std::string& error_message){
      CROW_LOG_INFO << error_message << "\n"; 
      isLive = false; 
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& message,
    bool is_binary) {
    });

  std::cout << "Server is starting\n"; 
  auto server = app.port(5002).run_async(); 

  std::cout << "Server finished init\n"; 

  return 0; 
}
