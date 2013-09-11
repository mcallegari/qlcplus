/*
  Q Light Controller
  vcspeeddialproperties.h

  Copyright (c) Heikki Junnila

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

#ifndef VCSPEEDDIALPROPERTIES_H
#define VCSPEEDDIALPROPERTIES_H

#include <QDialog>
#include "ui_vcspeeddialproperties.h"
#include "qlcinputsource.h"

class VCSpeedDial;
class Doc;

class VCSpeedDialProperties : public QDialog, public Ui_VCSpeedDialProperties
{
    Q_OBJECT

public:
    VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc);
    ~VCSpeedDialProperties();

public slots:
    /** @reimp */
    void accept();

private:
    VCSpeedDial* m_dial;
    Doc* m_doc;

    /************************************************************************
     * Functions page
     ************************************************************************/
private slots:
    void slotAddClicked();
    void slotRemoveClicked();

private:
    /** Generate a QSet of functions currently in the tree widget */
    QSet <quint32> functions() const;

    /** Create a tree item for the given function $id */
    void createFunctionItem(quint32 id);

    /************************************************************************
     * Input page
     ************************************************************************/
private:
    void updateInputSources();

private slots:
    void slotAutoDetectAbsoluteInputSourceToggled(bool checked);
    void slotChooseAbsoluteInputSourceClicked();
    void slotAbsoluteInputValueChanged(quint32 universe, quint32 channel);

    void slotAutoDetectTapInputSourceToggled(bool checked);
    void slotChooseTapInputSourceClicked();
    void slotTapInputValueChanged(quint32 universe, quint32 channel);

    void slotAttachKey();
    void slotDetachKey();

private:
    QLCInputSource m_absoluteInputSource;
    QLCInputSource m_tapInputSource;
    QKeySequence m_tapKeySequence;
};

#endif
