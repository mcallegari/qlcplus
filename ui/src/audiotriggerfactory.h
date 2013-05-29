/*
  Q Light Controller Plus
  audiotriggerfactory.h

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

#ifndef AUDIOTRIGGERFACTORY_H
#define AUDIOTRIGGERFACTORY_H

#include <QDialog>
#include <QThread>

#include "audiotriggerwidget.h"

class AudioCapture;

namespace Ui {
class AudioTriggerFactory;
}

class AudioTriggerFactory : public QDialog
{
    Q_OBJECT
    
public:
    explicit AudioTriggerFactory(QWidget *parent = 0);
    ~AudioTriggerFactory();
    
protected slots:
    void slotEnableCapture(bool enable);
    void slotConfiguration();

private:
    Ui::AudioTriggerFactory *ui;

    AudioCapture *m_inputCapture;
    AudioTriggerWidget *m_spectrum;
};

#endif // AUDIOTRIGGERFACTORY_H
