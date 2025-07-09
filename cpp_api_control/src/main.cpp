#include <iostream> 
#include <opencv2/core.hpp> 
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

  crow::SimpleApp app; 
  crow::SimpleApp(app, "/")([](){
    if (frame == NULL) 
      return cv::Mat{frame->size(), frame->type(), cv::Scalar{0,0,0}}

    is_success, numpy_arr = cv2.imencode(".jpg", frame)

    if (!is_success) 
      return cv::Mat{frame->size(), frame->type(), cv::Scalar{0,0,0}}

    return numpy_arr.to_bytes(); 
  }); 

  app.port(18080).multithreaded().run(); 

  cv::Videocapture cap(0); 

  if (!cap.isOpened()) {
    std::cout << "Error: Could not open Camera! \n"; 
    return -1; 
  }

  init_opencv(cap, frame); ; 

  return 0; 
}