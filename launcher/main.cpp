#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QTimer>
#include <QDir>

#include "qlcconfig.h"
#include "launcher.h"

void loadTranslation(const QString& locale, QApplication& app)
{
    QString file(QString("launcher_%1").arg(locale));
#if defined(__APPLE__) || defined(Q_OS_MAC)
    QString path(QString("%1/../%2").arg(QApplication::applicationDirPath())
                 .arg(TRANSLATIONDIR));
#else
    QString path(TRANSLATIONDIR);
#endif

    QTranslator* translator = new QTranslator(&app);
    if (translator->load(file, path) == true)
    {
        qDebug() << "Using translation for" << locale;
        app.installTranslator(translator);
    }
    else
    {
        qDebug() << "Unable to find translation for" << locale
        << "in" << path;
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    /* Load plugins from within the bundle ONLY */
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));

    loadTranslation(QLocale::system().name(), app);

    Launcher launcher;
    app.installEventFilter(&launcher);

    // If launcher is started by the system after user has activated
    // either a .qxf or .qxw file, we don't need to show the dialog
    // at all. Since this "mime-open" comes as an event, we won't know
    // about it until app.exec() has been called. So, give some grace
    // time before actually showing the launcher dialog, in case the
    // event arrives and we can destroy the dialog before actually
    // showing it.
    QTimer::singleShot(100, &launcher, SLOT(show()));
    return app.exec();
}
