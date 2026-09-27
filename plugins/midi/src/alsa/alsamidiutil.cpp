/*
  Q Light Controller
  alsamidiutil.cpp

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

#include <alsa/asoundlib.h>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

#include "alsamidiutil.h"

QVariant AlsaMidiUtil::addressToVariant(const snd_seq_addr_t* addr)
{
    Q_ASSERT(addr != NULL);
    uint value = addr->client << 8;
    value = value | addr->port;
    return QVariant(value);
}

bool AlsaMidiUtil::variantToAddress(const QVariant& var, snd_seq_addr_t* addr)
{
    Q_ASSERT(addr != NULL);

    if (var.isValid() == false)
        return false;

    uint value = var.toUInt();
    addr->client = (value >> 8);
    addr->port = (value & 0xFF);
    return true;
}

QString AlsaMidiUtil::extractName(snd_seq_t* alsa, const snd_seq_addr_t* address)
{
    //qDebug() << Q_FUNC_INFO;
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(address != NULL);

    snd_seq_port_info_t* portInfo = NULL;
    snd_seq_port_info_alloca(&portInfo);
    int r = snd_seq_get_any_port_info(alsa, address->client, address->port, portInfo);
    if (r == 0)
    {
        qDebug() << "ALSA Port name: " << QString(snd_seq_port_info_get_name(portInfo));
        return QString(snd_seq_port_info_get_name(portInfo));
    }
    else
        return QString();
}

QString AlsaMidiUtil::extractUsbPath(snd_seq_t* alsa, int client)
{
    Q_ASSERT(alsa != NULL);

    snd_seq_client_info_t* clientInfo = NULL;
    snd_seq_client_info_alloca(&clientInfo);
    if (snd_seq_get_any_client_info(alsa, client, clientInfo) != 0)
        return QString();

    int card = snd_seq_client_info_get_card(clientInfo);
    if (card < 0)
        return QString();

    // Resolve /sys/class/sound/cardN symlink to its real device path.
    // symLinkTarget() follows one level; canonicalFilePath() resolves fully.
    QString sysfsPath = QString("/sys/class/sound/card%1").arg(card);
    QString resolved = QFileInfo(sysfsPath).canonicalFilePath();
    if (resolved.isEmpty())
        return QString();

    // Extract USB device path: matches "1-8.1.2" from paths like
    // /sys/devices/.../usb1/1-8/1-8.1/1-8.1.2/1-8.1.2:1.0/sound/cardN
    static const QRegularExpression re(R"(([\d]+-[\d.]+)(?=:[\d.]+/sound))");
    QRegularExpressionMatch m = re.match(resolved);
    if (!m.hasMatch())
        return QString();

    return m.captured(1);
}

QVariant AlsaMidiUtil::buildUid(snd_seq_t* alsa, const snd_seq_addr_t* addr)
{
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(addr != NULL);

    QString usbPath = extractUsbPath(alsa, addr->client);
    if (!usbPath.isEmpty())
    {
        // Include port so different ports on the same USB device get distinct UIDs
        QString uid = QString("%1:%2").arg(usbPath).arg(addr->port);
        qDebug() << "ALSA MIDI USB UID:" << uid;
        return QVariant(uid);
    }

    // Fall back to numeric client:port encoding for non-USB devices
    return addressToVariant(addr);
}
