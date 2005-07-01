#include <qapplication.h>
#include "drawer.h"

int main(int argc, char ** argv) {

    QApplication app(argc, argv);
    //QPixmapViewer *test = new QPixmapViewer();

	/*app.setMainWidget(test);
	// if you want fullscreen support, plese uncomment the following line
	// test.setWindowState(test.windowState() ^ Qt::WindowFullScreen);
	test->show();
	test->setFocus();*/

	ImageViewer *w =  new ImageViewer(0, "New window", Qt::WDestructiveClose | Qt::WResizeNoErase);
	w->setIcon(QPixmap::fromMimeSource("amanith32.png"));

	app.setMainWidget(w);
	w->setCaption("Amanith - Image Example");
	w->show();

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
