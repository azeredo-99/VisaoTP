#include "detecao.h"

void detectarContornos(const cv::Mat& mask, cv::Mat& output, std::vector<cv::Rect>& boxes) {
    std::vector<std::vector<cv::Point>> contornos;
    cv::findContours(mask, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    boxes.clear();

    for (size_t i = 0; i < contornos.size(); i++) {
        cv::Rect box = cv::boundingRect(contornos[i]);
        boxes.push_back(box);
        cv::rectangle(output, box, cv::Scalar(0, 255, 0), 2);

        // Calcule o centroide
        cv::Moments m = cv::moments(contornos[i], false);
        cv::Point center(m.m10 / m.m00, m.m01 / m.m00);
        cv::circle(output, center, 5, cv::Scalar(255, 0, 0), -1);
    }
}

std::string identificarValorResistencia(const cv::Mat& frame, const cv::Rect& box) {
    // Recorte a região da resistência
    cv::Mat resistencia = frame(box);

    // Identifique as cores das faixas
    // (Exemplo simplificado; ajuste para seu caso específico)
    cv::Scalar mean_color = cv::mean(resistencia);

    // Converta as cores para valores de resistência
    // Utilize um mapeamento das cores para valores de resistência

    return "Valor da resistência em ohms"; // Ajuste conforme necessário
}

void desenharInformacao(cv::Mat& frame, const std::vector<cv::Rect>& boxes, const std::vector<std::string>& valores) {
    for (size_t i = 0; i < boxes.size(); i++) {
        cv::rectangle(frame, boxes[i], cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, valores[i], boxes[i].tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
    }
}
