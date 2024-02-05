/*
  Q Light Controller Plus
  physicaledit.h

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

#ifndef PHYSICALEDIT_H
#define PHYSICALEDIT_H

#include <QObject>

#include "qlcphysical.h"

class PhysicalEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString bulbType READ bulbType WRITE setBulbType NOTIFY bulbTypeChanged)
    Q_PROPERTY(int bulbLumens READ bulbLumens WRITE setBulbLumens NOTIFY bulbLumensChanged)
    Q_PROPERTY(int bulbColorTemperature READ bulbColorTemperature WRITE setBulbColorTemperature NOTIFY bulbColorTemperatureChanged)

    Q_PROPERTY(double weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int depth READ depth WRITE setDepth NOTIFY depthChanged)

    Q_PROPERTY(QString lensType READ lensType WRITE setLensType NOTIFY lensTypeChanged)
    Q_PROPERTY(double lensDegreesMin READ lensDegreesMin WRITE setLensDegreesMin NOTIFY lensDegreesMinChanged)
    Q_PROPERTY(double lensDegreesMax READ lensDegreesMax WRITE setLensDegreesMax NOTIFY lensDegreesMaxChanged)

    Q_PROPERTY(QString focusType READ focusType WRITE setFocusType NOTIFY focusTypeChanged)
    Q_PROPERTY(int focusPanMax READ focusPanMax WRITE setFocusPanMax NOTIFY focusPanMaxChanged)
    Q_PROPERTY(int focusTiltMax READ focusTiltMax WRITE setFocusTiltMax NOTIFY focusTiltMaxChanged)
    Q_PROPERTY(QSize layoutSize READ layoutSize WRITE setLayoutSize NOTIFY layoutSizeChanged)

    Q_PROPERTY(int powerConsumption READ powerConsumption WRITE setPowerConsumption NOTIFY powerConsumptionChanged)
    Q_PROPERTY(QString dmxConnector READ dmxConnector WRITE setDmxConnector NOTIFY dmxConnectorChanged)

public:
    PhysicalEdit(QLCPhysical phy, QObject *parent = nullptr);
    ~PhysicalEdit();

    QLCPhysical physical();

public:
    QString bulbType() const;
    void setBulbType(const QString type);

    int bulbLumens() const;
    void setBulbLumens(int lumens);

    int bulbColorTemperature() const;
    void setBulbColorTemperature(int temp);

    double weight() const;
    void setWeight(double weight);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    int depth() const;
    void setDepth(int depth);

    QString lensType() const;
    void setLensType(const QString type);

    double lensDegreesMin() const;
    void setLensDegreesMin(double degrees);

    double lensDegreesMax() const;
    void setLensDegreesMax(double degrees);

    QString focusType() const;
    void setFocusType(const QString type);

    int focusPanMax() const;
    void setFocusPanMax(int pan);

    int focusTiltMax() const;
    void setFocusTiltMax(int tilt);

    QSize layoutSize() const;
    void setLayoutSize(QSize size);

    int powerConsumption() const;
    void setPowerConsumption(int watt);

    QString dmxConnector() const;
    void setDmxConnector(const QString type);

signals:
    void changed();
    void bulbTypeChanged();
    void bulbLumensChanged();
    void bulbColorTemperatureChanged();
    void weightChanged();
    void widthChanged();
    void heightChanged();
    void depthChanged();
    void lensTypeChanged();
    void lensDegreesMinChanged();
    void lensDegreesMaxChanged();
    void focusTypeChanged();
    void focusPanMaxChanged();
    void focusTiltMaxChanged();
    void layoutSizeChanged();
    void powerConsumptionChanged();
    void dmxConnectorChanged();

private:
    QLCPhysical m_phy;
};

#endif /* PHYSICALEDIT_H */
