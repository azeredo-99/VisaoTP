#ifndef DETECAO_H
#define DETECAO_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

void detectarContornos(const cv::Mat& mask, cv::Mat& output, std::vector<cv::Rect>& boxes);
std::string identificarValorResistencia(const cv::Mat& frame, const cv::Rect& box);
void desenharInformacao(cv::Mat& frame, const std::vector<cv::Rect>& boxes, const std::vector<std::string>& valores);

#endif
