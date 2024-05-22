#include "vc.h"
#include "segmentacao.h"
#include "detecao.h"
#include <iostream>

void vc_timer(void) {
    static bool running = false;
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

    if (!running) {
        running = true;
    }
    else {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Tempo em segundos.
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}

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

void detectarContornos(const cv::Mat& mask, cv::Mat& output, std::vector<cv::Rect>& boxes) {
    std::vector<std::vector<cv::Point>> contornos;
    cv::findContours(mask, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    boxes.clear();

    for (size_t i = 0; i < contornos.size(); i++) {
        cv::Rect box = cv::boundingRect(contornos[i]);
        boxes.push_back(box);
        cv::rectangle(output, box, cv::Scalar(0, 255, 0), 2);
    }
}

std::string identificarValorResistencia(const cv::Mat& frame, const cv::Rect& box) {
    // Placeholder: substitua por uma função real que identifica o valor da resistência
    return "Valor";
}

void desenharInformacao(cv::Mat& frame, const std::vector<cv::Rect>& boxes, const std::vector<std::string>& valores) {
    for (size_t i = 0; i < boxes.size(); ++i) {
        cv::rectangle(frame, boxes[i], cv::Scalar(255, 0, 0), 2);
        cv::putText(frame, valores[i], boxes[i].tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    }
}
