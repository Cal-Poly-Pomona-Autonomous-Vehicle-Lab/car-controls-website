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

void init_opencv(cv::Mat *frame) {
  cv::VideoCapture cap(0); 

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

int main() {
  cv::Mat *frame = new cv::Mat();

  std::vector<uchar> buff(200 * 1024 * 1024); 
  std::vector<int> param(2); 
  /* TODO: Implement a queue to manage the frames */

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = 80;

  crow::SimpleApp app; 

  CROW_WEBSOCKET_ROUTE(app, "/")
    .max_payload(200 * 1024 * 1024)
    .onopen([&](crow::websocket::connection& conn) {
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason,
    uint16_t status_code) {
    })
    .onaccept([&](const crow::request& req, void **userdata){
      return true 
    }) 
    .onmessage([&](crow::websocket::connection& conn, const std::string& message,
    bool is_binary) {
      std::string result;

      if (frame == NULL) {
        conn.send_text("NULL Frame");
        return; 
      } else if (frame->empty()) {
        conn.send_text("Empty Frame");
        return; 
      }

      bool is_success = cv::imencode(".jpg", *frame, buff, param); 
      
      if (!is_success) {
        conn.send_text("Failed to encode to jpg"); 
        return; 
      }

      result.clear(); 
      result = buff; 
      // for (uchar c: buff)
      //   result.push_back(c); 
      conn.send_binary(result);
    });

  auto server = app.port(18080).multithreaded().run_async(); 

  std::cout << "Server finished init\n"; 

  std::cout << "Thread is starting\n";
  std::thread worker(init_opencv, std::ref(frame)); 
  worker.detach(); 

  return 0; 
}
