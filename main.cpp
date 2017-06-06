#include "feature.h"
#include <QApplication>
#include <QPushButton>
#include "layouts.h"
#include "devio.h"
#include "xlog/win/fix_vs2003_singleton_threads.h"

Layouts *g_layouts;

int main(int argc, char **argv)
{
    // for QSettings
    QCoreApplication::setOrganizationName("Logitech");
    QCoreApplication::setApplicationName("Bazinga");

    // Cleans up devio before exiting main to eliminate the need for devio to perform
    // a non-graceful shutdown to fix VS2012 std::thread deadlock bug 747145.  Place
    // it first before any singleton classes which use threads so that it goes out of
    // scope last.
    //
    Vs2003SingletonThreadCleanup vs2013_thread_cleanup_fix;

    QApplication app (argc, argv);

    Layouts window;

    QObject::connect(&app, SIGNAL(aboutToQuit()), &window, SLOT(slotAbandonDevices()));

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 999
#endif
    window.setWindowTitle(QString("Bazinga V1.00.%1").arg(BUILD_NUMBER));

#ifdef BAZINGA_DFU_ONLY
    window.setWindowTitle(window.windowTitle().append(" (DFU Only)"));
#endif

    window.show();

    g_layouts = &window;

    return app.exec();
}


