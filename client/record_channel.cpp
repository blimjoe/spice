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
#include "red_client.h"
#include "audio_channels.h"
#include "audio_devices.h"

#define NUM_SAMPLES_MESSAGES 4


static uint32_t get_mm_time()
{
    return uint32_t(Platform::get_monolithic_time() / (1000 * 1000));
}

class RecordSamplesMessage: public RedChannel::OutMessage {
public:
    RecordSamplesMessage(RecordChannel& channel);
    virtual ~RecordSamplesMessage();

    virtual RedPeer::OutMessage& peer_message() { return *_massage;}
    virtual void release();

private:
    RecordChannel& _channel;
    RedPeer::OutMessage *_massage;
};

RecordSamplesMessage::RecordSamplesMessage(RecordChannel& channel)
    : _channel (channel)
    , _massage (new Message(REDC_RECORD_DATA, sizeof(RedcRecordPacket) + 4096))
{
}

RecordSamplesMessage::~RecordSamplesMessage()
{
    delete _massage;
}

void RecordSamplesMessage::release()
{
    _channel.release_message(this);
}

int RecordChannel::data_mode = RED_AUDIO_DATA_MODE_CELT_0_5_1;

class RecordHandler: public MessageHandlerImp<RecordChannel, REDC_RECORD_MESSAGES_END> {
public:
    RecordHandler(RecordChannel& channel)
        : MessageHandlerImp<RecordChannel, REDC_RECORD_MESSAGES_END>(channel) {}
};

RecordChannel::RecordChannel(RedClient& client, uint32_t id)
    : RedChannel(client, RED_CHANNEL_RECORD, id, new RecordHandler(*this))
    , _wave_recorder (NULL)
    , _mode (RED_AUDIO_DATA_MODE_INVALD)
    , _celt_mode (NULL)
    , _celt_encoder (NULL)
{
    for (int i = 0; i < NUM_SAMPLES_MESSAGES; i++) {
        _messages.push_front(new RecordSamplesMessage(*this));
    }

    RecordHandler* handler = static_cast<RecordHandler*>(get_message_handler());

    handler->set_handler(RED_MIGRATE, &RecordChannel::handle_migrate, 0);
    handler->set_handler(RED_SET_ACK, &RecordChannel::handle_set_ack, sizeof(RedSetAck));
    handler->set_handler(RED_PING, &RecordChannel::handle_ping, sizeof(RedPing));
    handler->set_handler(RED_WAIT_FOR_CHANNELS, &RecordChannel::handle_wait_for_channels,
                         sizeof(RedWaitForChannels));
    handler->set_handler(RED_DISCONNECTING, &RecordChannel::handle_disconnect,
                         sizeof(RedDisconnect));
    handler->set_handler(RED_NOTIFY, &RecordChannel::handle_notify, sizeof(RedNotify));

    handler->set_handler(RED_RECORD_START, &RecordChannel::handle_start, sizeof(RedRecordStart));

    set_capability(RED_RECORD_CAP_CELT_0_5_1);
}

RecordChannel::~RecordChannel(void)
{
    while (!_messages.empty()) {
        RecordSamplesMessage *mes;
        mes = *_messages.begin();
        _messages.pop_front();
        delete mes;
    }
    delete _wave_recorder;

    if (_celt_encoder) {
        celt051_encoder_destroy(_celt_encoder);
    }
    if (_celt_mode) {
        celt051_mode_destroy(_celt_mode);
    }
}

bool RecordChannel::abort(void)
{
    return (!_wave_recorder || _wave_recorder->abort()) && RedChannel::abort();
}

void RecordChannel::on_connect()
{
    Message* message = new Message(REDC_RECORD_MODE, sizeof(RedcRecordMode));
    RedcRecordMode *mode = (RedcRecordMode *)message->data();
    mode->time = get_mm_time();
    mode->mode = _mode = test_capability(RED_RECORD_CAP_CELT_0_5_1) ? RecordChannel::data_mode :
                                                                      RED_AUDIO_DATA_MODE_RAW;
    post_message(message);
}

void RecordChannel::send_start_mark()
{
    Message* message = new Message(REDC_RECORD_START_MARK, sizeof(RedcRecordStartMark));
    RedcRecordStartMark *start_mark = (RedcRecordStartMark *)message->data();
    start_mark->time = get_mm_time();
    post_message(message);
}

