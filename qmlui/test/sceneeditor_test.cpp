/*
  Q Light Controller Plus - Unit test
  Licensed under the Apache License, Version 2.0.
  http://www.apache.org/licenses/LICENSE-2.0
*/

#include <QtTest>
#include <QApplication>
#include <QQmlContext>
#include <QTemporaryDir>
#include <QSettings>

#include "app.h"
#include "doc.h"
#include "fixture.h"
#include "functionmanager.h"
#include "scene.h"
#define private public
#define protected public
#include "sceneeditor.h"
#undef protected
#undef private

class FixtureConsole : public QQuickItem
{
    Q_OBJECT
public:
    int changes = 0;
    Q_INVOKABLE void setChannelValue(QVariant, QVariant) { ++changes; }
};

class SceneEditor_Test : public QObject
{
    Q_OBJECT
private slots:
    void destroyedDelegate();
    void replacementDelegate();
    void rebuiltFixtureList();
    void replacedSceneConsole();
    void nativeEditorCycles();
};

void SceneEditor_Test::destroyedDelegate()
{
    QQuickView view;
    view.setContent(QUrl(), nullptr, new QQuickItem);
    Doc doc(nullptr);
    auto *fixture = new Fixture(&doc);
    fixture->setChannels(1);
    QVERIFY(doc.addFixture(fixture));
    SceneEditor editor(&view, &doc);
    editor.m_fixtureIDs.append(fixture->id());
    QQuickItem console;
    editor.sceneConsoleLoaded(true, &console);
    auto *delegate = new FixtureConsole;
    editor.registerFixtureConsole(0, delegate);
    delete delegate; // its QML destruction callback has not unregistered it
    QVERIFY(!editor.m_fxConsoleMap.value(0));
    editor.slotSceneValueChanged(SceneValue(fixture->id(), 0, 100));
    QCOMPARE(editor.m_channelsCache.value(fixture->id()).at(0), char(100));
}

void SceneEditor_Test::replacementDelegate()
{
    QQuickView view;
    view.setContent(QUrl(), nullptr, new QQuickItem);
    Doc doc(nullptr);
    auto *fixture = new Fixture(&doc);
    fixture->setChannels(1);
    QVERIFY(doc.addFixture(fixture));
    SceneEditor editor(&view, &doc);
    editor.m_fixtureIDs.append(fixture->id());
    QQuickItem console;
    editor.sceneConsoleLoaded(true, &console);
    FixtureConsole retired, replacement;
    editor.registerFixtureConsole(0, &retired);
    editor.registerFixtureConsole(0, &replacement);
    editor.unRegisterFixtureConsole(0, &retired);
    editor.slotSceneValueChanged(SceneValue(fixture->id(), 0, 100));
    QCOMPARE(retired.changes, 0);
    QCOMPARE(replacement.changes, 1);
    editor.unRegisterFixtureConsole(0, &replacement);
    editor.slotSceneValueChanged(SceneValue(fixture->id(), 0, 101));
    QCOMPARE(replacement.changes, 1);
}

void SceneEditor_Test::rebuiltFixtureList()
{
    QQuickView view;
    view.setContent(QUrl(), nullptr, new QQuickItem);
    Doc doc(nullptr);
    auto *fixture = new Fixture(&doc);
    fixture->setChannels(1);
    QVERIFY(doc.addFixture(fixture));
    Scene scene(&doc);
    scene.addFixture(fixture->id());
    SceneEditor editor(&view, &doc);
    editor.m_scene = &scene;
    editor.updateLists();
    QQuickItem console;
    editor.sceneConsoleLoaded(true, &console);
    FixtureConsole retired;
    editor.registerFixtureConsole(0, &retired);
    editor.updateLists();
    QVERIFY(editor.m_fxConsoleMap.isEmpty());
    editor.slotSceneValueChanged(SceneValue(fixture->id(), 0, 100));
    QCOMPARE(retired.changes, 0);
}

