/*
  Q Light Controller
  playbackwing.cpp

  Copyright (c) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QHostAddress>
#include <QMessageBox>
#include <QByteArray>
#include <QUdpSocket>
#include <QString>
#include <QDebug>

#include "playbackwing.h"


/****************************************************************************
 * Playback wing specifics
 ****************************************************************************/
/*
The ENTTEC Playback wing produces packets that contain
WING_PLAYBACK_BUTTON_SIZE bytes of button data. Each bit in the button bytes
signifies the state of one button. Thus, each byte contains 8 buttons, and
they are in reversed order, with some of them (27, 28, 29, 22, 23, 24, 25, 26)
mixed up in a very weird way:

WING_PLAYBACK_BYTE_BUTTON +
                04  03  02  01  00
----------------------------------
bit 0 : buttons 07, 15, 28, 31, 39
bit 1 : buttons 06, 14, 27, 30, 38
bit 2 : buttons 05, 13, 21, 26, 37
bit 3 : buttons 04, 12, 20, 25, 36
bit 4 : buttons 03, 11, 19, 24, 35
bit 5 : buttons 02, 10, 18, 23, 34
bit 6 : buttons 01, 09, 17, 22, 33
bit 7 : buttons 00, 08, 16, 29, 32

As it can be seen from the table above, the byte order is also reversed:

WING_PLAYBACK_BYTE_BUTTON + 0: Buttons 32 - 39 (8 buttons)
WING_PLAYBACK_BYTE_BUTTON + 1: Buttons 24 - 31 (8 buttons)
WING_PLAYBACK_BYTE_BUTTON + 2: Buttons 16 - 23 (8 buttons)
WING_PLAYBACK_BYTE_BUTTON + 3: Buttons 08 - 15 (8 buttons)
WING_PLAYBACK_BYTE_BUTTON + 4: Buttons 00 - 07 (8 buttons)

The Playback Wing contains also WING_PLAYBACK_SLIDER_SIZE bytes of slider data,
where each byte contains an 8bit char value signifying the slider value. Slider
bytes are not reversed:

WING_PLAYBACK_BYTE_SLIDER + 0: Slider 01 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 1: Slider 02 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 2: Slider 03 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 3: Slider 04 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 4: Slider 05 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 5: Slider 06 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 6: Slider 07 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 7: Slider 08 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 8: Slider 09 (0-255)
WING_PLAYBACK_BYTE_SLIDER + 9: Slider 10 (0-255)
*/

#define WING_PLAYBACK_BYTE_BUTTON 7 /* Bytes 7-11 are for buttons */
#define WING_PLAYBACK_BUTTON_SIZE 5 /* 5 bytes of button states */

#define WING_PLAYBACK_BYTE_SLIDER 15 /* Bytes 15-25 are for sliders */
#define WING_PLAYBACK_SLIDER_SIZE 10 /* 10 slider values in all */

#define WING_PLAYBACK_PACKET_SIZE (WING_PLAYBACK_BYTE_SLIDER + WING_PLAYBACK_SLIDER_SIZE) /* 25, total packet size */

#define WING_PLAYBACK_BYTE_EXTRA_BUTTONS 6
#define WING_PLAYBACK_BIT_PAGEUP   (1 << 7)
#define WING_PLAYBACK_BIT_PAGEDOWN (1 << 6)
#define WING_PLAYBACK_BIT_BACK     (1 << 5)
#define WING_PLAYBACK_BIT_GO       (1 << 4)

/** Should constitute up to 50 channels */
#define WING_PLAYBACK_CHANNEL_COUNT 8 * WING_PLAYBACK_BUTTON_SIZE \
					+ WING_PLAYBACK_SLIDER_SIZE

/** number of extra buttons (go,back, pageup, pagedown) */
#define WING_PLAYBACK_EXTRA_BUTTONS_COUNT 4
#define WING_PLAYBACK_BUTTON_GO 50
#define WING_PLAYBACK_BUTTON_BACK 51
#define WING_PLAYBACK_BUTTON_PAGEDOWN 52
#define WING_PLAYBACK_BUTTON_PAGEUP 53


#define WING_PLAYBACK_INPUT_VERSION 1
#define WING_PLAYBACK_INPUT_BYTE_VERSION 4
#define WING_PLAYBACK_INPUT_BYTE_PAGE 37

/****************************************************************************
 * Initialization
 ****************************************************************************/

PlaybackWing::PlaybackWing(QObject* parent, const QHostAddress& address,
                           const QByteArray& data)
    : Wing(parent, address, data)
{
    m_values = QByteArray((WING_PLAYBACK_CHANNEL_COUNT) + (WING_PLAYBACK_EXTRA_BUTTONS_COUNT), 0);

    /* Playback wing keys seem to be in a somewhat weird order */
    m_channelMap[0] = 7 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[1] = 6 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[2] = 5 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[3] = 4 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[4] = 3 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[5] = 2 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[6] = 1 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[7] = 0 + WING_PLAYBACK_SLIDER_SIZE;

    m_channelMap[8] = 15 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[9] = 14 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[10] = 13 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[11] = 12 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[12] = 11 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[13] = 10 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[14] = 9 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[15] = 8 + WING_PLAYBACK_SLIDER_SIZE;

    /* Weird order here */
    m_channelMap[16] = 28 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[17] = 27 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[18] = 21 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[19] = 20 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[20] = 19 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[21] = 18 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[22] = 17 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[23] = 16 + WING_PLAYBACK_SLIDER_SIZE;

    /* Weird order also here */
    m_channelMap[24] = 31 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[25] = 30 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[26] = 26 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[27] = 25 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[28] = 24 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[29] = 23 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[30] = 22 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[31] = 29 + WING_PLAYBACK_SLIDER_SIZE;

    m_channelMap[32] = 39 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[33] = 38 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[34] = 37 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[35] = 36 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[36] = 35 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[37] = 34 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[38] = 33 + WING_PLAYBACK_SLIDER_SIZE;
    m_channelMap[39] = 32 + WING_PLAYBACK_SLIDER_SIZE;

    /* Take initial values from the first received datagram packet.
       The plugin hasn't yet connected to valueChanged() signal, so this
       won't cause any input events. */
    parseData(data);
    sendPageData();
}

