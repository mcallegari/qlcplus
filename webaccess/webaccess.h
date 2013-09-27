/*
  Q Light Controller Plus
  webaccess.h

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

#ifndef WEBACCESS_H
#define WEBACCESS_H

#include <QObject>
#include "mongoose.h"

class VirtualConsole;
class VCWidget;
class VCButton;
class VCSlider;
class VCLabel;
class VCFrame;
class Doc;

class WebAccess : public QObject
{
    Q_OBJECT
public:
    explicit WebAccess(Doc *doc, VirtualConsole *vcInstance, QObject *parent = 0);
    /** Destructor */
    ~WebAccess();

    int beginRequestHandler(struct mg_connection *conn);
    void websocketReadyHandler(struct mg_connection *conn);
    int websocketDataHandler(struct mg_connection *conn, int flags,
                               char *data, size_t data_len);

private:

    QString getFrameHTML(VCFrame *frame);
    QString getButtonHTML(VCButton *btn);
    QString getSliderHTML(VCSlider *slider);
    QString getLabelHTML(VCLabel *label);

    QString getChildrenHTML(VCWidget *frame);
    QString getVCHTML();
protected:
    QString m_JScode;
    QString m_CSScode;

    bool m_buttonFound;
    bool m_frameFound;
    bool m_labelFound;
    bool m_cueListFound;
    bool m_sliderFound;
    bool m_knobFound;
    bool m_xyPadFound;
    bool m_speedDialFound;

protected:
    Doc *m_doc;
    VirtualConsole *m_vc;

    struct mg_context *m_ctx;
    struct mg_callbacks m_callbacks;

signals:
    
public slots:
    
};

#endif // WEBACCESS_H
