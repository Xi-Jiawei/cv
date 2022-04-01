#include "utils.h"
#include "edgedetect.h"

/** \ingroup edgedetect
    \brief Convert rgb to gray
    \param dst Output padding image data
    \param src Input image data
    \param width Input image width
    \param height Input image height
    \param samples_per_pixel Input image samples per pixel
    \param padding_size Padding size
    \returns 1 if an error occurred, 0 else

    This function pads a image around.
    此函数实现图像扩展。
*/
int padding(uint8_t **dst, uint8_t *src, int width, int height, int samples_per_pixel, int padding_size)
{
    int padding_width, padding_height;
    int linesize, padding_linesize;
	int i, j, ret;
    uint8_t *p, *q;

    linesize = width * samples_per_pixel;
    padding_width = width + padding_size * 2;
    padding_height = height + padding_size * 2;
    padding_linesize = padding_width * samples_per_pixel;

    *dst = (uint8_t*)calloc(padding_linesize * padding_height, sizeof(uint8_t));
    for (j = 0, p = *dst + padding_linesize * padding_size + padding_size * samples_per_pixel, q = src; j < height; ++j, p += padding_linesize, q += width)
        memcpy(p, q, width);

    fprintf(stderr, "Padding succeeded.\n");

	return 0;
}

/** \ingroup edgedetect
    \brief Convert rgb to gray
    \param dst Output gray image data
    \param src Input rgb image data
    \param width Input image width
    \param height Input image height
    \returns 1 if an error occurred, 0 else

    This function converts a rgb image to a gray image.
    此函数实现rgb图转灰度图。
*/
int rgb2gray(uint8_t **dst, uint8_t *src, int width, int height)
{
	int i, j, ret;
    uint8_t *p, *q;

    // rgb转bayer
    *dst = (uint8_t*)malloc(width * height);
    p = *dst, q = src;
    for (j = 0; j < height; ++j) {
    	for (i = 0; i < width; ++i, p += 1, q += 3) {
            p[0] = (299 * q[0] + 587 * q[1] + 114 * q[2]) / 1000;
    	}
    }

    fprintf(stderr, "Transform rgb to gray succeeded.\n");

	return 0;
}

/** \ingroup edgedetect
    \brief Gaussian matrix
    \param winsize window size of the gaussian kernel matrix
    \param sigma Gaussian sigma
    \returns a gaussian kernel matrix

    This function makes a gaussian kernel matrix.
    此函数生成高斯核矩阵。
*/
double *gaussian_matrix(int winsize, double sigma)
{
	double *kernel, sum = 0.0;
	int winradius = winsize / 2; // 一般而言，winsize为单数
	kernel = (double*)calloc(winsize, sizeof(double));

	double d = (2.0 * sigma * sigma);
	double t = sqrt(2.0 * 3.14159)*sigma;
	for (int x = 0; x <= winradius; x++) {
		kernel[x] = kernel[winsize - 1 - x] = exp(-((winradius - x) * (winradius - x)) / d) / t;
		sum += kernel[x] + ((winradius - x != 0) ? kernel[winsize - 1 - x] : 0.0);
	}
	for (int x = 0; x < winsize; x++)
		kernel[x] /= sum;

	return kernel;
}