void RecordChannel::handle_start(RedPeer::InMessage* message)
{
    RecordHandler* handler = static_cast<RecordHandler*>(get_message_handler());
    RedRecordStart* start = (RedRecordStart*)message->data();

    handler->set_handler(RED_RECORD_START, NULL, 0);
    handler->set_handler(RED_RECORD_STOP, &RecordChannel::handle_stop, 0);
    ASSERT(!_wave_recorder && !_celt_mode && !_celt_encoder);

    // for now support only one setting
    if (start->format != RED_AUDIO_FMT_S16) {
        THROW("unexpected format");
    }

    int bits_per_sample = 16;
    try {
        _wave_recorder = Platform::create_recorder(*this, start->frequency,
                                                   bits_per_sample,
                                                   start->channels);
    } catch (...) {
        LOG_WARN("create recorder failed");
        return;
    }

    int frame_size = 256;
    int celt_mode_err;
    _frame_bytes = frame_size * bits_per_sample * start->channels / 8;
    if (!(_celt_mode = celt051_mode_create(start->frequency, start->channels, frame_size,
                                           &celt_mode_err))) {
        THROW("create celt mode failed %d", celt_mode_err);
    }

    if (!(_celt_encoder = celt051_encoder_create(_celt_mode))) {
        THROW("create celt encoder failed");
    }

    send_start_mark();
    _wave_recorder->start();
}

void RecordChannel::handle_stop(RedPeer::InMessage* message)
{
    RecordHandler* handler = static_cast<RecordHandler*>(get_message_handler());
    handler->set_handler(RED_RECORD_START, &RecordChannel::handle_start, sizeof(RedRecordStart));
    handler->set_handler(RED_RECORD_STOP, NULL, 0);
    if (!_wave_recorder) {
        return;
    }
    ASSERT(_celt_mode && _celt_encoder);
    _wave_recorder->stop();
    celt051_encoder_destroy(_celt_encoder);
    _celt_encoder = NULL;
    celt051_mode_destroy(_celt_mode);
    _celt_mode = NULL;
    delete _wave_recorder;
    _wave_recorder = NULL;
}

RecordSamplesMessage* RecordChannel::get_message()
{
    Lock lock(_messages_lock);
    if (_messages.empty()) {
        return NULL;
    }

    RecordSamplesMessage* ret = *_messages.begin();
    _messages.pop_front();
    return ret;
}

void RecordChannel::release_message(RecordSamplesMessage *message)
{
    Lock lock(_messages_lock);
    _messages.push_front(message);
}

void RecordChannel::add_evnet_sorce(EventsLoop::File& evnet_sorce)
{
    get_events_loop().add_file(evnet_sorce);
}

void RecordChannel::remove_evnet_sorce(EventsLoop::File& evnet_sorce)
{
    get_events_loop().remove_file(evnet_sorce);
}

void RecordChannel::add_evnet_sorce(EventsLoop::Trigger& evnet_sorce)
{
    get_events_loop().add_trigger(evnet_sorce);
}

void RecordChannel::remove_evnet_sorce(EventsLoop::Trigger& evnet_sorce)
{
    get_events_loop().remove_trigger(evnet_sorce);
}

#define FRAME_SIZE 256
#define CELT_BIT_RATE (64 * 1024)
#define CELT_COMPRESSED_FRAME_BYTES (FRAME_SIZE * CELT_BIT_RATE / 44100 / 8)

void RecordChannel::push_frame(uint8_t *frame)
{
    RecordSamplesMessage *message;
    ASSERT(_frame_bytes == FRAME_SIZE * 4);
    if (!(message = get_message())) {
        DBG(0, "blocked");
        return;
    }
    uint8_t celt_buf[CELT_COMPRESSED_FRAME_BYTES];
    int n;

    if (_mode == RED_AUDIO_DATA_MODE_CELT_0_5_1) {
        n = celt051_encode(_celt_encoder, (celt_int16_t *)frame, NULL, celt_buf,
                           CELT_COMPRESSED_FRAME_BYTES);
        if (n < 0) {
            THROW("celt encode failed");
        }
        frame = celt_buf;
    } else {
        n = _frame_bytes;
    }
    RedPeer::OutMessage& peer_message = message->peer_message();
    peer_message.resize(n + sizeof(RedcRecordPacket));
    RedcRecordPacket* packet = (RedcRecordPacket*)peer_message.data();
    packet->time = get_mm_time();
    memcpy(packet->data, frame, n);
    post_message(message);
}

class RecordFactory: public ChannelFactory {
public:
    RecordFactory() : ChannelFactory(RED_CHANNEL_RECORD) {}
    virtual RedChannel* construct(RedClient& client, uint32_t id)
    {
        return new RecordChannel(client, id);
    }
};

static RecordFactory factory;

ChannelFactory& RecordChannel::Factory()
{
    return factory;
}

