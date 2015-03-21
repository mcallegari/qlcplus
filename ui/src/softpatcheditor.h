/*
  Q Light Controller Plus
  channelsselection.h

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

#ifndef SOFTPATCHEDITOR_H
#define SOFTPATCHEDITOR_H

#include <QDialog>
#include <QMutex>

#include "ui_softpatcheditor.h"
#include "dmxsource.h"



/**
 * TODO:
 *  FIX: use real universeAddreses
 *
 */

class Doc;
class FixtureManager;

#define PROP_ID       Qt::UserRole
#define PROP_UNIVERSE Qt::UserRole + 1
#define PROP_ADDRESS  Qt::UserRole + 2
#define PROP_PATCH    Qt::UserRole + 3

class SoftpatchEditor: public QDialog, public Ui_SoftpatchEditor, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(SoftpatchEditor)

public:
    SoftpatchEditor(Doc* doc, FixtureManager *mgr, QWidget *parent=0);
    ~SoftpatchEditor();

    /** @reimp */
    void writeDMX(MasterTimer* timer, QList<Universe*> ua);

private:
    Doc* m_doc;
    FixtureManager* m_fixture_manager;
    QMultiMap<uint, QTreeWidgetItem*> m_duplicateChannels;
    QMutex m_mutex;

    bool runTest;
    bool resetTest;
    quint32 testUniverse;
    int testChannel;
    uchar testValue;

protected:
    void updateFixturesTree();

    /** returns if overlapping channels in current softpatch */
    bool hasDupliateChannels();

    /** returns a set containing universe channels a Fixture stored in the TreewidgetItem **/
    QSet<quint32> getChannelSet(QTreeWidgetItem* item);
    void initChannelSearch(QTreeWidgetItem* item);

    /** return a set of duplicate or overlapping channels of two Fixtures stored in the Treewidget **/
    QSet<quint32> duplicateChannelsSet(QTreeWidgetItem* changed, QTreeWidgetItem* other);

    /** mark all Fixtures containing overlapping channels with others red **/
    void markFixtures();


protected slots:
    /** Slot called when Test Button is pressed / released */
    void slotTestButtonPressed();

    /**
     * Slot called when channel address is changed
     * check for channel duplicates
     */
    void slotChannelPatched();

    /** Callback for OK button clicks */
    void accept();
};

#endif // SOFTPATCHEDITOR_H
