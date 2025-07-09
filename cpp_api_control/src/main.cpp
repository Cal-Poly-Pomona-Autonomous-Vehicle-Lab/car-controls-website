#include <iostream> 
#include <opencv2/core.hpp> 
#include <opencv2/opencv.hpp>
#include "crow.h"

void init_opencv(cv::Videocapture cap, cv::Mat *frame) {
  while (true) {
    cap >> frame; 

    if (frame.empty()) {
      std::cout << "Error: unable to obtain frame! \n"; 
      return; 
    }

    if (cv::waitKey(1) == 27) {
      break; 
    }
  }
}

int main() {
  cv::Mat *frame; 

  std::vector<uchar> buff; 
  std::vector<int> param(2); 

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80;//default(95) 0-100

  crow::SimpleApp app; 
  CROW_ROUTE(app, "/")([](){
    if (frame == NULL) 
      return cv::Mat::zeros(400, 400, CV_8UC3);

    bool is_success = cv::imencode(".jpg", frame, buff, param);

    if (!is_success) 
      return cv::Mat::zeros(400, 400, CV_8UC3);

    return buff; 
  }); 

  app.port(18080).multithreaded().run(); 

  cv::VideoCapture cap(0);

  if (!cap.isOpened()) {
    std::cout << "Error: Could not open Camera! \n"; 
    return -1; 
  }

  init_opencv(cap, frame); ; 

  return 0; 
}