/** \ingroup edgedetect
    \brief Gaussian blur
    \param dst Output blurred image data
    \param src Input image data
    \param width Input image width
    \param height Input image height
    \param bitcount Input image bitcount
    \param sigma Gaussian sigma
    \returns 1 if an error occurred, 0 else

    This function blurs a image by gaussian kernel matrix.
    此函数实现图像的高斯模糊。
*/
int gaussian_blur(uint8_t **dst, uint8_t *src, int width, int height, int bitcount, double sigma)
{
	int	winsize, winradius;
	double *matrix;
	double v;
	int i, j, k, l, ret;
	int padding_width, padding_height, padding_i, padding_j;
	uint8_t *padding_data;
	uint8_t *temp;
    uint8_t *p, *q;

	winsize = (1 + (((int)ceil(3 * sigma)) * 2));//根据高斯模糊的3*sigma原则设计滤波器窗口大小
	winradius = winsize / 2;
	matrix = gaussian_matrix(winsize, sigma);

	if (bitcount == 8) {
		*dst = (uint8_t*)malloc(width * height);
		temp = (uint8_t*)malloc(width * height);

		padding_width = width + winradius * 2;

		// 横向Smooth
		padding(&padding_data, src, width, height, 1, winradius);
		p = temp, q = padding_data;
		for (j = 0; j < height; j++) {
			for (i = 0; i < width; i++, p += 1) {
				padding_i = i + winradius;
				padding_j = j + winradius;
				v = 0.0;
				for (l = 0; l < winsize; ++l) {
					v += padding_data[padding_width * padding_j + padding_i + l - winradius] * matrix[l];
				}
				p[0] = (uint8_t)v;
			}
		}
		free(padding_data);

		// 纵向Smooth
		padding(&padding_data, temp, width, height, 1, winradius);
		p = *dst, q = temp;
		for (j = 0; j < height; j++) {
			for (i = 0; i < width; i++, p += 1) {
				padding_i = i + winradius;
				padding_j = j + winradius;
				v = 0.0;
				for (l = 0; l < winsize; ++l)
					v += padding_data[padding_width * (padding_j + l - winradius) + padding_i] * matrix[l];
				p[0] = (uint8_t)v;
			}
		}
		free(padding_data);
	} else if (bitcount == 24) {
		*dst = (uint8_t*)malloc(width * height * 3);
		temp = (uint8_t*)malloc(width * height * 3);

		padding_width = (width + winradius * 2) * 3;

		// 横向Smooth
		padding(&padding_data, src, width, height, 3, winradius);
		p = temp, q = padding_data;
		for (j = 0; j < height; j++) {
			for (i = 0; i < width; i++, p += 1) {
				padding_i = i + winradius;
				padding_j = j + winradius;

				for (k = 0; k < 3; ++k) {
					v = 0.0;
					for (l = 0; l < winsize; ++l)
						v += padding_data[padding_width * padding_j + padding_i + l - winradius + k] * matrix[l];
					p[k] = (uint8_t)v;
				}
			}
		}
		free(padding_data);

		// 纵向Smooth
		padding(&padding_data, temp, width, height, 3, winradius);
		p = *dst, q = temp;
		for (j = 0; j < height; j++) {
			for (i = 0; i < width; i++, p += 1) {
				padding_i = i + winradius;
				padding_j = j + winradius;

				for (k = 0; k < 3; ++k) {
					v = 0.0;
					for (l = 0; l < winsize; ++l)
						v += padding_data[padding_width * (padding_j + l - winradius) + padding_i + k] * matrix[l];
					p[k] = (uint8_t)v;
				}
			}
		}
		free(padding_data);
	}

	free(temp);
	free(matrix);

	return 0;
}

/** \ingroup edgedetect
    \brief Calculate gradiants map of a image by simple algorithm
    \param dst Output gradiant map
    \param src Input image data
    \param width Input image width
    \param height Input image height
    \returns 1 if an error occurred, 0 else

    This function calculates gradiants map of a image with simple algorithm.
    此函数实现简单的梯度图计算。（只支持处理8bit灰度图，并且输出16bit梯度图）
*/
int gradient_simple(int16_t **dst, uint8_t *src, int width, int height) {
	int dx, dy;
	int i, j, ret;
    int16_t *p;
    uint8_t *q;

	*dst = (int16_t*)malloc(width * height * sizeof(int16_t));
	p = *dst, q = src;
	for (j = 1; j < height - 1; ++j) {
    	for (i = 1; i < width - 1; ++i, p += 1) {
    		dx = src[width * j + i] - src[width * j + i - 1]; // x deviation
    		dy = src[width * j + i] - src[width * (j - 1) + i]; // y deviation
            p[0] = (int16_t)sqrt(dx * dx + dy * dy);
    	}
    }

	return 0;
}

