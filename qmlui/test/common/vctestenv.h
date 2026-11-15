/*
  Q Light Controller Plus - Test Unit
  vctestenv.h

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

#ifndef VCTESTENV_H
#define VCTESTENV_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQuickView>
#include <QBuffer>

#include "virtualconsole.h"
#include "tardis.h"
#include "doc.h"

/**
 * The minimal environment needed by the Virtual Console widgets:
 * a Doc, a QQuickView, the (stubbed) Tardis singleton and a VirtualConsole.
 * Create one in init() and delete it in cleanup().
 */
class VCTestEnv
{
public:
    VCTestEnv()
    {
        m_doc = new Doc(nullptr);
        m_view = new QQuickView();
        m_vc = new VirtualConsole(m_view, m_doc, nullptr);
        m_tardis = new Tardis(m_view, m_doc, nullptr, nullptr, nullptr,
                              nullptr, nullptr, nullptr, m_vc);

        // without the App the pixel density is 0, and so is the snapping
        // size: disable snapping to avoid dividing by zero in VCFrame
        m_vc->setSnapping(false);
    }

    ~VCTestEnv()
    {
        // VirtualConsole has no destructor removing the pages children,
        // so they must be deleted while the widgets map is still alive
        m_vc->resetContents();
        delete m_tardis;
        delete m_vc;
        delete m_view;
        delete m_doc;
    }

    Doc *doc() const { return m_doc; }
    QQuickView *view() const { return m_view; }
    VirtualConsole *vc() const { return m_vc; }

private:
    Doc *m_doc;
    QQuickView *m_view;
    VirtualConsole *m_vc;
    Tardis *m_tardis;
};

/** Serialize a widget (or anything with a compatible saveXML) to a buffer,
 *  formatted like a project file (see App::saveWorkspace) unless $compact */
template <typename T>
static inline QByteArray vcSaveXML(const T *object, bool compact = false)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    if (compact == false)
    {
        xmlWriter.setAutoFormatting(true);
        xmlWriter.setAutoFormattingIndent(1);
    }
    object->saveXML(&xmlWriter);
    xmlWriter.writeEndDocument();
    buffer.close();
    return buffer.data();
}

/**
 * A XML reader backed by a QBuffer, positioned on the first start element.
 * A device is needed since the VC widgets refuse to load from a reader
 * without one.
 */
class VCXMLReader
{
public:
    explicit VCXMLReader(const QByteArray &data)
    {
        m_buffer.setData(data);
        m_buffer.open(QIODevice::ReadOnly | QIODevice::Text);
        m_reader.setDevice(&m_buffer);
        m_reader.readNextStartElement();
    }

    QXmlStreamReader &reader() { return m_reader; }

private:
    QBuffer m_buffer;
    QXmlStreamReader m_reader;
};

#endif
