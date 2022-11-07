/*
  Q Light Controller Plus
  palettemanager.h

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

#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QQuickView>
#include <QObject>

#include "qlcpalette.h"

class Doc;
class ListModel;
class ContextManager;

class PaletteManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant paletteList READ paletteList NOTIFY paletteListChanged)
    Q_PROPERTY(int typeFilter READ typeFilter WRITE setTypeFilter NOTIFY typeFilterChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

    Q_PROPERTY(int dimmerCount READ dimmerCount NOTIFY dimmerCountChanged)
    Q_PROPERTY(int colorCount READ colorCount NOTIFY colorCountChanged)
    Q_PROPERTY(int positionCount READ positionCount NOTIFY positionCountChanged)

public:
    PaletteManager(QQuickView *view, Doc *doc, ContextManager *ctxManager, QObject *parent = nullptr);
    ~PaletteManager();

    /** Get a list of Palettes filtered with typeFilter */
    QVariant paletteList();

    /** Return a list of Palette names from the give $list of IDs */
    Q_INVOKABLE QStringList selectedItemNames(QVariantList list);

    /** Get the reference to a QLCPalette from the given ID */
    Q_INVOKABLE QLCPalette *getPalette(quint32 id);

    /** Request a palette for editing.
     * The returned palette is mapped by type on m_editingMap */
    Q_INVOKABLE QLCPalette *getEditingPalette(int type);

    /** Request the removal of a previously requested palette */
    Q_INVOKABLE bool releaseEditingPalette(int type);

    /** Create a new palette, get a new ID and add it
     *  to the current project */
    Q_INVOKABLE quint32 createPalette(QLCPalette *palette, QString name);

    /** Preview the given palette via Context manager */
    Q_INVOKABLE void previewPalette(QLCPalette *palette);

    /** Update the give palette with single value (e.g. dimmer, pan, tilt, etc) */
    Q_INVOKABLE void updatePalette(QLCPalette *palette, QVariant value1);

    /** Update the give palette with two values (e.g. pan & tilt) */
    Q_INVOKABLE void updatePalette(QLCPalette *palette, QVariant value1, QVariant value2);

    /** Delete the selected palettes from the current project */
    Q_INVOKABLE void deletePalettes(QVariantList list);

    /** Create a new Scene and add the palette with the given id to it */
    Q_INVOKABLE void addPaletteToNewScene(quint32 id, QString sceneName);

    /** Get/Set the type of Palettes to be displayed */
    int typeFilter() const;
    void setTypeFilter(quint32 filter);

    /** Get/Set a string to filter Function names */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

    int dimmerCount() const { return m_dimmerCount; }
    int colorCount() const { return m_colorCount; }
    int positionCount() const { return m_positionCount; }

    void updatePaletteList();

signals:
    void typeFilterChanged();
    void searchFilterChanged();
    void paletteListChanged();

    void dimmerCountChanged();
    void colorCountChanged();
    void positionCountChanged();

public slots:
    void slotDocLoaded();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Context Manager. Used to apply DMX values */
    ContextManager *m_contextManager;

    quint32 m_typeFilter;
    QString m_searchFilter;

    int m_dimmerCount, m_colorCount, m_positionCount;

    ListModel *m_paletteList;
    // map of type/palette used for editing
    QMap<int, QLCPalette *> m_editingMap;
};

#endif // PALETTEMANAGER_H
