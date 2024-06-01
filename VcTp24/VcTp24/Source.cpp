#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
extern "C" {
#include "vc.h"
}

using namespace cv;
using namespace std;

// Estrutura para armazenar informações de uma cor
struct ColorRange {
    Scalar lower;
    Scalar upper;
    string name;
};

// Função para identificar a cor de uma faixa
string identificarCor(const Vec3b& color) {
    // Definir intervalos de cores HSV para cada faixa
    vector<ColorRange> colorRanges = {
        {Scalar(0, 70, 50), Scalar(10, 255, 255), "Vermelho"},
        {Scalar(10, 100, 100), Scalar(25, 255, 255), "Laranja"},
        {Scalar(25, 100, 100), Scalar(35, 255, 255), "Amarelo"},
        {Scalar(35, 50, 50), Scalar(85, 255, 255), "Verde"},
        {Scalar(85, 50, 50), Scalar(130, 255, 255), "Azul"},
        {Scalar(130, 50, 50), Scalar(160, 255, 255), "Violeta"},
        {Scalar(0, 0, 200), Scalar(180, 20, 255), "Branco"},
        {Scalar(0, 0, 0), Scalar(180, 255, 30), "Preto"},
        {Scalar(0, 0, 50), Scalar(180, 50, 200), "Cinza"},
        {Scalar(0, 50, 50), Scalar(10, 100, 200), "Marrom"},
        {Scalar(20, 50, 50), Scalar(30, 100, 200), "Ouro"},
        {Scalar(0, 0, 150), Scalar(20, 100, 200), "Prata"}
    };

    Mat colorMat(1, 1, CV_8UC3, color);
    Mat hsvMat;
    cvtColor(colorMat, hsvMat, COLOR_BGR2HSV);
    Vec3b hsvColor = hsvMat.at<Vec3b>(0, 0);

    for (const auto& range : colorRanges) {
        if (hsvColor[0] >= range.lower[0] && hsvColor[0] <= range.upper[0] &&
            hsvColor[1] >= range.lower[1] && hsvColor[1] <= range.upper[1] &&
            hsvColor[2] >= range.lower[2] && hsvColor[2] <= range.upper[2]) {
            return range.name;
        }
    }

    return "Desconhecido";
}

// Função para identificar o valor da resistência a partir das cores das faixas
string identificarResistencia(const Mat& roi) {
    // Converter ROI para o espaço de cor HSV
    Mat hsv;
    cvtColor(roi, hsv, COLOR_BGR2HSV);

    // Dividir a ROI em três partes para identificar as três faixas de cores
    int width = roi.cols / 3;
    Vec3b color1 = hsv.at<Vec3b>(roi.rows / 2, width / 2);
    Vec3b color2 = hsv.at<Vec3b>(roi.rows / 2, width + width / 2);
    Vec3b color3 = hsv.at<Vec3b>(roi.rows / 2, 2 * width + width / 2);

    // Identificar cores das faixas
    string faixa1 = identificarCor(color1);
    string faixa2 = identificarCor(color2);
    string faixa3 = identificarCor(color3);

    // Mostrar as cores identificadas na imagem
    cout << "Faixa 1: " << faixa1 << ", Faixa 2: " << faixa2 << ", Faixa 3: " << faixa3 << endl;

    // Converter cores para valores
    map<string, int> colorToValue = {
        {"Preto", 0}, {"Marrom", 1}, {"Vermelho", 2}, {"Laranja", 3},
        {"Amarelo", 4}, {"Verde", 5}, {"Azul", 6}, {"Violeta", 7},
        {"Cinza", 8}, {"Branco", 9}
    };

    if (colorToValue.find(faixa1) == colorToValue.end() ||
        colorToValue.find(faixa2) == colorToValue.end() ||
        colorToValue.find(faixa3) == colorToValue.end()) {
        return "Desconhecido";
    }

    int digit1 = colorToValue[faixa1];
    int digit2 = colorToValue[faixa2];
    int multiplier = pow(10, colorToValue[faixa3]);

    int resistanceValue = (digit1 * 10 + digit2) * multiplier;

    return to_string(resistanceValue) + " Ohms";
}

int main() {
    string videoPath = "C:/Users/Asus/OneDrive/Documents/2ªAno-Lesi/VC/VcTp24/video_resistors.mp4";  // Atualizar o caminho do vídeo conforme necessário

    VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir o vídeo!" << endl;
        return -1;
    }

    Mat frame, hsv, mask;
    IVC* image = nullptr;
    IVC* imageBlobs = nullptr;
    int nblobs = 0;
    OVC* blobs = nullptr;

    while (cap.read(frame)) {
        if (frame.empty()) {
            cerr << "Erro ao ler o frame!" << endl;
            break;
        }

        // Converter o frame de BGR para RGB
        cvtColor(frame, frame, COLOR_BGR2RGB);

        // Inicializar IVC image
        if (!image) {
            image = vc_image_new(frame.cols, frame.rows, 3, 255);
            imageBlobs = vc_image_new(frame.cols, frame.rows, 1, 255);
        }
        memcpy(image->data, frame.data, frame.cols * frame.rows * 3);

        // Segmentação de cores
        vc_hsv_segmentation(image, imageBlobs, 0, 180, 50, 255, 50, 255);

        // Abertura (remoção de ruído)
        vc_binary_open(imageBlobs, imageBlobs, 5);

        // Etiquetagem de blobs
        blobs = vc_binary_blob_labelling(imageBlobs, imageBlobs, &nblobs);

        // Processar blobs
        for (int i = 0; i < nblobs; i++) {
            if (blobs[i].area > 500) { // Ajuste o valor da área conforme necessário
                Rect boundingBox(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
                Mat roi = frame(boundingBox);

                // Identificar valor da resistência
                string valorResistencia = identificarResistencia(roi);

                // Desenhar a bounding box e o valor da resistência
                rectangle(frame, boundingBox, Scalar(255, 0, 0), 2);
                putText(frame, valorResistencia, Point(blobs[i].x, blobs[i].y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
            }
        }

        // Converter de volta para BGR para exibição
        cvtColor(frame, frame, COLOR_RGB2BGR);

        // Exibir o frame processado
        imshow("Frame", frame);

        // Pressionar 'q' para sair
        if (waitKey(30) == 'q') break;
    }

    cap.release();
    destroyAllWindows();

    if (image) vc_image_free(image);
    if (imageBlobs) vc_image_free(imageBlobs);
    if (blobs) free(blobs);

    return 0;
}
