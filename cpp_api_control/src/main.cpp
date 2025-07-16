#include <iostream> 
#include <opencv2/core.hpp> 
#include <opencv2/opencv.hpp>
#include "crow.h"

void init_opencv(cv::VideoCapture &cap, cv::Mat *frame) {
  while (cap.isOpened()) {
    cap >> *frame; 

    if ( frame == NULL || frame->empty() ) {
      std::cout << "Error: unable to obtain frame! \n"; 
      return; 
    }

    if ( cv::waitKey(1) == 27 ) {
      break; 
    }
  }
}

int main() {
  cv::Mat *frame = new cv::Mat();

  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80;

  crow::SimpleApp app; 

  CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn) {
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason,
    uint16_t status_code) {
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& message,
    bool is_binary) {
      std::string result = "error";

      if (frame == NULL)
        conn.send_text(result);
        return; 
      else if (frame->empty())
        conn.send_text(result);
        return; 

      bool is_success = cv::imencode(".jpg", *frame, buff, param); 
      
      if (!is_success)
        conn.send_text(result); 
        return; 

      result.clear(); 
      for (uchar c: buff)
        result.push_back(c); 
      
      conn.send_text(result);
    })

  app.port(18080).multithreaded().run_async(); 

  std::cout << "Server finished init\n"; 
  
  cv::VideoCapture cap(0);

  if (!cap.isOpened()) {
    std::cout << "Error: Could not open Camera! \n"; 
    return -1; 
  }

  init_opencv(cap, frame); ; 

  return 0; 
}