/*
  Q Light Controller
  programwing.cpp

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
#include <QString>
#include <QDebug>

#include "programwing.h"

/****************************************************************************
 * Program wing specifics
 ****************************************************************************/

#define WING_PROGRAM_BYTE_BUTTON 6
#define WING_PROGRAM_BUTTON_SIZE 9

#define WING_PROGRAM_BYTE_ENCODER 25
#define WING_PROGRAM_ENCODER_SIZE 3

/** Should constitute up to 75 channels (with some unused ones in the middle) */
#define WING_PROGRAM_CHANNEL_COUNT (8 * WING_PROGRAM_BUTTON_SIZE) \
					+ WING_PROGRAM_ENCODER_SIZE

/****************************************************************************
 * Initialization
 ****************************************************************************/

ProgramWing::ProgramWing(QObject* parent, const QHostAddress& address,
                         const QByteArray& data)
    : Wing(parent, address, data)
{
    m_values = QByteArray(WING_PROGRAM_CHANNEL_COUNT, 0);

    m_channelMap[0] = 6;
    m_channelMap[1] = 5;
    m_channelMap[2] = WING_INVALID_CHANNEL;
    m_channelMap[3] = 4;
    m_channelMap[4] = 3;
    m_channelMap[5] = 2;
    m_channelMap[6] = 1;
    m_channelMap[7] = 0;

    m_channelMap[8] = 14;
    m_channelMap[9] = 13;
    m_channelMap[10] = 12;
    m_channelMap[11] = 11;
    m_channelMap[12] = 10;
    m_channelMap[13] = 9;
    m_channelMap[14] = 8;
    m_channelMap[15] = 7;

    m_channelMap[16] = 17;
    m_channelMap[17] = 16;
    m_channelMap[18] = WING_INVALID_CHANNEL;
    m_channelMap[19] = WING_INVALID_CHANNEL;
    m_channelMap[20] = WING_INVALID_CHANNEL;
    m_channelMap[21] = WING_INVALID_CHANNEL;
    m_channelMap[22] = WING_INVALID_CHANNEL;
    m_channelMap[23] = 15;

    m_channelMap[24] = 25;
    m_channelMap[25] = 24;
    m_channelMap[26] = 23;
    m_channelMap[27] = 22;
    m_channelMap[28] = 21;
    m_channelMap[29] = 20;
    m_channelMap[30] = 19;
    m_channelMap[31] = 18;

    m_channelMap[32] = 38;
    m_channelMap[33] = 37;
    m_channelMap[34] = 36;
    m_channelMap[35] = 30;
    m_channelMap[36] = 29;
    m_channelMap[37] = 28;
    m_channelMap[38] = 27;
    m_channelMap[39] = 26;

    m_channelMap[40] = 56;
    m_channelMap[41] = 50;
    m_channelMap[42] = 49;
    m_channelMap[43] = 48;
    m_channelMap[44] = 47;
    m_channelMap[45] = 46;
    m_channelMap[46] = 40;
    m_channelMap[47] = 39;

    m_channelMap[48] = 57;
    m_channelMap[49] = 35;
    m_channelMap[50] = 34;
    m_channelMap[51] = 45;
    m_channelMap[52] = 44;
    m_channelMap[53] = 55;
    m_channelMap[54] = 54;
    m_channelMap[55] = 58;

    m_channelMap[56] = 32;
    m_channelMap[57] = 31;
    m_channelMap[58] = 43;
    m_channelMap[59] = 42;
    m_channelMap[60] = 41;
    m_channelMap[61] = 53;
    m_channelMap[62] = 52;
    m_channelMap[63] = 51;

    m_channelMap[64] = WING_INVALID_CHANNEL;
    m_channelMap[65] = 64;
    m_channelMap[66] = 63;
    m_channelMap[67] = 62;
    m_channelMap[68] = 61;
    m_channelMap[69] = 60;
    m_channelMap[70] = 59;
    m_channelMap[71] = 33;

    m_channelMap[72] = 65;
    m_channelMap[73] = 66;
    m_channelMap[74] = 67;

    /* Take initial values from the first received datagram packet.
       The plugin hasn't yet connected to valueChanged() signal, so this
       won't cause any input events. */
    parseData(data);
}

ProgramWing::~ProgramWing()
{
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QString ProgramWing::name() const
{
    QString name("Program");
    name += QString(" ") + tr("at") + QString(" ");
    name += m_address.toString();

    return name;
}

/****************************************************************************
 * Input data
 ****************************************************************************/

void ProgramWing::parseData(const QByteArray& data)
{
    uchar value;
    int size;
    int byte;

    /* Check that we can get all buttons from the packet */
    size = WING_PROGRAM_BYTE_BUTTON + WING_PROGRAM_BUTTON_SIZE;
    if (data.size() < size)
    {
        qWarning() << Q_FUNC_INFO << "Expected at least" << size
                   << "bytes for buttons but got only" << data.size();
        return;
    }

    /* Read the state of each button */
    for (byte = size - 1; byte >= WING_PROGRAM_BYTE_BUTTON; byte--)
    {
        /* Each byte has 8 button values as binary bits */
        for (int bit = 7; bit >= 0; bit--)
        {
            int key;

            /* Calculate the key number, which is 0-64 */
            key = (size - byte - 1) * 8;
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

    /* Check that we can get all sliders from the packet */
    size = WING_PROGRAM_BYTE_ENCODER + WING_PROGRAM_ENCODER_SIZE;
    if (data.size() < size)
    {
        qWarning() << "Expected at least" << size
        << "bytes for sliders but got only" << data.size();
        return;
    }

    /* Read the direction of each encoder. 255 = CW, 1 = CCW, 0 = NOP. */
    for (int encoder = 0; encoder < WING_PROGRAM_ENCODER_SIZE; encoder++)
    {
        int channel = (WING_PROGRAM_CHANNEL_COUNT -
                       WING_PROGRAM_ENCODER_SIZE) + encoder;
        unsigned char cvalue = cacheValue(m_channelMap[channel]);

        value = data[WING_PROGRAM_BYTE_ENCODER + encoder];
        if (value == 255)
            setCacheValue(m_channelMap[channel], ++cvalue);
        else if (value == 1)
            setCacheValue(m_channelMap[channel], --cvalue);
    }
}
