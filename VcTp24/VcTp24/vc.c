#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}


int vc_gray_negative(IVC* srcdst) {
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channel = srcdst->channels;
	int x, y;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))return 0;
	if (channel != 1)return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channel;

			data[pos] = 255 - data[pos];
		}
	}

	return 1;
}



int vc_rgb_negative(IVC* srcdst) {
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channel = srcdst->channels;
	int x, y;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))return 0;
	if (channel != 3)return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channel;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return 1;
}

int vc_bgr_to_rgb(IVC* src) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src;
	unsigned char r, g, b;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (src->channels != 3) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;

			b = datasrc[pos_src];
			g = datasrc[pos_src + 1];
			r = datasrc[pos_src + 2];

			datasrc[pos_src] = r;
			datasrc[pos_src + 1] = g;
			datasrc[pos_src + 2] = b;
		}
	}
	return 1;
}

int vc_rgb_to_bgr(IVC* src) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src;
	unsigned char r, g, b;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (src->channels != 3) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;

			r = datasrc[pos_src];
			g = datasrc[pos_src + 1];
			b = datasrc[pos_src + 2];

			datasrc[pos_src] = b;
			datasrc[pos_src + 1] = g;
			datasrc[pos_src + 2] = r;
		}
	}
	return 1;
}

int vc_convert_to_three_channels(IVC* src, IVC* dst) {
	int i, j;

	// Check that input image has only one channel
	if (src->channels != 1) {
		printf("Error: input image must have only one channel.\n");
		return 0;
	}

	// Allocate memory for color image
	dst->data = (unsigned char*)malloc(dst->bytesperline * dst->height * sizeof(unsigned char));
	dst->channels = 3;

	// Copy pixel values from black and white image to color image
	for (i = 0; i < src->height; i++) {
		for (j = 0; j < src->width; j++) {
			int bw_idx = i * src->bytesperline + j;
			int color_idx = i * dst->bytesperline + j * dst->channels;

			unsigned char bw_value = src->data[bw_idx];
			unsigned char color_value = bw_value;

			dst->data[color_idx] = color_value;
			dst->data[color_idx + 1] = color_value;
			dst->data[color_idx + 2] = color_value;
		}
	}
	return 1;
}

int vc_rgb_to_gray(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 3) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.229) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}



int vc_rgb_to_hsv(IVC* srcdst, int tipo)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		r = (float)data[i];
		g = (float)data[i + 1];
		b = (float)data[i + 2];

		// Calcula valores maximo e minimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}


		if (tipo == 0) { // moedas 10 20 50 e 1 euro para binario
			if ((hue >= 40.0f) && (hue <= 85.0f) && saturation >= 50.0f && value >= 50.0f) {
				data[i] = 255;
				data[i + 1] = 255;
				data[i + 2] = 255;
			}
			else {
				data[i] = 0;
				data[i + 1] = 0;
				data[i + 2] = 0;
			}
		}
		else if (tipo == 1) { //moedas 1 2 5 centimos para binario
			if ((hue >= 15.0f) && (hue <= 40.0f) && (saturation >= 80.0f)) {
				data[i] = 255;
				data[i + 1] = 255;
				data[i + 2] = 255;
			}
			else {
				data[i] = 0;
				data[i + 1] = 0;
				data[i + 2] = 0;
			}
		}

	}


	return 1;
}
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax) {
	//converter de hsv para segmentar
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	float max, min, hue, sat, valor;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//verificalão de errors
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1))return 0;

	// meter em hsv
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			//calcular value
			//find max
			max = MAX3(rf, gf, bf);
			/*max = rf;
			if (gf > max) {
				max = gf;
			}
			if (bf > max) {
				max = bf;
			}*/

			//find min
			min = MIN3(rf, gf, bf);
			/*min = rf;
			if (gf < min) {
				min = gf;
			}
			if (bf < min) {
				min = bf;
			}*/

			//calcular value
			valor = max;

			//calcular saturação
			if (max == 0 || max == min) {
				sat = 0;
				hue = 0;
			}
			else {
				sat = (max - min) * 100.0f / valor;
				//calcular hue
				// Quando o vermelho é o maior, Hue será um ângulo entre 300 e 360 ou entre 0 e 60
				if (rf == max && gf >= bf) {
					hue = 60.0f * (gf - bf) / (max - min);
				}
				else if (rf == max && bf > gf) {
					hue = 360 + 60.0f * (gf - bf) / (max - min);
				}
				else if (gf == max) {
					hue = 120 + 60.0f * (bf - rf) / (max - min);
				}
				else if (max == bf) {
					hue = 240 + 60.0f * (rf - gf) / (max - min);
				}
			}

			//se o hmin for maior que o hmax  entao hmin ate 360 e de 0 ate hmax

			if (hmin > hmax) {
				if ((hue >= 0 && hue <= hmax || hue <= 360 && hue >= hmin) && sat <= smax && sat >= smin && valor <= vmax / 100.0f * 255 && valor >= vmin / 100.0f * 255)
					datadst[pos_dst] = 255;
				else
					datadst[pos_dst] = 0;
			}
			else {
				if (hue <= hmax && hue >= hmin && sat <= smax && sat >= smin && valor <= vmax / 100.0f * 255 && valor >= vmin / 100.0f * 255)
					datadst[pos_dst] = 255;
				else
					datadst[pos_dst] = 0;
			}
		}
	}
	return 1;
}

