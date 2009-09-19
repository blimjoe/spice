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
#include <stdint.h>
#include "red_gdi_canvas.h"
#include "utils.h"
#include "debug.h"
#include "region.h"
#include "red_pixmap_gdi.h"

GDICanvas::GDICanvas(PixmapCache& pixmap_cache, PaletteCache& palette_cache,
                     GlzDecoderWindow &glz_decoder_window)
    : Canvas (pixmap_cache, palette_cache, glz_decoder_window)
    , _canvas (NULL)
    , _pixmap (0)
{
}

GDICanvas::~GDICanvas()
{
    destroy();
}

void GDICanvas::destroy()
{
    if (_canvas) {
        _canvas = NULL;
    }
    destroy_pixmap();
}

void GDICanvas::clear()
{
    if (_canvas) {
        gdi_canvas_clear(_canvas);
    }
}

void GDICanvas::destroy_pixmap()
{
    delete _pixmap;
    _pixmap = NULL;
}

void GDICanvas::create_pixmap(int width, int height)
{
    _pixmap = new RedPixmapGdi(width, height, RedPixmap::RGB32, true, NULL);
}

void GDICanvas::copy_pixels(const QRegion& region, RedDrawable& dest_dc)
{
    for (int i = 0; i < (int)region.num_rects; i++) {
        Rect* r = &region.rects[i];
        dest_dc.copy_pixels(*_pixmap, r->left, r->top, *r);
    }
}

void GDICanvas::copy_pixels(const QRegion& region, RedDrawable* dest_dc, const PixmapHeader* pixmap)
{
    copy_pixels(region, *dest_dc);
}

void GDICanvas::set_mode(int width, int height, int depth)
{
    destroy();
    create_pixmap(width, height);
    if (!(_canvas = gdi_canvas_create(_pixmap->get_dc(),
                                      &_pixmap->get_mutex(),
                                      depth, &pixmap_cache(), bits_cache_put,
                                      bits_cache_get, &palette_cache(),
                                      palette_cache_put, palette_cache_get,
                                      palette_cache_release,
                                      &glz_decoder(),
                                      glz_decode))) {
        THROW("create canvas failed");
    }
}

void GDICanvas::set_access_params(ADDRESS delta, unsigned long base, unsigned long max)
{
    gdi_canvas_set_access_params(_canvas, delta, base, max);
}

void GDICanvas::draw_fill(Rect *bbox, Clip *clip, Fill *fill)
{
    gdi_canvas_draw_fill(_canvas, bbox, clip, fill);
}

void GDICanvas::draw_text(Rect *bbox, Clip *clip, Text *text)
{
    gdi_canvas_draw_text(_canvas, bbox, clip, text);
}

void GDICanvas::draw_opaque(Rect *bbox, Clip *clip, Opaque *opaque)
{
    gdi_canvas_draw_opaque(_canvas, bbox, clip, opaque);
}

void GDICanvas::draw_copy(Rect *bbox, Clip *clip, Copy *copy)
{
    gdi_canvas_draw_copy(_canvas, bbox, clip, copy);
}

void GDICanvas::draw_transparent(Rect *bbox, Clip *clip, Transparent* transparent)
{
    gdi_canvas_draw_transparent(_canvas, bbox, clip, transparent);
}

void GDICanvas::draw_alpha_blend(Rect *bbox, Clip *clip, AlphaBlnd* alpha_blend)
{
    gdi_canvas_draw_alpha_blend(_canvas, bbox, clip, alpha_blend);
}

void GDICanvas::copy_bits(Rect *bbox, Clip *clip, Point *src_pos)
{
    gdi_canvas_copy_bits(_canvas, bbox, clip, src_pos);
}

void GDICanvas::draw_blend(Rect *bbox, Clip *clip, Blend *blend)
{
    gdi_canvas_draw_blend(_canvas, bbox, clip, blend);
}

void GDICanvas::draw_blackness(Rect *bbox, Clip *clip, Blackness *blackness)
{
    gdi_canvas_draw_blackness(_canvas, bbox, clip, blackness);
}

void GDICanvas::draw_whiteness(Rect *bbox, Clip *clip, Whiteness *whiteness)
{
    gdi_canvas_draw_whiteness(_canvas, bbox, clip, whiteness);
}

void GDICanvas::draw_invers(Rect *bbox, Clip *clip, Invers *invers)
{
    gdi_canvas_draw_invers(_canvas, bbox, clip, invers);
}

void GDICanvas::draw_rop3(Rect *bbox, Clip *clip, Rop3 *rop3)
{
    gdi_canvas_draw_rop3(_canvas, bbox, clip, rop3);
}

void GDICanvas::draw_stroke(Rect *bbox, Clip *clip, Stroke *stroke)
{
    gdi_canvas_draw_stroke(_canvas, bbox, clip, stroke);
}

void GDICanvas::put_image(HDC dc, const PixmapHeader& image, const Rect& dest, const QRegion* clip)
{
    gdi_canvas_put_image(_canvas, dc, &dest, image.data, image.width, image.height, image.stride,
                         clip);
}

CanvasType GDICanvas::get_pixmap_type()
{
    return CANVAS_TYPE_GDI;
}

