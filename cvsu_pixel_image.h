/**
 * @file cvsu_pixel_image.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Basic image types and operations for cvsu.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CVSU_PIXEL_IMAGE_H
#   define CVSU_PIXEL_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"

/**
 * Stores an image and its format description as an array of pixels.
 * Can be used also for referring to a region of interest (ROI) of an image.
 */
typedef struct pixel_image_t {
  /** Parent image for ROIs */
  struct pixel_image_t *parent;
  /** Pointer to pixel data array, data type specified by @see type */
  void *data;
  /** Pointers to row beginnings for handling discontinuous images */
  data_pointer *rows;
  /** Data type used to store pixel values in the @see data array */
  pixel_type type;
  /** Format used to store data for one pixel */
  pixel_format format;
  /** The x coordinate for the top left corner of the possible ROI */
  uint32 dx;
  /** The y coordinate for the top left corner of the possible ROI */
  uint32 dy;
  /** Width of the ROI (may be smaller than the width of the whole image) */
  uint32 width;
  /** Height of the ROI (may be smaller than the width of the whole image) */
  uint32 height;
  /** Channel offset for multi-channel images, if only one channel is used */
  uint32 offset;
  /** Number of array elements to the next column (the number of channels) */
  uint32 step;
  /** Number of elements to the same position on the next row */
  uint32 stride;
  /** Total number of elements in the @see data array */
  uint32 size;
} pixel_image;

/* TODO: Consider:
 * Storing also maximum (and minimum) value for pixels? Useful for
 * normalizing to 8-bit images when the desired maximum is something
 * else than maximum value present in the image. How to do this?
 * Need a pixel value type? Use a union? Might be feasible since the
 * possible pixel types are fixed.
 */

/**
 * Allocates a pixel image structure.
 */
pixel_image *pixel_image_alloc();

/**
 * Frees a pixel image structure allocated with @see pixel_image_alloc.
 */
void pixel_image_free
(
  pixel_image *ptr
);

/**
 * Allocates data for a pixel image.
 * @see pixel_image_destroy
 */
result pixel_image_create
(
  /** Pointer to the target struct where the image is stored */
  pixel_image *target,
  /** Data type used for storing the pixel values */
  pixel_type type,
  /** Pixel format for multi-channel images or GREY for greyscale */
  pixel_format format,
  /** Width of image in pixels */
  uint32 width,
  /** Height of image in pixels */
  uint32 height,
  /** Step between columns of pixels (amount of channels per pixel) */
  uint32 step,
  /** Stride between rows of pixels (distance to same column on next row) */
  uint32 stride
);

/**
 * Creates a pixel image from existing data.
 * Make sure the data is of correct type, as defined in parameters.
 * Also check whether the image data has to be deallocated or not.
 * @see pixel_image_destroy
 */
result pixel_image_create_from_data
(
  /** Pointer to target struct where image is stored */
  pixel_image *target,
  /** Pointer to existing image data */
  data_pointer data,
  /** Data type used for storing the pixel values */
  pixel_type type,
  /** Pixel format for multi-channel images or GREY for greyscale */
  pixel_format format,
  /** Width of image in pixels */
  uint32 width,
  /** Height of image in pixels */
  uint32 height,
  /** Step between columns of pixels (amount of channels per pixel) */
  uint32 step,
  /** Stride between rows of pixels (distance to same column on next row) */
  uint32 stride
);

/**
 * Deallocates the pixel image data.
 * @see pixel_image_create
 */
result pixel_image_destroy
(
  pixel_image *target
);

/**
 * Sets the image fields to null values.
 * @note Does not deallocate memory, use @see pixel_image_destroy first
 */
result pixel_image_nullify
(
  pixel_image *target
);

/**
* Everything that can be nullified should be able to tell if it's null.
*/
truth_value pixel_image_is_null
(
  pixel_image *target
);

/**
 * Creates a subset, or region of interest (ROI) of an image.
 * Useful for processing only a part of an image.
 * Does not copy the data, no need to deallocate with @see pixel_image_destroy.
 */
result pixel_image_create_roi
(
  pixel_image *target,
  pixel_image *source,
  uint32 dx,
  uint32 dy,
  uint32 width,
  uint32 height
);

/**
 * Transform the target image into a clone of the source image.
 * Only the structure is cloned, for copying content, use the copy function.
 * @see pixel_image_copy
 */
result pixel_image_clone
(
  /** The target image, that will become a clone of the source image. */
  pixel_image *target,
  /** The source image, that will be cloned to the target image. */
  const pixel_image *source
);

/**
 * Copy the contents of the source image into the target image.
 * The two images must have the same structure.
 * @see pixel_image_clone
 */
result pixel_image_copy
(
  /** The target image, to which the data contents will be copied. */
  pixel_image *target,
  /** The source image, from which the contents will be copied. */
  const pixel_image *source
);

/**
 * Resets the image contents to 0.
 */
result pixel_image_clear
(
  pixel_image *target
);

/**
 * Reads the pixel image from a pnm file.
 */
result pixel_image_read
(
  pixel_image *target,
  string source
);

/**
 * Writes the pixel image to a pnm file.
 */
result pixel_image_write
(
  pixel_image *source,
  string target,
  truth_value use_ascii
);

/**
 * Checks if the image is stored in a continuous block of memory, meaning
 * all pixels are stored one after another in memory. This information can be
 * used for optimizing some image operations.
 */
truth_value pixel_image_is_continuous
(
  const pixel_image *image
);

