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

#ifndef _H_RED_PIXMAP
#define _H_RED_PIXMAP

#include "red_drawable.h"

class RedPixmap: public RedDrawable {
public:
    enum Format {
        ARGB32,
        RGB32,
        A1,
    };

    RedPixmap(int width, int height, Format format, bool top_bottom,
              rgb32_t* pallete);
    virtual ~RedPixmap();

    virtual Point get_size() { Point pt = {_width, _height}; return pt;}

    int get_width() { return _width;}
    int get_height() { return _height;}
    int get_stride() { return _stride;}
    uint8_t* get_data() { return _data;}
    bool is_big_endian_bits();

protected:
    Format _format;
    int _width;
    int _height;
    int _stride;
    bool _top_bottom;
    uint8_t* _data;
};

#endif

