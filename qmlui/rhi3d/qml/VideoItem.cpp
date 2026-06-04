/*
  Q Light Controller Plus
  VideoItem.cpp

  Copyright (c) Massimo Callegari

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

#include "qml/VideoItem.h"

#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>
#include <QtCore/QMutexLocker>
#include <QtMath>

VideoItem::VideoItem(QObject *parent)
    : MeshItem(parent)
{
    m_videoSink = new QVideoSink(this);
    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &VideoItem::onVideoFrameChanged);
}

void VideoItem::setPlayer(QMediaPlayer *player)
{
    if (m_player == player)
        return;
    if (m_player)
    {
        disconnect(m_player, nullptr, this, nullptr);
        m_player->setVideoOutput(nullptr);
    }
    m_player = player;
    if (m_player)
    {
        connect(m_player, &QObject::destroyed, this, [this]()
        {
            m_player = nullptr;
            emit playerChanged();
        });
        m_player->setVideoOutput(m_videoSink);
    }
    emit playerChanged();
}

void VideoItem::setLedColumns(int columns)
{
    columns = qMax(0, columns);
    if (m_ledColumns == columns)
        return;
    m_ledColumns = columns;
    {
        QMutexLocker locker(&m_frameMutex);
        if (!m_latestFrame.isNull())
            m_frameDirty = true;
    }
    emit ledColumnsChanged();
    notifyParent();
}

void VideoItem::setLedRows(int rows)
{
    rows = qMax(0, rows);
    if (m_ledRows == rows)
        return;
    m_ledRows = rows;
    {
        QMutexLocker locker(&m_frameMutex);
        if (!m_latestFrame.isNull())
            m_frameDirty = true;
    }
    emit ledRowsChanged();
    notifyParent();
}

bool VideoItem::takeFrame(QImage &frame)
{
    QImage localFrame;
    int columns = 0;
    int rows = 0;
    {
        QMutexLocker locker(&m_frameMutex);
        if (!m_frameDirty || m_latestFrame.isNull())
            return false;
        localFrame = m_latestFrame;
        columns = m_ledColumns;
        rows = m_ledRows;
        m_frameDirty = false;
    }
    frame = applyLedEffect(localFrame, columns, rows);
    return true;
}

void VideoItem::onVideoFrameChanged(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return;
    QImage image = frame.toImage();
    if (image.isNull())
        return;
    if (image.format() != QImage::Format_RGBX8888)
        image = image.convertToFormat(QImage::Format_RGBX8888);
    image = image.convertToFormat(QImage::Format_RGBA8888);
    {
        QMutexLocker locker(&m_frameMutex);
        m_latestFrame = image;
        m_frameDirty = true;
    }
    notifyParent();
}

QImage VideoItem::applyLedEffect(const QImage &image, int columns, int rows) const
{
    if (image.isNull())
        return image;
    int cols = columns;
    int rowCount = rows;
    if (cols <= 0 && rowCount <= 0)
        return image;
    if (cols <= 0)
    {
        const float aspect = float(image.width()) / float(qMax(1, image.height()));
        cols = qMax(1, int(qRound(float(rowCount) * aspect)));
    }
    if (rowCount <= 0)
    {
        const float aspect = float(image.height()) / float(qMax(1, image.width()));
        rowCount = qMax(1, int(qRound(float(cols) * aspect)));
    }
    cols = qMax(1, cols);
    rowCount = qMax(1, rowCount);
    const QImage small = image.scaled(cols, rowCount, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    return small.scaled(image.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
}
