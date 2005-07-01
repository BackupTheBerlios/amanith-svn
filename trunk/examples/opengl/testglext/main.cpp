#include <qapplication.h>
#include "testext.h"

int main(int argc, char ** argv) {

    QApplication a(argc, argv);
    QGLWidgetTest test;

	a.setMainWidget(&test);
	test.show();
	test.setFocus();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
