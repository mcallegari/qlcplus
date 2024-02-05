/*
  Q Light Controller Plus
  colorfilters.h

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

#ifndef COLORFILTERS_H
#define COLORFILTERS_H

#include <QVariant>
#include <QString>
#include <QColor>
#include <QFile>

#define KXMLColorFilters QString("ColorFilters")

typedef struct
{
    QString m_name; // the filter's name
    QColor m_rgb;   // RGB / CMY color
    QColor m_wauv;  // White / Amber / UV values
} ColorInfo;

class ColorFilters : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isUser READ isUser CONSTANT)
    Q_PROPERTY(QVariantList filtersList READ filtersList NOTIFY filtersListChanged)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    ColorFilters(QObject *parent = 0);

    /** Get/Set the color filters name */
    QString name() const;
    void setName(QString name);

    /** Get the path where the color filters are stored in. Don't use
        this as a unique ID since this varies between platforms. */
    QString path() const;

    /** Get/Set if the color filter file is in the system or user folder */
    bool isUser() const;
    void setIsUser(bool user);

    /** Get the list of filters loaded from XML suitable for QML */
    QVariantList filtersList();

signals:
    void filtersListChanged();

    /********************************************************************
     * Editing
     ********************************************************************/
public:
    /** Add a new color filter with the given $name */
    Q_INVOKABLE void addFilter(QString name, quint8 red, quint8 green, quint8 blue,
                               quint8 white, quint8 amber, quint8 uv);

    /** Modify an existing filter at $index position */
    Q_INVOKABLE void changeFilterAt(int index, quint8 red, quint8 green, quint8 blue,
                                    quint8 white, quint8 amber, quint8 uv);

    /** Remove an existing filter at $index position */
    Q_INVOKABLE void removeFilterAt(int index);

    /** Rename an existing filter at $index position with $newName */
    Q_INVOKABLE void renameFilterAt(int index, QString newName);

protected:
    QString m_name;
    QString m_path;
    bool m_isUser;
    QList<ColorInfo> m_filterList;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Save the filter with a previously set m_path */
    Q_INVOKABLE void save();

    /** Save the filter into an XML file */
    QFile::FileError saveXML(const QString& fileName);

    /** Load this filter's content from the given file */
    QFile::FileError loadXML(const QString& fileName);
};

#endif /* COLORFILTERS_H */
