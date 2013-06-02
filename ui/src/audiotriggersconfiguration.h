/*
  Q Light Controller Plus
  audiotriggersconfiguration.h

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

#ifndef AUDIOTRIGGERSCONFIGURATION_H
#define AUDIOTRIGGERSCONFIGURATION_H

#include <QDialog>

#include "ui_audiotriggersconfiguration.h"
#include "audiotriggerfactory.h"
#include "audiocapture.h"
#include "doc.h"

class AudioTriggersConfiguration : public QDialog, public Ui_AudioTriggersConfiguration
{
    Q_OBJECT
    
public:
    explicit AudioTriggersConfiguration(QWidget *parent = 0, Doc *doc = 0,
                                        AudioCapture *capture = 0);
    ~AudioTriggersConfiguration();

    /** @reimp */
    void accept();
    
private slots:
    void updateTreeItem(QTreeWidgetItem *item, int idx);
    void updateTree();
    void slotTypeComboChanged(int comboIndex);
    void slotDmxSelectionClicked();
    void slotFunctionSelectionClicked();
    void slotWidgetSelectionClicked();
    void slotMinThresholdChanged(int val);
    void slotMaxThresholdChanged(int val);

private:
    Doc *m_doc;
    AudioTriggerFactory *m_factory;
    AudioCapture *m_capture;
};

#endif // AUDIOTRIGGERSCONFIGURATION_H