int vc_scale_gray_to_rgb(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 3))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;


			if (datasrc[pos_src] <= 64) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = datasrc[pos_src] * (255 / 64);
				datadst[pos_dst + 2] = 255;
			}
			else if (datasrc[pos_src] <= 128) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 255 - (datasrc[pos_src] - 64) * (255 / 64);
			}
			else if (datasrc[pos_src] <= 192) {
				datadst[pos_dst] = (datasrc[pos_src] - 128) * (255 / 64);
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 0;
			}
			else {
				datadst[pos_dst] = 255;
				datadst[pos_dst + 1] = 255 - (datasrc[pos_src] - 192) * (255 / 64);
				datadst[pos_dst + 2] = 0;
			}

		}
	}
	return 1;
}


int vc_gray_to_binary(IVC* src, IVC* dst, int threshold) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

	return 1;
}


int vc_gray_to_binary_global_mean(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	float threshold = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			threshold += datasrc[pos_src];
		}
	}

	threshold = threshold / (width * height);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

}

int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	float threshold = 0;
	int max, min;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;
	kernel *= 0.5;
	for (y = kernel; y < height - kernel; y++)
	{
		for (x = kernel; x < width - kernel; x++)
		{
			pos_dst = y * bytesperline_dst + x * channels_dst;

			for (max = 0, min = 255, y2 = y - kernel; y2 <= y + kernel; y2++)
			{
				for (x2 = x - kernel; x2 <= x + kernel; x2++)
				{
					pos_src = y2 * bytesperline_src + x2 * channels_src;
					if (datasrc[pos_src] > max) { max = datasrc[pos_src]; }
					else if (datadst[pos_src] < min) { min = datasrc[pos_src]; }
				}
			}
			threshold = 0.5 * (max + min);

			pos_src = y * bytesperline_src + x * channels_src;
			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->channels * src->width;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->channels * dst->width;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, vY, vX;
	long int pos_src, pos_dst;
	float threshold;
	float vMax = 0, vMin = 256;

	//Percorre uma imagem pyxel a pyxel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			vY = kernel / 2;
			vX = kernel / 2;



			int xx, yy;

			//Percorre a vizinhança e descobre o Vmax e Vmin da vizinhança e faz o threshould
			for (vMax = 0, vMin = 255, yy = y - vY; yy <= y + vY; yy++)
			{

				for (xx = x - vX; xx <= x + vX; xx++)
				{
					//verificação para não sair fora da imagem
					if (yy >= 0 && yy < height && xx >= 0 && xx < width)
					{
						//Calcula o threshould da vizinhança
						pos_src = yy * bytesperline_src + xx * channels_src;

						if (datasrc[pos_src] > vMax)
						{
							vMax = datasrc[pos_src];
						}
						if (datasrc[pos_src] < vMin)
						{
							vMin = datasrc[pos_src];
						}

					}
				}
			}

			//Caso a variação 
			if ((vMax - vMin) < Cmin)
			{
				threshold = 255 / 2;
			}
			else
			{
				threshold = (vMin + vMax) / 2;
			}


			if (datasrc[pos_src] > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}
	return 1;
}

// Erosão binária
int vc_binary_dilate(IVC* src, IVC* dst, int size)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xk, yk;
	int i, j;
	long int pos, posk;
	int s1, s2;
	unsigned char pixel;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	s2 = (size - 1) / 2;
	s1 = -(s2);

	memcpy(datadst, datasrc, bytesperline * height);

	// Cálculo da erosão
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			pixel = datasrc[pos];

			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;

				if ((j < 0) || (j >= height)) continue;

				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;

					if ((i < 0) || (i >= width)) continue;

					posk = j * bytesperline + i * channels;

					pixel |= datasrc[posk];
				}
			}

			// Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
			// na imagem de destino é também definido como plano de fundo.
			if (pixel == 0) datadst[pos] = 0;
		}
	}

	return 1;
}
// Erosão binária
int vc_binary_erode(IVC* src, IVC* dst, int size)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xk, yk;
	int i, j;
	long int pos, posk;
	int s1, s2;
	unsigned char pixel;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	s2 = (size - 1) / 2;
	s1 = -(s2);

	memcpy(datadst, datasrc, bytesperline * height);

	// Cálculo da erosão
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			pixel = datasrc[pos];

			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;

				if ((j < 0) || (j >= height)) continue;

				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;

					if ((i < 0) || (i >= width)) continue;

					posk = j * bytesperline + i * channels;

					pixel &= datasrc[posk];
				}
			}

			// Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
			// na imagem de destino é também definido como plano de fundo.
			if (pixel == 0) datadst[pos] = 0;
		}
	}

	return 1;
}