/**
 * Normalizes and scales image values to byte value range (0-255).
 * Generic function that calls specific functions depending on pixel type.
 * Specific functions are provided in the public interface because they
 * provide the possibility of giving the min and max values as parameters.
 * TODO: figure out a good way of giving the parameters through the generic
 * interface to get rid of the specific normalize functions.
 */
result normalize
(
  pixel_image *source,
  pixel_image *target
);

result normalize_byte
(
  pixel_image *source,
  pixel_image *target,
  byte min,
  byte max,
  byte mean
);

result normalize_char
(
  pixel_image *source,
  pixel_image *target,
  char min,
  char max,
  char mean
);

result normalize_word
(
  pixel_image *source,
  pixel_image *target,
  word min,
  word max,
  word mean
);

result normalize_long
(
  pixel_image *source,
  pixel_image *target,
  long min,
  long max,
  long mean
);

result normalize_float
(
  pixel_image *source,
  pixel_image *target,
  float min,
  float max,
  float mean
);

result normalize_double
(
  pixel_image *source,
  pixel_image *target,
  double min,
  double max,
  double mean
);

/**
 * Generic function for converting between different image types and copying the data
 */
result pixel_image_convert
(
  pixel_image *target,
  pixel_image *source
);

/**
 * Turns a one-channel greyscale image into a three-channel greyscale image
 * by cloning the channel; this way the image can be used as an RGB image.
 */
result convert_grey8_to_grey24
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Turns a one-channel greyscale image into a three-channel yuv image
 * by setting the color channels to value 128.
 */
result convert_grey8_to_yuv24
(
  const pixel_image *source,
  pixel_image *target
);


/**
 * Turns a three-channel RGB image into a one-channel greyscale image by
 * combining the color channels with formula 0.3 * R + 0.59 * G + 0.11 * B.
 */
result convert_rgb24_to_grey8
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Transforms RGB color values in a three-channel RGB image to YUV color space.
 */
result convert_rgb24_to_yuv24
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Transforms YUV color values in a three-channel YUV image to RGB color space.
 */
result convert_yuv24_to_rgb24
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Transforms YUV color values in a three-channel YUV image to RGB color space.
 */
result convert_yuv24_to_grey8
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Turns a two-channel uyvy image into a one-channel greyscale image by taking
 * the values from second channel (y channel) and discarding the first channel.
 */
result convert_uyvy16_to_grey8(
    const pixel_image *source,
    pixel_image *target
);

/**
 * Turns a two-channel uyvy image into a three-channel yuv image by taking
 * the values from second channel (y channel) and copying the values from
 * first channel in even and odd columns to second and third channels of
 * the target image.
 */
result convert_uyvy16_to_yuv24(
    const pixel_image *source,
    pixel_image *target
);

/**
 * Turns a two-channel yuyv image into a one-channel greyscale image by taking
 * the values from first channel (y channel) and discarding the second channel.
 */
result convert_yuyv16_to_grey8(
    const pixel_image *source,
    pixel_image *target
);

/**
 * Takes a 3-channel image and makes a 1-channel image selecting one channel.
 */
result pick_1_channel_from_3_channels
(
  /** source image */
  const pixel_image *source,
  /** target image */
  pixel_image *target,
  /** channel as offset 0-2 */
  uint32 channel
);

result pixel_image_replicate_pixels
(
  const pixel_image *source,
  pixel_image *target,
  uint32 n
);

/**
 * Scales the image down by factor of two without smoothing.
 * Takes every other pixel on every other row.
 * Supports using the same image for both source and target.
 * This way, the function can be used for successive scale_downs and scale_ups.
 */
result scale_down
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Scales the image up by factor of two without interpolation.
 * Each pixel becomes four pixels.
 * Supports using the same image for both source and target.
 * This way, the function can be used for successive scale_downs and scale_ups.
 */
result scale_up
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Safely get the pixel value at the given position.
 */
integral_value pixel_image_get_value
(
  pixel_image *target,
  uint32 x,
  uint32 y
);

/**
 * Fast but unsafe method for getting a pixel value.
 * Useful when multiple values are needed and the bound checking etc. can be
 * done once for the whole patch.
 */
integral_value pixel_image_unsafe_get_value
(
  void *data,
  uint32 x,
  uint32 y,
  uint32 step,
  uint32 stride,
  uint32 offset,
  pixel_type type
);

/**
 * Produces a valid rectangle for the given pixel_image. Takes into account
 * image dimensions and reduces the size of the region at the border. May
 * cause edge effects in some applications.
 */
image_rect pixel_image_create_rect
(
  /** the pixel_image used for calculations */
  pixel_image *target,
  /** left coordinate of the rectangle, can be negative */
  sint32 x,
  /** top coordinate of the rectangle, can be negative */
  sint32 y,
  /** width of the rectangle, signed to allow adjustment using x */
  sint32 dx,
  /** height of the rectangle, signed to allow adjustment using y */
  sint32 dy,
  /** channel offset for multi-channel images */
  uint32 offset
);

/**
 * Finds the minimum intensity value in the given image region.
 * The region may be adjusted near the border to ensure the region is valid.
 */
integral_value pixel_image_find_min_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
);

/**
* Finds the maximum intensity value in the given image region.
* The region may be adjusted near the border to ensure the region is valid.
*/
integral_value pixel_image_find_max_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
);

/**
* Calculates the intensity mean in the given image region.
* The region may be adjusted near the border to ensure the region is valid.
*/
integral_value pixel_image_calculate_mean_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
);

/**
* Calculates the intensity variance in the given image region.
* The region may be adjusted near the border to ensure the region is valid.
*/
integral_value pixel_image_calculate_variance_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_PIXEL_IMAGE_H */
