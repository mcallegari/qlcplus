/*
  Q Light Controller
  shortcutwing.cpp

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

#include "shortcutwing.h"

/****************************************************************************
 * Shortcut wing specifics
 ****************************************************************************/
/*
The ENTTEC Shortcut wing produces packets that contain
WING_SHORTCUT_BUTTON_SIZE bytes of button data. Each bit in these bytes
signifies the state of one button. Thus, each byte contains 8 buttons (except
the first one only 4 buttons), and they are in reversed order:

WING_SHORTCUT_BYTE_BUTTON +
                07  06  05  04  03  02  01  00
-----------------------------------------------
bit 0 : buttons 07, 15, 23, 31, 39, 47, 55, N/A
bit 1 : buttons 06, 14, 22, 30, 38, 46, 54, N/A
bit 2 : buttons 05, 13, 21, 29, 37, 45, 53, N/A
bit 3 : buttons 04, 12, 20, 28, 36, 44, 52, N/A
bit 4 : buttons 03, 11, 19, 27, 35, 43, 51, 59
bit 5 : buttons 02, 10, 18, 26, 34, 42, 50, 58
bit 6 : buttons 01, 09, 17, 25, 33, 41, 49, 57
bit 7 : buttons 00, 08, 16, 24, 32, 40, 48, 56

As it can be seen from the table above, the byte order is also reversed:

WING_SHORTCUT_BYTE_BUTTON + 0: Buttons 56 - 59 (4 buttons)
WING_SHORTCUT_BYTE_BUTTON + 1: Buttons 48 - 55 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 2: Buttons 40 - 47 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 3: Buttons 32 - 39 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 4: Buttons 24 - 31 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 5: Buttons 16 - 23 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 6: Buttons 08 - 15 (8 buttons)
WING_SHORTCUT_BYTE_BUTTON + 7: Buttons 00 - 07 (8 buttons)
*/

#define WING_SHORTCUT_PAGE_UP   (1 << 3)
#define WING_SHORTCUT_PAGE_DOWN (1 << 2)

#define WING_SHORTCUT_BYTE_BUTTON 6
#define WING_SHORTCUT_BUTTON_SIZE 8

/** Should constitute up to 64 values (with the last 4 unused) */
#define WING_SHORTCUT_CHANNEL_COUNT 8 * WING_SHORTCUT_BUTTON_SIZE

#define WING_SHORTCUT_INPUT_VERSION 1
#define WING_SHORTCUT_INPUT_BYTE_VERSION 4
#define WING_SHORTCUT_INPUT_BYTE_PAGE 37

/****************************************************************************
 * Initialization
 ****************************************************************************/

ShortcutWing::ShortcutWing(QObject* parent, const QHostAddress& address,
                             const QByteArray& data)
    : Wing(parent, address, data)
{
    m_values = QByteArray(WING_SHORTCUT_CHANNEL_COUNT, 0);

    /* Take initial values from the first received datagram packet.
       The plugin hasn't yet connected to valueChanged() signal, so this
       won't cause any input events. */
    parseData(data);
    sendPageData();
}

ShortcutWing::~ShortcutWing()
{
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QString ShortcutWing::name() const
{
    QString name("Shortcut");
    name += QString(" ") + tr("at") + QString(" ");
    name += m_address.toString();

    return name;
}

/****************************************************************************
 * Input data
 ****************************************************************************/

void ShortcutWing::parseData(const QByteArray& data)
{
    /* Check if page buttons were pressed and act accordingly */
    applyPageButtons(data);

    /* Check that we can get all channels from the packet */
    int size = WING_SHORTCUT_BYTE_BUTTON + WING_SHORTCUT_BUTTON_SIZE;
    if (data.size() < size)
    {
        qWarning() << Q_FUNC_INFO << "Expected at least" << size
                   << "bytes for buttons but got only" << data.size();
        return;
    }

    /* Read the state of each button */
    for (int byte = size - 1; byte >= WING_SHORTCUT_BYTE_BUTTON; byte--)
    {
        /* Each byte has 8 button values as binary bits */
        for (int bit = 7; bit >= 0; bit--)
        {
            int key;
            uchar value;

            key = (size - byte - 1) * 8;
            key += (7 - bit);

            /* There's only 60 channels in a Shortcut Wing, but
               the data packet contains 64 values. So don't read
               the extra 4 bits. */
            if (key > 59)
                break;

            /* 0 = button down, 1 = button up */
            if ((data[byte] & (1 << bit)) == 0)
                value = UCHAR_MAX;
            else
                value = 0;

            setCacheValue(key, value);
        }
    }
}

void ShortcutWing::applyPageButtons(const QByteArray& data)
{
    /* Check that there's enough data for flags */
    if (data.size() < WING_BYTE_FLAGS + 1)
        return;

    if ((data[WING_BYTE_FLAGS] & WING_SHORTCUT_PAGE_UP) == 0)
    {
        nextPage();
        sendPageData();
    }
    else if ((data[WING_BYTE_FLAGS] & WING_SHORTCUT_PAGE_DOWN) == 0)
    {
        previousPage();
        sendPageData();
    }
}

void ShortcutWing::sendPageData()
{
    QByteArray sendData(42, char(0));
    sendData.replace(0, sizeof(WING_HEADER_INPUT), WING_HEADER_INPUT);
    sendData[WING_SHORTCUT_INPUT_BYTE_VERSION] = WING_SHORTCUT_INPUT_VERSION;
    sendData[WING_SHORTCUT_INPUT_BYTE_PAGE] = toBCD(page());

    QUdpSocket sock(this);
    sock.writeDatagram(sendData, address(), Wing::UDPPort);
}
