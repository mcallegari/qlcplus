/*
  Q Light Controller
  ioplugincache.h

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

#ifndef IOPLUGINCACHE_H
#define IOPLUGINCACHE_H

#include <QObject>
#include <QDir>

class QLCIOPlugin;

class IOPluginCache : public QObject
{
    Q_OBJECT

public:
    IOPluginCache(QObject* parent);
    ~IOPluginCache();

    /** Load plugins from the given directory. */
    void load(const QDir& dir);

    /** Get a list of available I/O plugins. */
    QList <QLCIOPlugin*> plugins() const;

    /** Get an I/O plugin by its name. */
    QLCIOPlugin* plugin(const QString& name) const;

    /** Get the system plugin directory. */
    static QDir systemPluginDirectory();

signals:
    void pluginConfigurationChanged(QLCIOPlugin* plugin);
    void pluginLoaded(const QString& name);

private slots:
    void slotConfigurationChanged();

private:
    QList <QLCIOPlugin*> m_plugins;
};

#endif
