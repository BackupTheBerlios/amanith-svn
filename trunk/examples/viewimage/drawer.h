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
	#include <q3scrollview.h>
	#include <q3filedialog.h>
	#include <q3mainwindow.h>
	#include <QResizeEvent>
	#include <Q3PopupMenu>
#else
	#include <qscrollview.h>
	#include <qfiledialog.h>
	#include <qmainwindow.h>
#endif

using namespace Amanith;

class QPixmapViewer : public QScrollView {

protected:
	void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph);

public:
	QPixmap *gPixmap;

	// constructor
	QPixmapViewer(QWidget *parent, const char *name, Qt::WFlags wFlags);
	// destructor
	~QPixmapViewer();
};

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


	QMenuBar   *menubar;

	QPopupMenu  *file;
	QPopupMenu  *edit;

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