/** \ingroup edgedetect
    \brief Calculate gradiants map of a image by sobel operator algorithm
    \param dst Output gradiant map
    \param dx Output x axis gradiant map
    \param dy Output y axis gradiant map
    \param src Input image data
    \param width Input image width
    \param height Input image height
    \returns 1 if an error occurred, 0 else

    This function calculates gradiants map of a image with sobel operator algorithm.
    此函数实现基于Sobel算子的梯度图计算。（只支持处理8bit灰度图，并且输出16bit梯度图）
    Sobel梯度算法分别计算x轴和y轴两个方向的梯度。Sobel算子：
    偏x方向
    [-1, 0, 1;
    -2, 0, 2;
    -1, 0, 1]
    偏y方向
    [-1, -2, -1;
    0, 0, 0;
    1, 2, 1]
*/
int gradient_sobel(int16_t **dst, int16_t **dx, int16_t **dy, uint8_t *src, int width, int height) {
	int16_t gradx, grady, grad;
	int i, j, pos, ret;
    int16_t *p, *r, *s;
    uint8_t *q;

	*dst = (int16_t*)calloc(width * height, sizeof(int16_t));
	*dx = (int16_t*)calloc(width * height, sizeof(int16_t));
	*dy = (int16_t*)calloc(width * height, sizeof(int16_t));
	p = *dst, q = src, r = *dx, s = *dy;
	for (j = 1; j < height - 1; ++j) {
    	for (i = 1; i < width - 1; ++i) {
    		pos = width * j + i;
            gradx = src[width * (j - 1) + i - 1] * (-1) +
                src[width * j       + i - 1] * (-2) +
                src[width * (j + 1) + i - 1] * (-1) +
                src[width * (j - 1) + i + 1] * (1) +
                src[width * j       + i + 1] * (2) +
                src[width * (j + 1) + i + 1] * (1);
            grady = src[width * (j - 1) + i - 1] * (-1) +
                src[width * (j - 1) + i    ] * (-2) +
                src[width * (j - 1) + i + 1] * (-1) +
                src[width * (j + 1) + i - 1] * (1) +
                src[width * (j + 1) + i    ] * (2) +
                src[width * (j + 1) + i + 1] * (1);
			gradx /= 4;
			grady /= 4;
			r[pos] = gradx;
			s[pos] = grady;
			//grad = (int16_t)sqrt(gradx * gradx + grady * grady);
			//p[pos] = grad > 255 ? 255 : grad;
			p[pos] = (int16_t)sqrt(gradx * gradx + grady * grady);
    	}
    }

	return 0;
}

/** \ingroup edgedetect
    \brief Calculate gradiants map of a image by laplacian operator algorithm
    \param dst Output gradiant map
    \param src Input image data
    \param width Input image width
    \param height Input image height
    \param has_diagonals use laplacian operator with diagonals
    \returns 1 if an error occurred, 0 else

    This function calculates gradiants map of a image with laplacian operator algorithm.
    此函数实现基于Laplacian算子的梯度图计算。（只支持处理8bit灰度图，并且输出16bit梯度图）
    Laplacian算子：
    [0, -1, 0;
    -1, 4, -1;
    0, -1, 0]
    Laplacian算子（带边角）：
    [-1, -1, -1;
    -1, 8, -1;
    -1, -1, -1]
*/
int gradient_laplacian(int16_t **dst, uint8_t *src, int width, int height, int has_diagonals) {
	int16_t gradx, grady, grad;
	int i, j, pos, ret;
    int16_t *p;
    uint8_t *q;

	*dst = (int16_t*)calloc(width * height, sizeof(int16_t));
	p = *dst, q = src;
	if (has_diagonals) {
		for (j = 1; j < height - 1; ++j) {
			for (i = 1; i < width - 1; ++i) {
    			p[width * j + i] = src[width * j + i] * 8 -
    				src[width * (j - 1) + i - 1] -
                	src[width * (j - 1) + i    ] -
                	src[width * (j - 1) + i + 1] -
                	src[width * j       + i - 1] -
    				src[width * j       + i + 1] -
                	src[width * (j + 1) + i - 1] -
                	src[width * (j + 1) + i    ] -
                	src[width * (j + 1) + i + 1];
    		}
    	}
    } else {
    	for (j = 1; j < height - 1; ++j) {
    		for (i = 1; i < width - 1; ++i) {
    			p[width * j + i] = src[width * j + i] * 4 -
    				src[width * j       + i - 1] -
                	src[width * j       + i + 1] -
                	src[width * (j - 1) + i    ] -
                	src[width * (j + 1) + i    ];
    		}
    	}
    }

	return 0;
}

