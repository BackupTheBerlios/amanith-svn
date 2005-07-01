#include <qapplication.h>
#include "drawpath.h"

int main(int argc, char ** argv) {

    QApplication app(argc, argv);
    QGLWidgetTest test;

	app.setMainWidget(&test);
	// if you want fullscreen support, plese uncomment the following line
	// test.setWindowState(test.windowState() ^ Qt::WindowFullScreen);
	test.show();
	test.setFocus();
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
