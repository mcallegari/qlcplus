/*
  Q Light Controller
  vcframeproperties.h

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

#ifndef VCFRAMEPROPERTIES_H
#define VCFRAMEPROPERTIES_H

#include <QDialog>
#include "ui_vcframeproperties.h"
#include "qlcinputsource.h"

class VCFrame;
class Doc;

class VCFrameProperties : public QDialog, public Ui_VCFrameProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCFrameProperties)

public:
    VCFrameProperties(QWidget* parent, VCFrame *frame, Doc *doc);
    ~VCFrameProperties();

    bool allowChildren() const;
    bool allowResize() const;
    bool showHeader() const;
    QString frameName() const;
    bool multipageEnabled() const;
    bool cloneWidgets() const;

protected:
    VCFrame *m_frame;
    Doc* m_doc;

    /************************************************************************
     * Next page
     ************************************************************************/
protected slots:
    void slotNextAttachClicked();
    void slotNextDetachClicked();
    void slotNextChooseInputClicked();
    void slotNextAutoDetectInputToggled(bool checked);
    void slotNextInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateNextInputSource();

protected:
    QKeySequence m_nextKeySequence;
    QLCInputSource m_nextInputSource;

    /************************************************************************
     * Previous page
     ************************************************************************/
protected slots:
    void slotPreviousAttachClicked();
    void slotPreviousDetachClicked();
    void slotPreviousChooseInputClicked();
    void slotPreviousAutoDetectInputToggled(bool checked);
    void slotPreviousInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updatePreviousInputSource();

protected:
    QKeySequence m_previousKeySequence;
    QLCInputSource m_previousInputSource;

public slots:
    void accept();
};

#endif
