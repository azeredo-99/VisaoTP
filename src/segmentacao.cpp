#include "segmentacao.h"

void segmentarResistencias(const cv::Mat& frame, cv::Mat& mask) {
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Defina os intervalos de cores para as faixas de resistências (exemplo para faixa vermelha)
    cv::Scalar lower_red = cv::Scalar(0, 70, 50);
    cv::Scalar upper_red = cv::Scalar(10, 255, 255);
    cv::inRange(hsv, lower_red, upper_red, mask);

    // Remova ruído
    cv::erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);
    cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);
}