int vc_binary_open(IVC* src, IVC* dst, int kernel) {
	IVC* dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	vc_binary_erode(src, dstTemp, kernel);
	vc_binary_dilate(dstTemp, dst, kernel);

	vc_image_free(dstTemp);

	return 1;
}


int vc_binary_close(IVC* src, IVC* dst, int kernel) {
	int verifica = 1;
	IVC* dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	verifica &= vc_binary_dilate(src, dstTemp, kernel);
	verifica &= vc_binary_erode(dstTemp, dst, kernel);

	vc_image_free(dstTemp);

	return verifica;
}

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int* labeltable = (int*)calloc(width * height, sizeof(int)); // Array para mapear etiquetas
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objetos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return NULL;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatoriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatoriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1, width*height - 1]
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = width * height;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Atualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objetos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

int vc_gray_histogram_equalization(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	float min;
	long int pos_src, pos_dst;
	int posicoes[256] = { 0 };
	float pdf[256] = { 0 }, cdf[256] = { 0 };
	float size = width * height;

	for (x = 0; x < height * width; x++)
	{
		posicoes[datasrc[x]]++;
	}
	for (x = 0; x <= 255; x++)
	{
		pdf[x] = (((float)posicoes[x]) / size);
	}

	for (cdf[0] = pdf[0], x = 1; x <= 255; x++)
	{
		cdf[x] = cdf[x - 1] + pdf[x];
	}


	for (min = 0, x = 0; x <= 255; x++)
	{
		if (cdf[x] != 0) { min = pdf[x]; break; }
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_dst = y * bytesperline_src + x * channels_src;
			datadst[pos_dst] = ((cdf[datasrc[pos_dst]] - min) / (1.0 - min)) * 255.0;
		}
	}

	return 1;
}