/** \ingroup edgedetect
    \brief Non-maximum suppression
    \param dst Output nms-gradiant map
    \param src Input gradiant map
    \param dx Input x axis gradiant map
    \param dy Input y axis gradiant map
    \param width Input gradiant map width
    \param height Input gradiant map height
    \returns 1 if an error occurred, 0 else

    This function process the gradiants map with non-maximum suppression algorithm.
    此函数对梯度图进行非极大值抑制。（注意输入的是16bit的梯度图）
*/
int gradient_nms(uint8_t **dst, int16_t *src, int16_t *dx, int16_t *dy, int width, int height) {
	double gradx, grady, grad1, grad2, t;
	int i, j, pos, ret;
    uint8_t *p;
    int16_t *q;

	*dst = (uint8_t*)calloc(width * height, sizeof(uint8_t));
	p = *dst, q = src;
	for (j = 1; j < height - 1; ++j) {
    	for (i = 1; i < width - 1; ++i) {
    		pos = width * j + i;
    		p[pos] = CLIP255(src[pos]);
    		gradx = dx[pos]; // x deviation
    		grady = dy[pos]; // y deviation
    		t = grady / gradx;
    		if (t > 0 && t <= 1) { // 0°~45°、-180°~-135°
    			grad1 = src[width * (j + 1) + i + 1] * t + src[width * j + i + 1] * (1 - t); // a22*t + a12*(1-t)
				grad2 = src[width * (j - 1) + i - 1] * t + src[width * j + i - 1] * (1 - t); // a00*t + a10*(1-t)
				if (p[pos] < grad1 || p[pos] < grad2)
					p[pos] = 0;
			} else if (t > 1) { // 45°~90°、-135°~-90°
				t = gradx / grady;
				grad1 = src[width * (j + 1) + i + 1] * t + src[width * (j + 1) + i] * (1 - t); // a22*t + a12*(1-t)
				grad2 = src[width * (j - 1) + i - 1] * t + src[width * (j - 1) + i] * (1 - t); // a00*t + a10*(1-t)
				if (p[pos] < grad1 || p[pos] < grad2)
					p[pos] = 0;
			} else if (t < -1) { // 90°~135°、-90°~-45°
				t = -gradx / grady;
				grad1 = src[width * (j + 1) + i - 1] * t + src[width * (j + 1) + i] * (1 - t); // a22*t + a12*(1-t)
				grad2 = src[width * (j - 1) + i + 1] * t + src[width * (j - 1) + i] * (1 - t); // a00*t + a10*(1-t)
				if (p[pos] < grad1 || p[pos] < grad2)
					p[pos] = 0;
			} else if (t >= -1 && t <= 0) { //135°~180°、-45°~0°
				t = -t;
				grad1 = src[width * (j + 1) + i - 1] * t + src[width * j + i - 1] * (1 - t); // a22*t + a12*(1-t)
				grad2 = src[width * (j - 1) + i + 1] * t + src[width * j + i + 1] * (1 - t); // a00*t + a10*(1-t)
				if (p[pos] < grad1 || p[pos] < grad2)
					p[pos] = 0;
			}
    	}
    }

	return 0;
}

