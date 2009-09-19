/*
   Copyright (C) 2009 Red Hat, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "resource.h"

#include "images/splash_image.c"

static const PixmapHeader splash_image = {
    (uint8_t *)_splash_image.pixel_data,
    _splash_image.width,
    _splash_image.height,
    _splash_image.width * 4,
};

#include "images/info_image.c"

static const PixmapHeader info_image = {
    (uint8_t *)_info_image.pixel_data,
    _info_image.width,
    _info_image.height,
    _info_image.width * 4,
};


typedef struct ResImage {
    int id;
    const PixmapHeader* image;
} ResImage;

static const ResImage res_image_map[] = {
    { SPLASH_IMAGE_RES_ID, &splash_image},
    { INFO_IMAGE_RES_ID, &info_image},
    {0, NULL},
};

const PixmapHeader *res_get_image(int id)
{
    const ResImage *now = res_image_map;
    for (; now->image; now++) {
        if (now->id == id) {
            return now->image;
        }
    }
    return NULL;
}

#include "images/red_icon.c"

static const IconHeader red_icon = {
    _red_icon.width,
    _red_icon.height,
    (uint8_t *)_red_icon.pixmap,
    (uint8_t *)_red_icon.mask,
};

typedef struct ResIcon {
    int id;
    const IconHeader* icon;
} ResIcon;

static const ResIcon res_icon_map[] = {
    { RED_ICON_RES_ID, &red_icon},
    {0, NULL},
};

const IconHeader *res_get_icon(int id)
{
    const ResIcon *now = res_icon_map;
    for (; now->icon; now++) {
        if (now->id == id) {
            return now->icon;
        }
    }
    return NULL;
}

