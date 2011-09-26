
#include "cvsu_uyvy.h"

string convert_uyvy16_to_grey8_name = "convert_uyvy16_to_grey8";
string scale_uyvy16_2_uyvy16_x2_name = "scale_uyvy16_2_uyvy16_x2";

result convert_uyvy16_to_grey8(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == U8);
    CHECK_PARAM(target->type == U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* simply copy y values from uyvy image to greyscale image */
    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        /* y values are in second channel, so must apply offset 1 to source */
        FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(1, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }
    else {
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        uint32 offset;
        /* for discontinuous images offset is applied to row table */
        /* therefore must correct the offset in case it's other than 0 */
        offset = 1 - source->offset;
        FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(offset, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }

    FINALLY(convert_uyvy16_to_grey8);
    RETURN();
}

/******************************************************************************/

result scale_uyvy16_2_uyvy16_x2(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == U8);
    CHECK_PARAM(target->type == U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 2);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == UYVY);
    CHECK_PARAM(target->width == 2 * source->width);
    CHECK_PARAM(target->height == 2 * source->height);

    /* read image in 4 byte chunks (unsigned long) and write out 4 times */
    {
        IMAGE_WITH_STEP_VARIABLES(uint32, uint32);
        uint32 offset_1, offset_2, offset_3;
        offset_1 = 1;
        offset_2 = target->stride / 2;
        offset_3 = offset_2 + 1;
        /* y values are in second channel, so must apply offset 1 to source */
        FOR_2_IMAGES_WITH_STEP(1, 1, 2, 2)
        {
            PIXEL_VALUE(target) =
            PIXEL_VALUE_PLUS(target, offset_1) =
            PIXEL_VALUE_PLUS(target, offset_2) =
            PIXEL_VALUE_PLUS(target, offset_3) =
            PIXEL_VALUE(source);
        }
    }

    /* alternative, slower code which preserves order of columns */
    /*
    u = src[srcPos++];
    y1 = src[srcPos++];
    v = src[srcPos++];
    y2 = src[srcPos++];

    dst[dstPos++] = u;
    dst[dstPos++] = y1;
    dst[dstPos++] = v;
    dst[dstPos++] = y1;
    dst[dstPos++] = u;
    dst[dstPos++] = y2;
    dst[dstPos++] = v;
    dst[dstPos++] = y2;

    dst[dstNxt++] = u;
    dst[dstNxt++] = y1;
    dst[dstNxt++] = v;
    dst[dstNxt++] = y1;
    dst[dstNxt++] = u;
    dst[dstNxt++] = y2;
    dst[dstNxt++] = v;
    dst[dstNxt++] = y2;
    */

    FINALLY(scale_uyvy16_2_uyvy16_x2);
    RETURN();
}

result scale_gray8_2_uyvy16_xn(pixel_image *src, pixel_image *dst, int scale)
{
    byte *src_data;
    byte *dst_data;
    int i, j, row, col, width, height, srcPos, dstPos, colStep, rowStep;
    int offsetCount;
    int offset[100];
    unsigned char value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    if (src->step != 1 || dst->step != 2) {
        return BAD_TYPE;
    }
    if (src->format != GREY || dst->format != UYVY) {
        return BAD_TYPE;
    }
    if (dst->width != scale * src->width || dst->height != scale * src->height) {
        return BAD_SIZE;
    }

    width = src->width;
    height = src->height;
    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;
    /* calculate the actual count of offset values */
    offsetCount = scale * scale;
    for (i = 0; i < scale; i++) {
        for (j = 0; j < scale; j++) {
            offset[j * scale + i] = j * scale * width * 2 + i * 2;
        }
    }
    colStep = scale * 2;
    rowStep = scale * scale * width * 2;

    memset(dst_data, 128, width * scale * height * scale * 2);
    /*row = 0;*/
    srcPos = 0;
    /*dstPos = 1;*/
    for (row = 0; row < height; row++) {
        /*col = 0;*/
        dstPos = row * rowStep + 1;
        for (col = 0; col < width; col++) {
            value = src_data[srcPos++];
            for (i = 0; i < offsetCount; i++) {
                dst_data[dstPos + offset[i]] = value;
            }
            dstPos += colStep;
            /*col++;*/
        }
        /*row++;*/
    }

    return SUCCESS;
}
