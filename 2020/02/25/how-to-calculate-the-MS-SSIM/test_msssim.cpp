/*
 * Copyright (c) 2003-2013 Loren Merritt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 USA
 */
/*
 * tiny_msssim.c
 * Computes the Multi-Scale Structural Similarity Metric between two rawYV12 video files.
 * original algorithm:
 * Z. Wang, E. P. Simoncelli and A. C. Bovik,
 *   "Multi-scale structural similarity for image quality assessment,"
 *   The Thrity-Seventh Asilomar Conference on Signals, Systems & Computers, 
 *       2003, Pacific Grove, CA, USA, 2003, pp. 1398-1402 Vol.2.
 *
 * To improve speed, this implementation uses the standard approximation of
 * overlapped 8x8 block sums, rather than the original gaussian weights.
 */

#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define FFSWAP(type, a, b) \
    do                     \
    {                      \
        type SWAP_tmp = b; \
        b = a;             \
        a = SWAP_tmp;      \
    } while (0)
#define FFMIN(a, b) ((a) > (b) ? (b) : (a))

#define BIT_DEPTH 8                      // 位深 使用几位来定位一个像素点 8位的话 像素值范围就是0-255
#define PIXEL_MAX ((1 << BIT_DEPTH) - 1) // 像素值最大值 公式里的L
typedef uint8_t pixel;                   // 8位无符号 表示像素值

const float WEIGHT[] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};

typedef struct
{
    float L;
    float C_S;
} ssim_value;

/****************************************************************************
 * structural similarity metric
 ****************************************************************************/
static void ssim_4x4x2_core(const pixel *pix1, intptr_t stride1,
                            const pixel *pix2, intptr_t stride2,
                            int sums[2][4])
{
    int x, y, z;

    for (z = 0; z < 2; z++)
    {
        uint32_t s1 = 0, s2 = 0, ss = 0, s12 = 0;
        for (y = 0; y < 4; y++)
            for (x = 0; x < 4; x++)
            {
                int a = pix1[x + y * stride1];
                int b = pix2[x + y * stride2];
                s1 += a;
                s2 += b;
                ss += a * a;
                ss += b * b;
                s12 += a * b;
            }
        sums[z][0] = s1;
        sums[z][1] = s2;
        sums[z][2] = ss;
        sums[z][3] = s12;
        pix1 += 4;
        pix2 += 4;
    }
}

static ssim_value ssim_end1(int s1, int s2, int ss, int s12)
{
    ssim_value value;
/* Maximum value for 10-bit is: ss*64 = (2^10-1)^2*16*4*64 = 4286582784, which will overflow in some cases.
 * s1*s1, s2*s2, and s1*s2 also obtain this value for edge cases: ((2^10-1)*16*4)^2 = 4286582784.
 * Maximum value for 9-bit is: ss*64 = (2^9-1)^2*16*4*64 = 1069551616, which will not overflow. */
#if BIT_DEPTH > 9
    typedef double type;
    static const double ssim_c1 = .01 * .01 * PIXEL_MAX * PIXEL_MAX * 64 * 64;
    static const double ssim_c2 = .03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63;
#else
    typedef int type;
    // k1=0.01, k2=0.03
    static const int ssim_c1 = (int)(.01 * .01 * PIXEL_MAX * PIXEL_MAX * 64 * 64 + .5);
    static const int ssim_c2 = (int)(.03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63 + .5);
#endif
    type fs1 = s1;
    type fs2 = s2;
    type fss = ss;
    type fs12 = s12;
    type vars = fss * 64 - fs1 * fs1 - fs2 * fs2;
    type covar = fs12 * 64 - fs1 * fs2;
    
    value.L = (float)(2 * fs1 * fs2 + ssim_c1) / (float)(fs1 * fs1 + fs2 * fs2 + ssim_c1);
    value.C_S = (float)(2 * covar + ssim_c2) / (float)(vars + ssim_c2);

    return value;
}