void SceneEditor_Test::replacedSceneConsole()
{
    QQuickView view;
    view.setContent(QUrl(), nullptr, new QQuickItem);
    Doc doc(nullptr);
    SceneEditor editor(&view, &doc);
    QQuickItem retired, replacement;
    editor.sceneConsoleLoaded(true, &retired);
    editor.sceneConsoleLoaded(true, &replacement);
    editor.sceneConsoleLoaded(false, &retired);
    QCOMPARE(editor.m_sceneConsole.data(), &replacement);
    editor.sceneConsoleLoaded(false, &replacement);
    QVERIFY(!editor.m_sceneConsole);
}

void SceneEditor_Test::nativeEditorCycles()
{
    if (!qEnvironmentVariableIsSet("QLC_SCENE_UI_TEST"))
        QSKIP("Set QLC_SCENE_UI_TEST=1 to exercise native QML editor lifetimes.");

    App app;
    app.set3dSupported(false);
    app.startup();
    app.show();
    auto *doc = app.doc();
    auto *fixture = new Fixture(doc);
    fixture->setName("Test dimmer");
    fixture->setChannels(1);
    QVERIFY(doc->addFixture(fixture));
    auto *addedFixture = new Fixture(doc);
    addedFixture->setName("Added dimmer");
    addedFixture->setChannels(1);
    addedFixture->setAddress(1);
    QVERIFY(doc->addFixture(addedFixture));
    auto *scene = new Scene(doc);
    scene->addFixture(fixture->id());
    QVERIFY(doc->addFunction(scene));
    auto *manager = qobject_cast<FunctionManager *>(
        app.rootContext()->contextProperty("functionManager").value<QObject *>());
    QVERIFY(manager);
    QVERIFY(QMetaObject::invokeMethod(app.rootObject(), "switchToContext",
            Q_ARG(QVariant, "FIXANDFUNC"), Q_ARG(QVariant, "qrc:/FixturesAndFunctions.qml")));
    QTest::qWait(100);
    auto *panel = app.rootObject()->findChild<QQuickItem *>("funcRightPanel");
    QVERIFY(panel);
    auto *bottomPanel = app.rootObject()->findChild<QQuickItem *>("bottomPanelItem");
    QVERIFY(bottomPanel);
    for (int i = 0; i < 100; ++i)
    {
        scene->removeFixture(addedFixture->id());
        manager->setEditorFunction(scene->id(), true, false);
        QVERIFY(QMetaObject::invokeMethod(bottomPanel, "animatePanel", Q_ARG(QVariant, true)));
        QTest::qWait(10);
        auto *editor = qobject_cast<SceneEditor *>(manager->currentEditor());
        QVERIFY(editor);
        QTRY_VERIFY(editor->m_sceneConsole);
        QTRY_VERIFY(!editor->m_fxConsoleMap.isEmpty());
        editor->addComponent(App::FixtureDragItem, addedFixture->id());
        QTRY_VERIFY(!editor->m_fxConsoleMap.isEmpty());
        manager->setChannelValue(fixture->id(), 0, uchar(i));
        QTest::qWait(10);
        QVERIFY(editor->m_fxConsoleMap.cbegin().value());
        QCOMPARE(editor->m_fixtureIDs.size(), 2);
        QVERIFY(QMetaObject::invokeMethod(panel, "requestEditor",
                Q_ARG(QVariant, -1), Q_ARG(QVariant, 0)));
        manager->setEditorFunction(Function::invalidId(), false, false);
        app.engine()->collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QTest::qWait(10);
    }
    app.stopAllFunctions();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("qlcplus-tests");
    QCoreApplication::setApplicationName("sceneeditor-test");
    QTemporaryDir settings;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
    SceneEditor_Test test;
    return QTest::qExec(&test, argc, argv);
}

#include "sceneeditor_test.moc"
