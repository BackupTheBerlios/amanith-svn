#include <qapplication.h>
#include "mainform.h"

int main(int argc, char **argv) {

    QApplication app(argc, argv);

    MainForm f;

	app.setMainWidget(&f);

    f.show();
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
