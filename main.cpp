#include <QApplication>
#include <QSurfaceFormat>

#include "time.h"
time_t time_func() { return time(nullptr); }

//#include "mywidget.h"
#include "launcherform.h"

int main(int argc, char *argv[]) {
    srand(uint(time_func()));

    QApplication app(argc, argv);
    setlocale(LC_NUMERIC, "C");

    QSurfaceFormat fmt;
    fmt.setSamples(16);
    QSurfaceFormat::setDefaultFormat(fmt);

    LauncherForm launcher(&app);
    launcher.show();

    //MyWidget w;
    //w.show();
    //a.installEventFilter(&w);

    return app.exec();
}
