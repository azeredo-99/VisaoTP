#ifndef VC_H
#define VC_H

#include <opencv2/opencv.hpp>

// Estrutura para armazenar informa��es das resist�ncias
typedef struct {
    cv::Rect boundingBox;
    cv::Point centroid;
    int value; // Valor da resist�ncia em ohms
} Resistor;

// Fun��o para segmentar a imagem
cv::Mat segmentImage(const cv::Mat& frame);

// Fun��o para identificar resist�ncias na imagem segmentada
std::vector<Resistor> identifyResistors(const cv::Mat& segmentedFrame);

// Fun��o para desenhar informa��es das resist�ncias no frame original
void drawResistorInfo(cv::Mat& frame, const std::vector<Resistor>& resistors);

#endif // VC_H
