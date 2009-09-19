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

#ifndef _H_PLATFORM
#define _H_PLATFORM

#include "cursor.h"
#include "events_loop.h"

#define INVALID_TIMER (~TimerID(0))
typedef unsigned long TimerID;
typedef void (*timer_proc_t)(void* opaque, TimerID timer);

class WaveRecordAbstract;
class WavePlaybackAbstract;
class Icon;

class Monitor;
typedef std::list<Monitor*> MonitorsList;

class Platform {
public:
    static void init();
    static void wait_events();
    static bool process_events();
    static void wakeup();
    static void msleep(unsigned int millisec);
    static void yield();
    static void send_quit_request();
    static TimerID create_interval_timer(timer_proc_t proc, void* opaque);
    static bool activate_interval_timer(TimerID timer, unsigned int millisec);
    static bool deactivate_interval_timer(TimerID timer);
    static void destroy_interval_timer(TimerID timer);
    static uint64_t get_monolithic_time();
    static void get_temp_dir(std::string& path);

    static const MonitorsList& init_monitors();
    static void destroy_monitors();
    static bool is_monitors_pos_valid();

    enum ThreadPriority {
        PRIORITY_INVALID,
        PRIORITY_TIME_CRITICAL,
        PRIORITY_HIGH,
        PRIORITY_ABOVE_NORMAL,
        PRIORITY_NORMAL,
        PRIORITY_BELOW_NORMAL,
        PRIORITY_LOW,
        PRIORITY_IDLE,
    };

    static void set_thread_priority(void *thread, ThreadPriority priority);

    class RecordClinet;
    static WaveRecordAbstract* create_recorder(RecordClinet& client,
                                               uint32_t sampels_per_sec,
                                               uint32_t bits_per_sample,
                                               uint32_t channels);
    static WavePlaybackAbstract* create_player(uint32_t sampels_per_sec,
                                               uint32_t bits_per_sample,
                                               uint32_t channels);

    enum {
        SCROLL_LOCK_MODIFIER_SHIFT,
        NUM_LOCK_MODIFIER_SHIFT,
        CAPS_LOCK_MODIFIER_SHIFT,

        SCROLL_LOCK_MODIFIER = (1 << SCROLL_LOCK_MODIFIER_SHIFT),
        NUM_LOCK_MODIFIER = (1 << NUM_LOCK_MODIFIER_SHIFT),
        CAPS_LOCK_MODIFIER = (1 << CAPS_LOCK_MODIFIER_SHIFT),
    };

    static uint32_t get_keyboard_modifiers();
    static void set_keyboard_modifiers(uint32_t modifiers);

    static LocalCursor* create_local_cursor(CursorData* cursor_data);
    static LocalCursor* create_inactive_cursor();
    static LocalCursor* create_default_cursor();

    static Icon* load_icon(int id);

    class EventListener;
    static void set_event_listener(EventListener* listener);

    class DisplayModeListner;
    static void set_display_mode_listner(DisplayModeListner* listener);
};

class Platform::EventListener {
public:
    virtual ~EventListener() {}
    virtual void on_app_activated() = 0;
    virtual void on_app_deactivated() = 0;
    virtual void on_monitors_change() = 0;
};

class Platform::RecordClinet {
public:
    virtual ~RecordClinet() {}
    virtual void add_evnet_sorce(EventsLoop::File& evnet_sorce) = 0;
    virtual void remove_evnet_sorce(EventsLoop::File& evnet_sorce) = 0;
    virtual void add_evnet_sorce(EventsLoop::Trigger& evnet_sorce) = 0;
    virtual void remove_evnet_sorce(EventsLoop::Trigger& evnet_sorce) = 0;
    virtual void push_frame(uint8_t *frame) = 0;
};

class Platform::DisplayModeListner {
public:
    virtual ~DisplayModeListner() {}
    virtual void on_display_mode_change() = 0;
};

class NamedPipe {
public:
    typedef unsigned long ListenerRef;
    typedef unsigned long ConnectionRef;
    static const ConnectionRef INVALID_CONNECTION = ~0;

    class ConnectionInterface {
    public:
        ConnectionInterface() : _opaque (INVALID_CONNECTION) {}
        virtual ~ConnectionInterface() {}
        virtual void bind(ConnectionRef conn_ref) = 0;
        virtual void on_data() = 0;

    protected:
        ConnectionRef _opaque;
    };

    class ListenerInterface {
    public:
        virtual ~ListenerInterface() {}
        virtual ConnectionInterface &create() = 0;
    };

    static ListenerRef create(const char *name, ListenerInterface& listener_interface);
    static void destroy(ListenerRef listener_ref);
    static void destroy_connection(ConnectionRef conn_ref);
    static int32_t read(ConnectionRef conn_ref, uint8_t *buf, int32_t size);
    static int32_t write(ConnectionRef conn_ref, const uint8_t *buf, int32_t size);
};

#endif

