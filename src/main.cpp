#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "vc.h"

int main(void) {
    // V�deo
    char videofile[20] = "video/video_resistors.mp4";
    cv::VideoCapture capture;
    struct
    {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;
    // Outros
    std::string str;
    int key = 0;

    // Leitura de v�deo de um ficheiro
    capture.open(videofile);

    // Verifica se foi poss�vel abrir o ficheiro de v�deo
    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
        return 1;
    }

    // N�mero total de frames no v�deo
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    // Frame rate do v�deo
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    // Resolu��o do v�deo
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Cria uma janela para exibir o v�deo
    cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

    // Inicia o timer
    vc_timer();

    cv::Mat frame, mask, output;
    std::vector<cv::Rect> boxes;
    std::vector<std::string> valores;

    while (key != 'q') {
        // Leitura de uma frame do v�deo
        capture.read(frame);

        // Verifica se conseguiu ler a frame
        if (frame.empty()) break;

        // N�mero da frame a processar
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        // Exemplo de inser��o texto na frame
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

        // Processamento do v�deo
        segmentarResistencias(frame, mask);
        frame.copyTo(output);
        detectarContornos(mask, output, boxes);

        valores.clear();
        for (const cv::Rect& box : boxes) {
            valores.push_back(identificarValorResistencia(frame, box));
        }

        desenharInformacao(output, boxes, valores);

        // Exibe a frame
        cv::imshow("VC - VIDEO", output);

        // Sai da aplica��o, se o utilizador premir a tecla 'q'
        key = cv::waitKey(1);
    }

    // Para o timer e exibe o tempo decorrido
    vc_timer();

    // Fecha a janela
    cv::destroyWindow("VC - VIDEO");

    // Fecha o ficheiro de v�deo
    capture.release();

    return 0;
}
