/*
   Copyright (C) 2009 Red Hat, Inc.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in
         the documentation and/or other materials provided with the
         distribution.
       * Neither the name of the copyright holder nor the names of its
         contributors may be used to endorse or promote products derived
         from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _H_JPEG_ENCODER
#define _H_JPEG_ENCODER

#include <spice/types.h>

typedef enum {
    JPEG_IMAGE_TYPE_INVALID,
    JPEG_IMAGE_TYPE_RGB16,
    /* in byte per color types, the notation is according to the order of the
       colors in the memory */
    JPEG_IMAGE_TYPE_RGB24,
    JPEG_IMAGE_TYPE_BGR24,
    JPEG_IMAGE_TYPE_BGRX32,
} JpegEncoderImageType;

typedef void* JpegEncoderContext;
typedef struct JpegEncoderUsrContext JpegEncoderUsrContext;

struct JpegEncoderUsrContext {
    int (*more_space)(JpegEncoderUsrContext *usr, uint8_t **io_ptr);
    int (*more_lines)(JpegEncoderUsrContext *usr, uint8_t **lines);
};

JpegEncoderContext* jpeg_encoder_create(JpegEncoderUsrContext *usr);
void jpeg_encoder_destroy(JpegEncoderContext *encoder);

/* returns the total size of the encoded data. Images must be supplied from the
   top line to the bottom */
int jpeg_encode(JpegEncoderContext *jpeg, int quality, JpegEncoderImageType type,
                int width, int height, uint8_t *lines, unsigned int num_lines, int stride,
                uint8_t *io_ptr, unsigned int num_io_bytes);
#endif
