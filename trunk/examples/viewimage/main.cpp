/****************************************************************************
**
** Copyright (C) 2004-2005 Mazatech Inc. All rights reserved.
**
** This file is part of Amanith Framework.
**
** This file may be distributed and/or modified under the terms of the Q Public License
** as defined by Mazatech Inc. of Italy and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Amanith Professional Edition license may use this file in
** accordance with the Amanith Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.mazatech.com or email sales@mazatech.com for
** information about Amanith Commercial License Agreements.
** See http://www.amanith.org/ for opensource version, public forums and news.
**
** Contact info@mazatech.com if any conditions of this licensing are
** not clear to you.
**********************************************************************/

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
