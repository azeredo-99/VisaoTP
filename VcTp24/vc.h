#define VC_DEBUG
#define _CRT_SECURE_NO_WARNINGS

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//							 MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MAX(a,b) (a>b? a:b)
#define MIN(a,b) (a<b? a:b)
#define MAX3(a,b,c) (a>b? (a>c?a:c):(b>c?b:c))
#define MIN3(a,b,c) (a<b? (a<c?a:c):(b<c?b:c))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Per�metro
	int label;					// Etiqueta
} OVC;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);

int vc_bgr_to_rgb(IVC* src);
int vc_rgb_to_bgr(IVC* src);

int vc_convert_to_three_channels(IVC* src, IVC* dst);

int vc_rgb_to_gray(IVC* src, IVC* dst);

int vc_rgb_to_hsv(IVC* srcdst, int tipo);
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

int vc_scale_gray_to_rgb(IVC* src, IVC* dst);

int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);

int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin);

int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_open(IVC* src, IVC* dst, int kernel);
int vc_binary_close(IVC* src, IVC* dst, int kernel);

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);