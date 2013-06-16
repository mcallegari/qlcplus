/*
  Q Light Controller
  rgbmatrixeditor.h

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

#ifndef RGBMATRIXEDITOR_H
#define RGBMATRIXEDITOR_H

#include <QPointer>
#include <QWidget>
#include <QHash>

#include "ui_rgbmatrixeditor.h"
#include "rgbmatrix.h"
#include "qlcpoint.h"
#include "doc.h"

class SpeedDialWidget;
class QGraphicsScene;
class RGBMatrix;
class QTimer;
class Doc;

class RGBMatrixEditor : public QWidget, public Ui_RGBMatrixEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(RGBMatrixEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBMatrixEditor(QWidget* parent, RGBMatrix* mtx, Doc* doc);
    ~RGBMatrixEditor();

public slots:
    void slotFunctionManagerActive(bool active);

private:
    void init();

    void createSpeedDials();
    void fillPatternCombo();
    void fillFixtureGroupCombo();
    void fillAnimationCombo();
    void updateExtraOptions();

    void createPreviewItems();

private slots:
    void slotPreviewTimeout();
    void slotNameEdited(const QString& text);
    void slotPatternActivated(const QString& text);
    void slotFixtureGroupActivated(int index);
    void slotColorButtonClicked();

    void slotTextEdited(const QString& text);
    void slotFontButtonClicked();
    void slotAnimationActivated(const QString& text);
    void slotOffsetSpinChanged();

    void slotLoopClicked();
    void slotPingPongClicked();
    void slotSingleShotClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

    void slotFadeInChanged(int ms);
    void slotFadeOutChanged(int ms);
    void slotHoldChanged(int ms);
    void slotDurationTapped();

    void slotTestClicked();
    void slotRestartTest();
    void slotModeChanged(Doc::Mode mode);
    void slotFixtureGroupAdded();
    void slotFixtureGroupRemoved();
    void slotFixtureGroupChanged(quint32 id);

private:
    Doc* m_doc;
    RGBMatrix* m_matrix; // The RGBMatrix being edited

    QList <RGBScript> m_scripts;
    QList <RGBMap> m_previewMaps;

    QPointer<SpeedDialWidget> m_speedDials;

    QGraphicsScene* m_scene;
    QTimer* m_previewTimer;
    uint m_previewIterator;
    int m_previewStep;
    QHash <QLCPoint,QGraphicsItem*> m_previewHash;
};

#endif
