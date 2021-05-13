#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
    #include "libvmaf/picture.h"
    #include "libvmaf/libvmaf.h"
}

static void copy_data(uint8_t *src, VmafPicture *dst, unsigned width,
                      unsigned height, int src_stride)
{
    uint8_t *a = src;
    uint8_t *b = (uint8_t*)dst->data[0];
    for (unsigned i = 0; i < height; i++) {
        for (unsigned j = 0; j < width; j++) {
            b[j] = a[j];
        }
        a += src_stride / sizeof(float);
        b += dst->stride[0];
    }
}

int main(int argc, char *argv[]) {
    FILE *f[2];
    uint8_t *buf[2], *plane[2][3];
    int frame_size, w, h;
    int frame_index, seek;
    int i;
    int stride;

    if (argc < 4 || 2 != sscanf(argv[3], "%dx%d", &w, &h)) {
        printf("test_vif <file1.yuv> <file2.yuv> <width>x<height> [<seek>]\n");
        return -1;
    }

    f[0] = fopen(argv[1], "rb");
    f[1] = fopen(argv[2], "rb");
    sscanf(argv[3], "%dx%d", &w, &h);

    if (w <= 0 || h <= 0 || w * (int64_t)h >= INT_MAX / 3) {
        fprintf(stderr, "Dimensions are too large, or invalid\n");
        return -2;
    }

    stride = w * sizeof(float);
    frame_size = w * h * 3LL / 2;

    for (i = 0; i < 2; i++) {
        buf[i] = (uint8_t *)malloc(frame_size);
        plane[i][0] = buf[i];
        plane[i][1] = plane[i][0] + w * h;
        plane[i][2] = plane[i][1] + w * h / 4;
    }
    
    seek = argc < 5 ? 0 : atoi(argv[4]);
    fseek(f[seek < 0], seek < 0 ? -seek : seek, SEEK_SET);

    int err = 0;
    VmafConfiguration cfg = {
        .log_level = VMAF_LOG_LEVEL_INFO,
        .n_threads = 1,
        .n_subsample = 1
    };

    VmafContext *vmaf;
    err = vmaf_init(&vmaf, cfg);
    if (err) {
        fprintf(stderr, "problem initializing VMAF context\n");
        return -1;
    }

    err = vmaf_use_feature(vmaf, "vif", NULL);
    if (err) {
        printf("use vif failed! \n");
        return -1;
    }

    for (frame_index = 0;; frame_index++) {
        if (fread(buf[0], frame_size, 1, f[0]) != 1)
            break;
        if (fread(buf[1], frame_size, 1, f[1]) != 1)
            break;
        
        // plane[0][0]-lumance for refence image
        // plane[1][0]-lumance for distortion image
        VmafPicture pic_ref, pic_dist;
        vmaf_picture_alloc(&pic_ref,  VMAF_PIX_FMT_YUV420P, 8, w, h);
        vmaf_picture_alloc(&pic_dist, VMAF_PIX_FMT_YUV420P, 8, w, h);
        copy_data(plane[0][0], &pic_ref, w, h, stride);
        copy_data(plane[1][0], &pic_dist, w, h, stride);

        err = vmaf_read_pictures(vmaf, &pic_ref, &pic_dist, frame_index);
        if (err) {
            printf("problem reading pictures\n");
            break;
        }

        vmaf_picture_unref(&pic_ref);
        vmaf_picture_unref(&pic_dist);
    }

    err = vmaf_read_pictures(vmaf, NULL, NULL, 0);
    if (err) {
        printf("problem flushing context\n");
        return err;
    }

    double s[5] = {0};
    for (int i = 0; i < frame_index; i++) {
        vmaf_feature_score_at_index(vmaf, "VMAF_integer_feature_vif_scale0_score", s, i);
        vmaf_feature_score_at_index(vmaf, "VMAF_integer_feature_vif_scale1_score", s + 1, i);
        vmaf_feature_score_at_index(vmaf, "VMAF_integer_feature_vif_scale2_score", s + 2, i);
        vmaf_feature_score_at_index(vmaf, "VMAF_integer_feature_vif_scale3_score", s + 3, i);
        vmaf_feature_score_at_index(vmaf, "integer_vif", s + 4, i);

        printf("frame_index:%d ", i);
        for (int j = 0; j < 4; j++) {
            printf("VMAF_integer_feature_vif_scale%d_score:%f ", j, s[j]);
        }
        printf("VMAF_integer_vif:%f \n", s[4]);
    }
    return 0;
}