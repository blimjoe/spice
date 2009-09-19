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

#ifndef _H_WINPLATFORM
#define _H_WINPLATFORM

#include "icon.h"

class EventOwner {
public:
    EventOwner() : _event_handle (0) {}
    HANDLE const get_event_handle() { return _event_handle;}
    virtual void on_event() = 0;

protected:
    HANDLE _event_handle;
};

class WinPlatform {
public:
    static void add_event(EventOwner& event_owner);
    static void remove_event(EventOwner& event_owner);
};

class WinIcon: public Icon {
public:
    WinIcon(HICON icon) : _icon (icon) {}

    HICON get_handle() {return _icon;}

private:
    HICON _icon;
};

#endif