static ssim_value ssim_end4(int sum0[5][4], int sum1[5][4], int width)
{
    ssim_value ssim;
    ssim.L = 0.0;
    ssim.C_S = 0.0;

    int i;
    for (i = 0; i < width; i++)
    {
        ssim_value tmp;
        tmp = ssim_end1(sum0[i][0] + sum0[i + 1][0] + sum1[i][0] + sum1[i + 1][0],
                        sum0[i][1] + sum0[i + 1][1] + sum1[i][1] + sum1[i + 1][1],
                        sum0[i][2] + sum0[i + 1][2] + sum1[i][2] + sum1[i + 1][2],
                        sum0[i][3] + sum0[i + 1][3] + sum1[i][3] + sum1[i + 1][3]);
        ssim.L   += tmp.L;
        ssim.C_S += tmp.C_S;
    }

    return ssim;
}

ssim_value ssim_plane(
    pixel *pix1, intptr_t stride1,
    pixel *pix2, intptr_t stride2,
    int width, int height, void *buf, int *cnt)
{
    int z = 0;
    int x, y;
    ssim_value ssim;
    ssim.L = 0.0;
    ssim.C_S = 0.0;

    int(*sum0)[4] = (int(*)[4])buf; 
    int(*sum1)[4] = sum0 + (width >> 2) + 3;
    width >>= 2;
    height >>= 2; 
    for (y = 1; y < height; y++)
    {
        for (; z <= y; z++)
        {
            // FFSWAP( (int (*)[4]), sum0, sum1 );
            int(*tmp)[4] = sum0;
            sum0 = sum1;
            sum1 = tmp;

            for (x = 0; x < width; x += 2)
                ssim_4x4x2_core(&pix1[4 * (x + z * stride1)], stride1, &pix2[4 * (x + z * stride2)], stride2, &sum0[x]);
        }

        for (x = 0; x < width - 1; x += 4)
        {
            ssim_value tmp;
            tmp = ssim_end4(sum0 + x, sum1 + x, FFMIN(4, width - x - 1));
            ssim.L   += tmp.L;
            ssim.C_S += tmp.C_S;
        }
    }

    ssim.L /= (height - 1) * (width - 1);
    ssim.C_S /= (height - 1) * (width - 1);
    return ssim;
}

static void print_results(float ms_ssim[3], int frames, int w, int h)
{
    printf("MS-SSIM Y:%.5f U:%.5f V:%.5f All:%.5f",
           ms_ssim[0] / frames,
           ms_ssim[1] / frames,
           ms_ssim[2] / frames,
           (ms_ssim[0] * 4 + ms_ssim[1] + ms_ssim[2]) / (frames * 6));
}

static void downsample_2x2_mean(pixel *input, int width, int height, pixel *output) 
{
    int downsample_width =  width >> 1;
    int downsample_height = height >> 1;

    for (int y = 0; y < downsample_height; y++) 
    {
        for (int x =0; x < downsample_width; x++) 
        {
            output[y * downsample_width + x] = (input[2 * y * width + 2 * x] +
                                                input[2 * y * width + 2 * x + 1] +
                                                input[(2 * y + 1) * width + 2 * x] +
                                                input[(2 * y + 1) * width + 2 * x + 1]) / 4;
        }
    }
}

