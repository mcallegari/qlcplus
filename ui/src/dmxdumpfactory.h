/*
  Q Light Controller Plus
  dmxdumpfactory.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DMXDUMPFACTORY_H
#define DMXDUMPFACTORY_H

#include <QDialog>

#include "ui_dmxdumpfactory.h"

class DmxDumpFactoryProperties;
class VCWidget;
class Doc;

class DmxDumpFactory : public QDialog, public Ui_DmxDumpFactory
{
    Q_OBJECT
    Q_DISABLE_COPY(DmxDumpFactory)
    
public:
    DmxDumpFactory(Doc* doc, DmxDumpFactoryProperties* props, QWidget *parent = 0);
    ~DmxDumpFactory();

protected slots:
    void slotUpdateChasersTree();
    void slotUpdateButtons();
    void slotUpdateSliders();

protected:
    void updateFixturesTree();
    QList<VCWidget *> getChildren(VCWidget *obj, int type);
    void updateWidgetsTree(int type);

private:
    Doc* m_doc;
    DmxDumpFactoryProperties* m_properties;

protected:
    int m_universesCount;
    int m_fixturesCount;
    int m_channelsCount;

protected slots:
    void slotDumpModeChanged(bool mode);
    void slotDumpNonZeroChanged(bool active);

    /** Callback for OK button clicks */
    void accept();

};

#endif // DMXDUMPFACTORY_H
