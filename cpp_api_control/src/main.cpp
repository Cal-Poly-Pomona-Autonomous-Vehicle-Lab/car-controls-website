#include <iostream> 
#include <opencv2/core.hpp> 
#include <opencv2/opencv.hpp>
#include "crow.h"

void init_opencv(cv::VideoCapture &cap, cv::Mat *frame) {
  while (cap.isOpened()) {
    cap >> *frame; 

    if (frame == NULL || frame.empty()) {
      std::cout << "Error: unable to obtain frame! \n"; 
      return; 
    }

    if (cv::waitKey(1) == 27) {
      break; 
    }
  }
}

int main() {
  cv::Mat *frame = new cv::Mat();

  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80

  crow::SimpleApp app; 

  CROW_WEBSOCKET_ROUTE(app, "/ws")
    .on_open([&](crow::websocket::connection& conn) {
    })
    .on_close([&](crow::websocket::connection& conn, const std::string& reason
    uint16_t status_code) {
      cap.release(); 
    })
    .on_message([&](crow::websocket::connection& conn, const std::string& message,
    bool is_binary) {
      if (frame == NULL)
        return; 

      bool is_sucess = cv::imencode(".jpg", *frame, buff, param); 
      
      if (!is_success)
        return result; 

        for (uchar c: buff)
          result.push_back(c); 
      
      return buff;
    })

  app.port(18080).multithreaded().run_async(); 

  cv::VideoCapture cap(0);

  if (!cap.isOpened()) {
    std::cout << "Error: Could not open Camera! \n"; 
    return -1; 
  }

  init_opencv(cap, frame); ; 

  return 0; 
}