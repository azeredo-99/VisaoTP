#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>

extern "C" {
#include "vc.h"
}

// Timer 
void vc_timer(void) {
    static bool running = false;
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

    if (!running) {
        running = true;
    }
    else {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Time in seconds
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}

// Identificar Cor
std::string identificarCor(int h, int s, int v) {
    if ((h >= 0 && h <= 10) || (h >= 160 && h <= 180) && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Vermelho";
    if (h >= 11 && h <= 20 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Castanho";
    if (h >= 21 && h <= 30 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Laranja";
    if (h >= 31 && h <= 45 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Amarelo";
    if (h >= 46 && h <= 85 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Verde";
    if (h >= 86 && h <= 130 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Azul";
    if (h >= 131 && h <= 160 && s >= 100 && s <= 255 && v >= 100 && v <= 255) return "Violeta";
    if (v >= 200 && s <= 50) return "Branco";
    if (v <= 50 && s <= 50) return "Preto";
    if (v <= 200 && s <= 50) return "Cinza";
    if (h >= 20 && h <= 30 && s >= 50 && s <= 200 && v >= 200 && v <= 255) return "Ouro";
    if (h >= 30 && h <= 40 && s >= 50 && s <= 200 && v >= 200 && v <= 255) return "Prata";

    return "Desconhecido";
}

// segmentar cinze e aplicar threshold
void segmentarParaCinza(IVC* src, IVC* dst) {

    vc_rgb_to_gray(src, dst);


    vc_gray_to_binary(dst, dst, 100);


    vc_binary_close(dst, dst, 7);
    vc_binary_open(dst, dst, 7);
}

// analisar imagens e outra feautures
OVC* analyzeImage(IVC* src, IVC* dst, int* numBlobs) {
    OVC* blobs = vc_binary_blob_labelling(dst, src, numBlobs);
    vc_binary_blob_info(src, blobs, *numBlobs);
    return blobs;
}

// Função para calcular cores
int calcularResistencia(const std::vector<std::string>& cores) {
    std::map<std::string, int> valorCores = {
        {"Preto", 0}, {"Castanho", 1}, {"Vermelho", 2}, {"Laranja", 3}, {"Amarelo", 4},
        {"Verde", 5}, {"Azul", 6}, {"Violeta", 7}, {"Cinza", 8}, {"Branco", 9}
    };

    if (cores.size() >= 3) {
        int valor1 = valorCores[cores[0]];
        int valor2 = valorCores[cores[1]];
        int multiplicador = pow(10, valorCores[cores[2]]);
        return (valor1 * 10 + valor2) * multiplicador;
    }
    return -1;
}

int main(void) {
    // Video
    char videofile[20] = "video_resistors.mp4";
    cv::VideoCapture capture;
    struct {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;

    std::string str;
    int key = 0;

    // ler ficheiro do video
    capture.open(videofile);

    // verificação se abriu
    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
        return 1;
    }

    // Total de frames
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    // frames do video
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    // resolucao
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    // window display
    cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

    // começar timer
    vc_timer();

    cv::Mat frame, hsvFrame, mask;
    IVC* image, * imageBinary;
    int numBlobs;
    OVC* blobs;

    while (key != 'q') {
        // ler frames
        capture.read(frame);

        if (frame.empty()) break;

        // numero de frames a processar
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        // Example of inserting text into the frame
        str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        // frame para ivc
        image = vc_image_new(video.width, video.height, 3, 255);
        memcpy(image->data, frame.data, video.width * video.height * 3);

        // frame para hsv
        cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

        // iniciar mascara binaria
        mask = cv::Mat::zeros(frame.size(), CV_8UC1);

        // alacance das cores para binario
        std::vector<cv::Scalar> lower_bounds = { cv::Scalar(0, 100, 100), cv::Scalar(160, 100, 100) };
        std::vector<cv::Scalar> upper_bounds = { cv::Scalar(10, 255, 255), cv::Scalar(180, 255, 255) };

        for (size_t i = 0; i < lower_bounds.size(); i++) {
            cv::Mat temp_mask;
            cv::inRange(hsvFrame, lower_bounds[i], upper_bounds[i], temp_mask);
            mask = mask | temp_mask;
        }

        // binario para ivc
        imageBinary = vc_image_new(video.width, video.height, 1, 255);
        memcpy(imageBinary->data, mask.data, video.width * video.height);


        blobs = analyzeImage(imageBinary, imageBinary, &numBlobs);


        for (int i = 0; i < numBlobs; i++) {
            cv::Rect boundingBox(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
            cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);

            std::vector<std::string> cores;
            for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++) {
                for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++) {
                    int h = hsvFrame.at<cv::Vec3b>(y, x)[0];
                    int s = hsvFrame.at<cv::Vec3b>(y, x)[1];
                    int v = hsvFrame.at<cv::Vec3b>(y, x)[2];
                    std::string cor = identificarCor(h, s, v);
                    if (cor != "Desconhecido") {
                        cores.push_back(cor);
                    }
                }
            }


            std::sort(cores.begin(), cores.end());
            cores.erase(std::unique(cores.begin(), cores.end()), cores.end());

            int valorResistor = calcularResistencia(cores);
            if (valorResistor != -1) {
                str = "Valor: " + std::to_string(valorResistor) + " ohms";
                cv::putText(frame, str, cv::Point(blobs[i].x, blobs[i].y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
            }

            std::cout << "Resistencia " << i + 1 << ": ";
            for (const std::string& cor : cores) {
                std::cout << cor << " ";
            }
            std::cout << std::endl;
        }


        cv::imshow("VC - VIDEO", frame);


        key = cv::waitKey(30);
    }


    vc_timer();


    capture.release();


    cv::destroyAllWindows();

    return 0;
}
