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
#include "cursor.h"
#include "utils.h"
#include "debug.h"

CursorData::CursorData(RedCursor& cursor, int data_size)
    : _atomic (1)
    , _header (cursor.header)
    , _data (NULL)
    , _opaque (NULL)
    , _local_cursor (NULL)
{
    int expected_size = 0;

    switch (cursor.header.type) {
    case CURSOR_TYPE_ALPHA:
        expected_size = (_header.width << 2) * _header.height;
        break;
    case CURSOR_TYPE_MONO:
        expected_size = (ALIGN(_header.width, 8) >> 2) * _header.height;
        break;
    case CURSOR_TYPE_COLOR4:
        expected_size = (ALIGN(_header.width, 2) >> 1) * _header.height;
        expected_size += (ALIGN(_header.width, 8) >> 3) * _header.height;
        expected_size += 16 * sizeof(uint32_t);
        break;
    case CURSOR_TYPE_COLOR8:
        expected_size = _header.width * _header.height;
        expected_size += (ALIGN(_header.width, 8) >> 3) * _header.height;
        expected_size += 256 * sizeof(uint32_t);
        break;
    case CURSOR_TYPE_COLOR16:
        expected_size = (_header.width << 1) * _header.height;
        expected_size += (ALIGN(_header.width, 8) >> 3) * _header.height;
        break;
    case CURSOR_TYPE_COLOR24:
        expected_size = (_header.width * 3) * _header.height;
        expected_size += (ALIGN(_header.width, 8) >> 3) * _header.height;
        break;
    case CURSOR_TYPE_COLOR32:
        expected_size = (_header.width << 2) * _header.height;
        expected_size += (ALIGN(_header.width, 8) >> 3) * _header.height;
        break;
    }

    if (data_size < expected_size) {
        THROW("access violation 0x%lx %u", (unsigned long)cursor.data, expected_size);
    }
    _data = new uint8_t[expected_size];
    memcpy(_data, cursor.data, expected_size);
}

void CursorData::set_local(LocalCursor* local_cursor)
{
    ASSERT(!_local_cursor);
    if (local_cursor) {
        _local_cursor = local_cursor->ref();
    }
}

CursorData::~CursorData()
{
    if (_local_cursor) {
        _local_cursor->unref();
    }
    delete _opaque;
    delete[] _data;
}

int LocalCursor::get_size_bits(const CursorHeader& header, int& size)
{
    switch (header.type) {
    case CURSOR_TYPE_ALPHA:
    case CURSOR_TYPE_COLOR32:
        size = (header.width << 2) * header.height;
        return 32;
    case CURSOR_TYPE_MONO:
        size = (ALIGN(header.width, 8) >> 3) * header.height;
        return 1;
    case CURSOR_TYPE_COLOR4:
        size = (ALIGN(header.width, 2) >> 1) * header.height;
        return 4;
    case CURSOR_TYPE_COLOR8:
        size = header.width * header.height;
        return 8;
    case CURSOR_TYPE_COLOR16:
        size = (header.width << 1) * header.height;
        return 16;
    case CURSOR_TYPE_COLOR24:
        size = (header.width * 3) * header.height;
        return 24;
    default:
        return 0;
    }
}

