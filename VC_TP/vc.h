#ifndef VC_H
#define VC_H

#include <opencv2/opencv.hpp>

// Estrutura para armazenar informações das resistências
typedef struct {
    cv::Rect boundingBox;
    cv::Point centroid;
    int value; // Valor da resistência em ohms
} Resistor;

// Função para segmentar a imagem
cv::Mat segmentImage(const cv::Mat& frame);

// Função para identificar resistências na imagem segmentada
std::vector<Resistor> identifyResistors(const cv::Mat& segmentedFrame);

// Função para desenhar informações das resistências no frame original
void drawResistorInfo(cv::Mat& frame, const std::vector<Resistor>& resistors);

#endif // VC_H
