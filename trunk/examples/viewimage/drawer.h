#ifndef IMAGE_DRAWER_H
#define IMAGE_DRAWER_H

#include <amanith/gkernel.h>
#include <amanith/2d/gpixelmap.h>
#include <qscrollview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qmainwindow.h>

using namespace Amanith;

class QPixmapViewer : public QScrollView {

private:

protected:
	void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph);

public:
	QPixmap *gPixmap;

	// constructor
	QPixmapViewer(QWidget *parent, const char *name, int wFlags);
	// destructor
	~QPixmapViewer();
};


class ImageViewer : public QMainWindow {

	Q_OBJECT

public:

	ImageViewer(QWidget *parent=0, const char *name=0, int wFlags=0);
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

	// Menu item ids
	//int si, sp, ac, co, mo, fd, bd,	td, ta, ba, fa, au, ad, dd,	ss, cc, t1, t8, t32;

private slots:

	void HorizontalFlip();
	void VerticalFlip();
	void NegativeImage();
	void TraceContours();
	void ResizeCanvas();
	void FilteredResize();
	void EdgeEnhance();
	void EdgePreservingSmooth();

	//void	newWindow();
	void OpenFile();
	void SaveFile();
};

#endif