PlaybackWing::~PlaybackWing()
{
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QString PlaybackWing::name() const
{
    QString name("Playback");
    name += QString(" ") + tr("at") + QString(" ");
    name += m_address.toString();

    return name;
}

/****************************************************************************
 * Input data
 ****************************************************************************/

void PlaybackWing::parseData(const QByteArray& data)
{
    if (data.size() < WING_PLAYBACK_PACKET_SIZE )
    {
        qWarning() << Q_FUNC_INFO << "Expected at least" << WING_PLAYBACK_PACKET_SIZE
                   << "bytes for buttons but got only" << data.size();
        return;
    }

    /* Check if page buttons were pressed and act accordingly */
    applyExtraButtons(data);

    int size = WING_PLAYBACK_BYTE_BUTTON + WING_PLAYBACK_BUTTON_SIZE;

    /* Read the state of each button */
    for (int byte = size - 1; byte >= WING_PLAYBACK_BYTE_BUTTON; byte--)
    {
        /* Each byte has 8 button values as binary bits */
        for (int bit = 7; bit >= 0; bit--)
        {
            char value;

            /* Calculate the key number, which is 10-49, since
               sliders are mapped to 0-9. */
            int key = (size - byte - 1) * 8;
            key += bit;

            /* 0 = button down, 1 = button up */
            if ((data[byte] & (1 << bit)) == 0)
                value = UCHAR_MAX;
            else
                value = 0;

            /* Get the correct channel number for each key. */
            setCacheValue(m_channelMap[key], value);
        }
    }

    size = WING_PLAYBACK_BYTE_SLIDER + WING_PLAYBACK_SLIDER_SIZE;

    /* Read the state of each slider. Each value takes all 8 bits. */
    for (int slider = 0; slider < WING_PLAYBACK_SLIDER_SIZE; slider++)
    {
        char value = data[WING_PLAYBACK_BYTE_SLIDER + slider];
        setCacheValue(slider, value);
    }
}

void PlaybackWing::applyExtraButtons(const QByteArray& data)
{
    /* Check that there's enough data for flags */
    if (data.size() < WING_PLAYBACK_PACKET_SIZE )
        return;

    // WING_PLAYBACK_BIT_PAGEUP
    if (!(data[WING_PLAYBACK_BYTE_EXTRA_BUTTONS] & WING_PLAYBACK_BIT_PAGEUP))
    {
        nextPage();
        sendPageData();
        setCacheValue(WING_PLAYBACK_BUTTON_PAGEUP, UCHAR_MAX);
    }
    else
    {
        setCacheValue(WING_PLAYBACK_BUTTON_PAGEUP, 0);
    }

    // WING_PLAYBACK_BIT_PAGEDOWN
    if (!(data[WING_PLAYBACK_BYTE_EXTRA_BUTTONS] & WING_PLAYBACK_BIT_PAGEDOWN))
    {
        previousPage();
        sendPageData();
        setCacheValue(WING_PLAYBACK_BUTTON_PAGEDOWN, UCHAR_MAX);
    }
    else
    {
        setCacheValue(WING_PLAYBACK_BUTTON_PAGEDOWN, 0);
    }

    // WING_PLAYBACK_BIT_PAGEGO
    if (!(data[WING_PLAYBACK_BYTE_EXTRA_BUTTONS] & WING_PLAYBACK_BIT_GO))
    {
        setCacheValue(WING_PLAYBACK_BUTTON_GO, UCHAR_MAX);
    }
    else
    {
        setCacheValue(WING_PLAYBACK_BUTTON_GO, 0);
    }

    // WING_PLAYBACK_BIT_PAGEBACK
    if (!(data[WING_PLAYBACK_BYTE_EXTRA_BUTTONS] & WING_PLAYBACK_BIT_BACK))
    {
        setCacheValue(WING_PLAYBACK_BUTTON_BACK, UCHAR_MAX);
    }
    else
    {
        setCacheValue(WING_PLAYBACK_BUTTON_BACK, 0);
    }
}

void PlaybackWing::sendPageData()
{
    QByteArray sendData(42, char(0));
    sendData.replace(0, sizeof(WING_HEADER_INPUT), WING_HEADER_INPUT);
    sendData[WING_PLAYBACK_INPUT_BYTE_VERSION] = WING_PLAYBACK_INPUT_VERSION;
    sendData[WING_PLAYBACK_INPUT_BYTE_PAGE] = toBCD(page());

    QUdpSocket sock(this);
    sock.writeDatagram(sendData, address(), Wing::UDPPort);
}
