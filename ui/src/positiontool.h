/*
  Q Light Controller Plus
  positiontool.h

  Copyright (c) Jano Svitok

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

#ifndef POSITIONTOOL_H
#define POSITIONTOOL_H

#include <QDialog>
#include <ui_positiontool.h>

class VCXYPadArea;

class PositionTool : public QDialog, Ui_PositionTool
{
    Q_OBJECT
    Q_DISABLE_COPY(PositionTool)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    PositionTool(const QPointF & initial, QRectF degreesRange, QWidget * parent = 0);
    virtual ~PositionTool();

private:

    /*************************************************************************
     * Current position
     *************************************************************************/
public:
    /** Get the pad's current position (i.e. where the point is) */
    QPointF position() const;
    void setPosition(const QPointF & position);

signals:
    void currentPositionChanged(const QPointF & position);

public slots:
    void slotPositionChanged(const QPointF & position);

private:
    VCXYPadArea *m_area;
};

#endif