/** \ingroup edgedetect
    \brief Detect edges
    \param dst Output edge image
    \param src Input gradiant map
    \param width Input gradiant map width
    \param height Input gradiant map height
    \returns 1 if an error occurred, 0 else

    This function process the gradiants map with dual threshold suppression algorithm, then link edge.
    此函数对梯度图进行双阈值抑制，然后通过抑制孤立低阈值点来连接边缘。
*/
int detect_edge(uint8_t **dst, uint8_t *src, int width, int height) {
	int size;
	int histogram[256], sum, threshold_sum, low_threshold, high_threshold;
	int i, j, pos, ret;
    uint8_t *p, *q;

    size = width * height;
	*dst = (uint8_t*)calloc(size, sizeof(uint8_t));

	// 统计梯度直方图，并计算高低阈值
	memset(histogram, 0, sizeof(int) * 256);
	for (i = 0, p = src; i < size; ++i)
		histogram[*p++]++;
	//threshold_sum = size * 0.03;
	//sum = 0;
	//for (i = 255; i >= 0; --i) {
	//	sum += histogram[i];
	//	if (sum >= threshold_sum) {
	//		high_threshold = i;
	//		low_threshold = i / 3;
	//		break;
	//	}
	//}
	threshold_sum = (size - histogram[0] - histogram[1] - histogram[2]) * 0.3;
	sum = 0;
	for (i = 255; i >= 0; --i) {
		sum += histogram[i];
		if (sum >= threshold_sum) {
			high_threshold = i;
			low_threshold = i / 3;
			break;
		}
	}
	fprintf(stdout, "[edgedetect] low_threshold: %d, high_threshold: %d\n", low_threshold, high_threshold);

	// 双阈值抑制
	//for (i = 0, p = *dst, q = src; i < size; ++i, p += 1, q += 1) {
	//	if (src[0] < low_threshold) p[0] = 0;
	//	else if (q[0] > high_threshold) p[0] = 255;
	//	else p[0] = q[0];
	//}
	// 抑制孤立低阈值点
	//for (j = 1, p = *dst; j < height - 1; ++j) {
    //	for (i = 1; i < width - 1; ++i, p += 1) {
    //		pos = width * j + i;
    //		if (p[pos] >= low_threshold && p[pos] <= high_threshold &&
    //			!(p[pos - width - 1] == 255 || 
    //			p[pos - width] == 255 || 
    //			p[pos - width + 1] == 255 || 
    //			p[pos - 1] == 255 || 
    //			p[pos + 1] == 255 || 
    //			p[pos + width - 1] == 255 || 
    //			p[pos + width] == 255 || 
    //			p[pos + width + 1] == 255))
    //			p[0] = 0; // 当介于低阈值和高阈值之间，考虑其是否连接强边缘点：如果周边8个点有1个强边缘点，则保留；否则抑制
    //	}
    //}

	// 双阈值抑制以及抑制孤立低阈值点
	for (j = 1, p = *dst; j < height - 1; ++j) {
    	for (i = 1; i < width - 1; ++i) {
    		pos = width * j + i;
    		if (src[pos] < low_threshold) p[pos] = 0;
    		else if (src[pos] > high_threshold) p[pos] = 255;
    		else if (src[pos - width - 1] > high_threshold || 
    			src[pos - width] > high_threshold || 
    			src[pos - width + 1] > high_threshold || 
    			src[pos - 1] > high_threshold || 
    			src[pos + 1] > high_threshold || 
    			src[pos + width - 1] > high_threshold || 
    			src[pos + width] > high_threshold || 
    			src[pos + width + 1] > high_threshold)
    			p[pos] = src[pos]; // 当介于低阈值和高阈值之间，考虑其是否连接强边缘点：如果周边8个点有1个强边缘点，则保留；否则抑制
    	}
    }

	return 0;
}

/** \ingroup edgedetect
    \brief Detect sharp edges
    \param dst Output edge image
    \param src Input gradiant map
    \param width Input gradiant map width
    \param height Input gradiant map height
    \returns 1 if an error occurred, 0 else

    This function detects sharp edges by the processing gradiants map with high threshold suppression algorithm.
    此函数对梯度图进行阈值抑制来过滤掉虚焦边缘，生成焦点处边缘。
*/
int detect_sharp_edge(uint8_t **dst, uint8_t *src, int width, int height) {
	int size;
	int histogram[256], sum, threshold_sum, cnt, flag, low_threshold, high_threshold;
	int i, j, pos, ret;
    uint8_t *p, *q;

    size = width * height;
	*dst = (uint8_t*)calloc(size, sizeof(uint8_t));

	// 统计梯度直方图，并计算高低阈值
	memset(histogram, 0, sizeof(int) * 256);
	for (i = 0, p = src; i < size; ++i)
		histogram[*p++]++;
	sum = 0;
	flag = 0;
	for (i = 255; i >= 0; --i) {
		cnt = !!(histogram[i]);
		if (!flag && cnt) {
			flag = 1;
			if (i < 24) {
				high_threshold = i / 3;
			}
		}
		sum += cnt;
		if (sum > 24) {
			high_threshold = i;
			break;
		}
	}
	fprintf(stdout, "[edgedetect] high_threshold: %d\n", high_threshold);

	// 高阈值加强
	for (j = 1, p = *dst; j < height - 1; ++j) {
    	for (i = 1; i < width - 1; ++i) {
    		pos = width * j + i;
    		if (src[pos] > high_threshold) p[pos] = 255;
    	}
    }

	return 0;
}
