#include <qapplication.h>
#include "drawbezier.h"

int main(int argc, char ** argv) {

    QApplication app(argc, argv);
    QGLWidgetTest test;

	app.setMainWidget(&test);
	// if you want full screen support, please uncomment the following line
	// test.setWindowState(test.windowState() ^ Qt::WindowFullScreen);
	test.show();
	test.setFocus();
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
