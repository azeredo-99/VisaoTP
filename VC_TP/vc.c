#include "vc.h"

// Função para segmentar a imagem
cv::Mat segmentImage(const cv::Mat& frame) {
    cv::Mat hsvFrame, segmentedFrame;
    cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

    // Define os limites para a segmentação por cor
    cv::Scalar lowerBound(0, 70, 50); // Ajustar conforme necessário
    cv::Scalar upperBound(10, 255, 255); // Ajustar conforme necessário
    cv::inRange(hsvFrame, lowerBound, upperBound, segmentedFrame);

    // Aplicar filtros para remover ruído
    cv::medianBlur(segmentedFrame, segmentedFrame, 5);
    return segmentedFrame;
}

// Função auxiliar para calcular o valor da resistência com base nas faixas de cor
int calculateResistorValue(const std::vector<cv::Vec3b>& bands) {
    // Implementar a lógica para calcular o valor com base nas faixas de cor
    // Placeholder para o valor calculado
    return 0;
}

// Função para identificar resistências na imagem segmentada
std::vector<Resistor> identifyResistors(const cv::Mat& segmentedFrame) {
    std::vector<Resistor> resistors;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(segmentedFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > 100) { // Filtrar áreas pequenas
            cv::Rect boundingBox = cv::boundingRect(contour);
            cv::Point centroid(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);

            // Placeholder para a identificação das faixas de cor
            std::vector<cv::Vec3b> bands; // Substituir com a lógica real
            int value = calculateResistorValue(bands);

            resistors.push_back({ boundingBox, centroid, value });
        }
    }

    return resistors;
}

// Função para desenhar informações das resistências no frame original
void drawResistorInfo(cv::Mat& frame, const std::vector<Resistor>& resistors) {
    for (const auto& resistor : resistors) {
        cv::rectangle(frame, resistor.boundingBox, cv::Scalar(0, 255, 0), 2);
        cv::circle(frame, resistor.centroid, 5, cv::Scalar(0, 0, 255), -1);
        cv::putText(frame, std::to_string(resistor.value) + " ohms", resistor.boundingBox.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
    }
}