static float ms_ssim_plane(pixel *pix1, pixel *pix2, int width, int height, int scale = 5)
{
    ssim_value value;
    float result = 1.0;
    float luminance_value[5];
    int w = width;
    int h = height;

    int   *temp;
    pixel *ori_img1;
    pixel *ori_img2;
    pixel *sample_img1;
    pixel *sample_img2;

    if (scale < 1 || scale > 5) 
    {
        scale = 5;
    }

    temp        = (int *)malloc((2*w+12)*sizeof(*temp));
    ori_img1    = (pixel*)malloc(w * h * sizeof(pixel));
    ori_img2    = (pixel*)malloc(w * h * sizeof(pixel));
    sample_img1 = (pixel*)malloc(w * h * sizeof(pixel));
    sample_img2 = (pixel*)malloc(w * h * sizeof(pixel));

    for (int i = 0; i < w * h; i++) 
    {
        ori_img1[i] = pix1[i];
        ori_img2[i] = pix2[i];
    }

    // 计算每个尺度的ssim值.
    for (int i = 1; i <= scale; i++) 
    {
        if (i != 1) 
        {
            memset(sample_img1, 0, width * height);
            memset(sample_img2, 0, width * height);

            downsample_2x2_mean(ori_img1, w, h, sample_img1);
            downsample_2x2_mean(ori_img2, w, h, sample_img2);

            w = w >> 1;
            h = h >> 1;

            memset(ori_img1, 0, width * height);
            memset(ori_img2, 0, width * height);

            for (int j = 0; j < w * h; j++) 
            {
                ori_img1[j] = sample_img1[j];
                ori_img2[j] = sample_img2[j];
            }
        }

        value = ssim_plane(ori_img1, w, ori_img2, w, w, h, temp, NULL);
        result *= pow(value.C_S, WEIGHT[i-1]);
        luminance_value[i-1] = value.L;
    }

    free(temp);
    free(ori_img1);
    free(ori_img2);
    free(sample_img1);
    free(sample_img2);
    temp = NULL;
    ori_img1 = NULL;
    ori_img2 = NULL;
    sample_img1 = NULL;
    sample_img2 = NULL;

    result *= pow(luminance_value[scale-1], WEIGHT[scale-1]);
    return result;
}

int main(int argc, char *argv[])
{
    FILE *f[2];
    uint8_t *buf[2], *plane[2][3];
    int *temp;
    float ms_ssim[3] = {0, 0, 0};
    int frame_size, w, h;
    int frames, seek;
    int i;

    // 输入格式
    if (argc < 4 || 2 != sscanf(argv[3], "%dx%d", &w, &h))
    {
        printf("ms-ssim <file1.yuv> <file2.yuv> <width>x<height> [<seek>]\n");
        return -1;
    }

    // 读入两个文件 长x宽
    f[0] = fopen(argv[1], "rb");
    f[1] = fopen(argv[2], "rb");
    sscanf(argv[3], "%dx%d", &w, &h);

    if (w <= 0 || h <= 0 || w * (int64_t)h >= INT_MAX / 3 || 2LL * w + 12 >= INT_MAX / sizeof(*temp))
    {
        fprintf(stderr, "Dimensions are too large, or invalid\n");
        return -2;
    }

    // 一帧的内存大小
    // yuv420格式：先w*h个Y，然后1/4*w*h个U，再然后1/4*w*h个
    frame_size = w * h * 3LL / 2;

    // plane[i][0] Y分量信息
    // plane[i][1] U分量信息
    // plane[i][2] V分量信息
    for (i = 0; i < 2; i++)
    {
        buf[i] = (uint8_t *)malloc(frame_size);
        plane[i][0] = buf[i]; // plane[i][0] = buf[i]
        plane[i][1] = plane[i][0] + w * h;
        plane[i][2] = plane[i][1] + w * h / 4;
    }

    seek = argc < 5 ? 0 : atoi(argv[4]);
    fseek(f[seek < 0], seek < 0 ? -seek : seek, SEEK_SET);

    // 逐帧计算
    for (frames = 0;; frames++)
    {
        float ms_ssim_one[3]; // Y U V 三个向量一帧ms-ssim的结果
        // 分别读入这一帧Y向量的地址，随之也获得了UV向量的起始地址
        if (fread(buf[0], frame_size, 1, f[0]) != 1)
            break;
        if (fread(buf[1], frame_size, 1, f[1]) != 1)
            break;
        for (int i = 0; i < 3; i++)
        {
            ms_ssim_one[i] = ms_ssim_plane(plane[0][i], plane[1][i], w >> !!i, h >> !!i);
            ms_ssim[i] += ms_ssim_one[i];
        }

        printf("Frame %d | ", frames);
        print_results(ms_ssim_one, 1, w, h);
        printf("                \r");
        fflush(stdout);
    }

    if (!frames)
        return 0;

    printf("Total %d frames | ", frames);
    print_results(ms_ssim, frames, w, h);
    printf("\n");

    return 0;
}