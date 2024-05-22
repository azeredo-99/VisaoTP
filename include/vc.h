#ifndef VC_H
#define VC_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <chrono>

void segmentarResistencias(const cv::Mat& frame, cv::Mat& mask);
void detectarContornos(const cv::Mat& mask, cv::Mat& output, std::vector<cv::Rect>& boxes);
std::string identificarValorResistencia(const cv::Mat& frame, const cv::Rect& box);
void desenharInformacao(cv::Mat& frame, const std::vector<cv::Rect>& boxes, const std::vector<std::string>& valores);
void vc_timer(void);

#endif
