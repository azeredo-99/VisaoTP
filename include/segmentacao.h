#ifndef SEGMENTACAO_H
#define SEGMENTACAO_H

#include <opencv2/opencv.hpp>

void segmentarResistencias(const cv::Mat& frame, cv::Mat& mask);

#endif
