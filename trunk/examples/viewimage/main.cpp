#include <qapplication.h>
#ifdef USE_QT4
	#include "drawer_qt4.h"
#else
	#include "drawer.h"
#endif

int main(int argc, char ** argv) {

    QApplication app(argc, argv);

	ImageViewer *w =  new ImageViewer(0, "New window", Qt::WDestructiveClose | Qt::WResizeNoErase);

#if QT_VERSION < 0x040000
	w->setIcon(QPixmap::fromMimeSource("amanith32.png"));
#endif

	app.setMainWidget(w);
	w->setCaption("Amanith - Image Example");
	w->show();

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
