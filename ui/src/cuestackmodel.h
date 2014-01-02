/*
  Q Light Controller
  cuestackmodel.h

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

#ifndef CUESTACKMODEL_H
#define CUESTACKMODEL_H

#include <QAbstractItemModel>
#include <qglobal.h>

class CueStack;

/** @addtogroup ui_vc_widgets
 * @{
 */

class CueStackModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CueStackModel(QObject* parent = 0);
    ~CueStackModel();

    void setCueStack(CueStack* cs);
    CueStack* cueStack() const;

private:
    CueStack* m_cueStack;

    /************************************************************************
     * CueStack slots
     ************************************************************************/
private slots:
    void slotAdded(int index);
    void slotRemoved(int index);
    void slotChanged(int index);
    void slotCurrentCueChanged(int index);

    /************************************************************************
     * QAbstractItemModel
     ************************************************************************/
public:
    enum Columns
    {
        IndexColumn     = 0,
        FadeInColumn    = 1,
        FadeOutColumn   = 2,
        DurationColumn  = 3,
        NameColumn      = 4,
        ColumnCount     = 5
    };

    int columnCount(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QStringList mimeTypes () const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QMimeData* mimeData(const QModelIndexList& indexes) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

private:
    /** Convert $ms milliseconds to a nicer seconds.milliseconds figure */
    QString speedText(uint ms) const;
};

/** @} */

#endif
