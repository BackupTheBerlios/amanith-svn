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

#ifndef IMAGE_DRAWER_H
#define IMAGE_DRAWER_H

#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qapplication.h>

// QT4 support
#ifdef USE_QT4
	//#include <q3scrollview.h>
	#include <QScrollArea>
	//#include <q3filedialog.h>
	#include <QFileDialog>
	//#include <q3mainwindow.h>
	#include <QMainWindow>
	#include <QResizeEvent>
	//#include <Q3PopupMenu>
	#include <QMenu>
#else
	#include <qscrollview.h>
	#include <qfiledialog.h>
	#include <qmainwindow.h>
#endif

using namespace Amanith;

class QPixmapViewer : public QScrollArea {

protected:
	void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph);

public:
	QPixmap *gPixmap;

	// constructor
	QPixmapViewer(QWidget *parent, const char *name, Qt::WFlags wFlags);
	// destructor
	~QPixmapViewer();
};

// QT4 support
class ImageViewer : public QMainWindow {

	Q_OBJECT

public:

	ImageViewer(QWidget *parent=0, const char *name = 0, Qt::WFlags wFlags = 0);
	~ImageViewer();
	GError LoadImage(const QString& FileName);

protected:

	void resizeEvent(QResizeEvent *);
	void ConvertPixmap();

private:
	GString gDataPath;
	GKernel *gKernel;
	GPixelMap *gPixMap;
	QImage *gImage;
	QPixmap *gQPixmap;
	GString gLoadFilter;
	GString gSaveFilter;
	GBool LittleEndian;

	QPixmapViewer *gPixmapViewer;

	QMenuBar *menubar;
	QMenu *file;
	QMenu *edit;

private slots:

	void HorizontalFlip();
	void VerticalFlip();
	void NegativeImage();
	void TraceContours();
	void ResizeCanvas();
	void FilteredResize();
	void EdgeEnhance();
	void EdgePreservingSmooth();

	void OpenFile();
	void SaveFile();
};

#endif
