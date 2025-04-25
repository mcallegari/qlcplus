/*
  Q Light Controller Plus
  addrgbpanel.h

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

#ifndef ADDRGBPANEL_H
#define ADDRGBPANEL_H

#include <QDialog>

#include "ui_addrgbpanel.h"
#include "fixture.h"

class QLCFixtureDef;
class QLCFixtureMode;
class Doc;

class AddRGBPanel : public QDialog, public Ui_AddRGBPanel
{
    Q_OBJECT
    Q_DISABLE_COPY(AddRGBPanel)

public:
    AddRGBPanel(QWidget *parent, const Doc* doc);
    ~AddRGBPanel();

    enum Orientation {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    enum Type {
        Unknown,
        Snake,
        ZigZag
    };

    enum Direction {
    	Undefined,
		Horizontal,
		Vertical
    };

    QString name();
    int universeIndex();
    int address();
    int columns();
    int rows();
    quint32 physicalWidth();
    quint32 physicalHeight();
    Orientation orientation();
    Type type();
    Direction direction();
    Fixture::Components components();
    bool is16Bit();
    bool crossUniverse();

private:
    /** Check if an address is available for contiguous channels.
     *  Note that value is an absolute address.
     */
    bool checkAddressAvailability();

protected slots:
    void slotUniverseChanged();
    void slotComponentsChanged(int index);
    void slotAddressChanged();
    void slotSizeChanged(int val);

protected:
    const Doc *m_doc;

};

#endif // ADDRGBPANEL_H
