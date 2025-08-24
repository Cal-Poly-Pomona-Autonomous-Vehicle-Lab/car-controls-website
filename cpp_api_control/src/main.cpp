#include <iostream> 
#include <opencv2/core.hpp> 
#include <opencv2/opencv.hpp>
#include <thread> 
#include <csignal> 
#include "crow.h"

void signal_handler(int signum) {
  std::cout << "Caught signal " << signum << ". Exiting gracefully. \n"; 
  exit(signum);
}

void init_opencv(cv::Mat *frame, cv::VideoCapture cap) {

  signal(SIGINT, signal_handler); 

  while (true) {
    if (cap.isOpened())
      cap >> *frame; 
    else
      *frame = cv::Mat(320, 240, CV_8UC3, cv::Scalar(0, 0, 0));

    if ( frame == NULL || frame->empty() ) {
      std::cout << "Error: unable to obtain frame! \n"; 
      return; 
    }
    
    if ( cv::waitKey(1) == 27 ) {
      break; 
    }
  }

  std::cout << "Thread is finished\n"; 
}

void init_opencv_onopen(cv::Mat *frame, cv::VideoCapture cap, 
  crow::websocket::connection& conn, bool isLive) {
  signal(SIGINT, signal_handler); 

  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 

  while (cap.isOpened() && isLive) {
    cap >> *frame; 

    if ( frame == NULL ) {
      CROW_LOG_INFO << "Error: Unable to obtain frame \n"; 
      return; 
    } 

    if ( frame->empty() ) {
      CROW_LOG_INFO << "Frame Empty, sending empty image"; 
      break; 
    }
    
    bool is_success = cv::imencode(".jpg", *frame, buff, param); 
    if (!is_success) {
      conn.send_text("Failed to encode to jpg"); 
      return; 
    }
    
    std::string result(buff.begin(), buff.end());

    CROW_LOG_INFO << "Sending frame...";
    

    conn.send_binary(result);
    if ( cv::waitKey(1) == 27 ) {
      break; 
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  CROW_LOG_INFO << "Thread has completed\n"; 
}

int main() {
  cv::Mat *frame = new cv::Mat();
  cv::VideoCapture cap(0);
  bool isLive = false; 

  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 
  /* TODO: Implement a queue to manage the frames */

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80;

  std::cout << "Starting\n"; 

  // TODO: Adding queue to live streaming

  crow::SimpleApp app; 

  CROW_WEBSOCKET_ROUTE(app, "/")
    .onopen([&](crow::websocket::connection& conn) {
      CROW_LOG_INFO << "Opening Socket";

      if (isLive) {
        CROW_LOG_INFO << "Livestream thread already exists";
        return; 
      }
        
      isLive = true; 
      std::thread livestream_thread(init_opencv_onopen, frame, cap, std::ref(conn), isLive); 
      livestream_thread.detach(); 
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason,
    uint16_t status_code) {
      CROW_LOG_INFO << "Closing socket"; 

      // TODO: Fix the segmentation fault
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
