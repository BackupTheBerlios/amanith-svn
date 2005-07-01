#include "drawer.h"
#include <amanith/gimpexp.h>
#include "resizeform.h"
#include "canvasresizeform.h"
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcombobox.h>

// constructor
QPixmapViewer::QPixmapViewer(QWidget *parent, const char *name, int wFlags ) : QScrollView(parent, name, wFlags) {

	gPixmap = NULL;
}
//------------------------------------------------------------

// destructor
QPixmapViewer::~QPixmapViewer() {
}

//------------------------------------------------------------

void QPixmapViewer::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {

	GInt32 x, y, f = 0;

	if (!gPixmap) {
		p->fillRect(clipx, clipy, clipw, cliph, QColor(255, 255, 255));
		return;
	}
	if (clipw > gPixmap->width()) {
		x = (clipw - gPixmap->width()) / 2;
		f = 1;
	}
	else
		x = 0;
	if (cliph > gPixmap->height()) {
		y = (cliph - gPixmap->height()) / 2;
		f = 1;
	}
	else
		y = 0;

	if (f) {
		p->fillRect(clipx, clipy, clipw, cliph, QColor(255, 255, 255));
	}
	p->drawPixmap(x, y, *gPixmap);
}

//----------------------------------------------------------------------

ImageViewer::ImageViewer( QWidget *parent, const char *name, int wFlags ): QMainWindow(parent, name, wFlags) {

	GError err;
	GUInt32 i, j, k, w;

	// create new kernel
	gKernel = new GKernel();
	// create new image
	gPixMap = (GPixelMap *)gKernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!gPixMap)
		abort();
	// build path for data (textures)
	gDataPath = SysUtils::AmanithPath();
	if (gDataPath.length() > 0)
		gDataPath += "data/";

	gImage = NULL;
	gQPixmap = new QPixmap();

	gPixmapViewer = new QPixmapViewer(this, "PixViewer", Qt::WNoAutoErase);
	gPixmapViewer->gPixmap = gQPixmap;

	// build input/output filters to use with dialogs
	GDynArray<GImpExpFeature> features;
	err = gKernel->ImpExpFeatures(G_PIXELMAP_CLASSID, features);
	if (err == G_NO_ERROR) {
		j = features.size();
		for (i = 0; i < j; i++) {
			w = features[i].FileExtensionsCount();
			for (k = 0; k < w; k++) {
				GString ext = features[i].FileExtension(k);
				if (features[i].ReadWriteOp() == G_IMPEXP_READ || features[i].ReadWriteOp() == G_IMPEXP_READWRITE)
					gLoadFilter += " *." + ext;
				if (features[i].ReadWriteOp() == G_IMPEXP_WRITE || features[i].ReadWriteOp() == G_IMPEXP_READWRITE)
					gSaveFilter += " *." + ext;
			}
		}
	}

	menubar = new QMenuBar(this);
	menubar->setSeparator(QMenuBar::InWindowsStyle);

	file = new QPopupMenu( menubar );
	menubar->insertItem( "&File", file );
	file->insertItem( "&Open...", this, SLOT(OpenFile()), CTRL + Key_O);
	file->insertItem( "&Save...", this, SLOT(SaveFile()), CTRL + Key_S);
	file->insertSeparator();
	//file->insertItem( "E&xit", qApp, SLOT(quit()), CTRL + Key_Q);
	file->insertItem( "E&xit", this, SLOT(close()), CTRL + Key_Q);

	edit =  new QPopupMenu( menubar );
	menubar->insertItem( "&Edit", edit );
	edit->insertItem("&Horizontal flip", this, SLOT(HorizontalFlip()));
	edit->insertItem("&Vertical flip", this, SLOT(VerticalFlip()));
	edit->insertItem("&Negative image", this, SLOT(NegativeImage()));
	edit->insertItem("&Trace contours", this, SLOT(TraceContours()));
	edit->insertItem("&Resize canvas", this, SLOT(ResizeCanvas()));
	edit->insertItem("&Resize", this, SLOT(FilteredResize()));
	edit->insertItem("&Edge enhancement", this, SLOT(EdgeEnhance()));
	edit->insertItem("&Edge preserving smooth", this, SLOT(EdgePreservingSmooth()));

	setCentralWidget(gPixmapViewer);
	// check for little/big endian
	GSystemInfo sysInfo;

	SysUtils::SystemInfo(sysInfo);
	if (sysInfo.EndianType == G_BIG_ENDIAN)
		LittleEndian = G_FALSE;
	else
		LittleEndian = G_TRUE;
}

ImageViewer::~ImageViewer() {

	if (gPixMap)
		delete gPixMap;
	if (gImage)
		delete gImage;
	if (gKernel)
		delete gKernel;
	if (gQPixmap)
		delete gQPixmap;
}

void ImageViewer::OpenFile() {

	if (gLoadFilter.length() == 0) {
		QMessageBox::critical(this, "Load image from file", "No input formats supported!");
		return;
	}

	GString filter = "Images (" + gLoadFilter + ")";

	QString newfilename = QFileDialog::getOpenFileName(StrUtils::ToAscii(gDataPath), StrUtils::ToAscii(filter), this);
	if (!newfilename.isEmpty()) {
		LoadImage(newfilename);
		repaint();
		gPixmapViewer->repaintContents(G_FALSE);
	}
}

void ImageViewer::ConvertPixmap() {

	if (!gPixMap) {
		if (gImage) {
			delete gImage;
			gImage = NULL;
		}
		gPixmapViewer->gPixmap = NULL;
	}
	else {
		if (gImage) {
			delete gImage;
			gImage = NULL;
		}

		if (gPixMap->IsHighColor() || gPixMap->IsGrayScale())
			gPixMap->SetPixelFormat(G_R8G8B8);
		if (LittleEndian)
			gImage = new QImage(gPixMap->Pixels(), gPixMap->Width(), gPixMap->Height(),	32, NULL, 1, QImage::LittleEndian);
		else
			gImage = new QImage(gPixMap->Pixels(), gPixMap->Width(), gPixMap->Height(),	32, NULL, 1, QImage::BigEndian);
		gQPixmap->convertFromImage(*gImage);
		gPixmapViewer->gPixmap = gQPixmap;
		gPixmapViewer->resizeContents(gPixMap->Width(), gPixMap->Height());
		delete gImage;
		gImage = NULL;
	}
}


GError ImageViewer::LoadImage(const QString& FileName) {

	GError err;

	if (!FileName.isEmpty()) {
		QApplication::setOverrideCursor(waitCursor);

		err = gPixMap->Load(FileName.ascii(), "expandpalette=true");
		if (err == G_NO_ERROR) {
			if (!LittleEndian)
				gPixMap->ReverseChannels(G_TRUE);

			ConvertPixmap();

			GString s = StrUtils::ExtractFileName(GString(FileName.ascii()));
			setCaption(StrUtils::ToAscii(s));
			resize(gPixMap->Width(), gPixMap->Height());
		}
		else {
			update();
		}
		QApplication::restoreOverrideCursor();	// restore original cursor
		return err;
	}
	else
		return G_INVALID_PARAMETER;
}


/*
The resize event handler, if a valid pixmap was loaded it will call
scale() to fit the pixmap to the new widget size.
*/

void ImageViewer::resizeEvent(QResizeEvent *) {

	if (!gPixMap || gPixMap->PixelsCount() <= 0)
		gPixmapViewer->resizeContents(0, 0);
	else
		gPixmapViewer->resizeContents(gPixMap->Width(), gPixMap->Height());
}


void ImageViewer::HorizontalFlip() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		gPixMap->Flip(G_TRUE, G_FALSE);
		ConvertPixmap();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Canvas resize", "There is no image loaded");
}

void ImageViewer::VerticalFlip() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		gPixMap->Flip(G_FALSE, G_TRUE);
		ConvertPixmap();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Vertical flip", "There is no image loaded");
}

void ImageViewer::NegativeImage() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		gPixMap->Negative();
		ConvertPixmap();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Negative image", "There is no image loaded");
}

void ImageViewer::TraceContours() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		QApplication::setOverrideCursor(waitCursor);
		gPixMap->TraceContours(20, G_FALSE);
		ConvertPixmap();
		QApplication::restoreOverrideCursor();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Trace contours", "There is no image loaded");	
}

void ImageViewer::ResizeCanvas() {

	CanvasResizeForm f(this, "Resize_Canvas", TRUE);

	if (gPixMap && gPixMap->PixelsCount() > 0) {

		if (f.exec() == QDialog::Accepted) {
			GInt32 top = f.TopSpinBox->value();
			GInt32 bottom = f.BottomSpinBox->value();
			GInt32 left = f.LeftSpinBox->value();
			GInt32 right = f.RightSpinBox->value();
			gPixMap->ResizeCanvas(top, bottom, left, right);
			ConvertPixmap();
			gPixmapViewer->repaintContents(FALSE);
		}
	}
	else
		QMessageBox::information(this, "Canvas resize", "There is no image loaded");
}

void ImageViewer::FilteredResize() {

	ResizeForm f(this, "Resize_Canvas", TRUE);

	if (gPixMap && gPixMap->PixelsCount() > 0) {

		GString s = "Current dimensions: " + StrUtils::ToString(gPixMap->Width());
		s += " x " + StrUtils::ToString(gPixMap->Height());
		f.DimsLabel->setText(StrUtils::ToAscii(s));

		f.NewWidth->setValue(gPixMap->Width());
		f.NewHeight->setValue(gPixMap->Height());

		if (f.exec() == QDialog::Accepted) {

			GInt32 w = f.NewWidth->value();
			GInt32 h = f.NewHeight->value();
			QApplication::setOverrideCursor(waitCursor);

			switch (f.FilterComboBox->currentItem()) {
				case 0:
					gPixMap->Resize(w, h, G_RESIZE_BOX);
					break;
				case 1:
					gPixMap->Resize(w, h, G_RESIZE_TRIANGLE);
					break;
				case 2:
					gPixMap->Resize(w, h, G_RESIZE_QUADRATIC);
					break;
				case 3:
					gPixMap->Resize(w, h, G_RESIZE_CUBIC);
					break;
				case 4:
					gPixMap->Resize(w, h, G_RESIZE_CATMULLROM);
					break;
				case 5:
					gPixMap->Resize(w, h, G_RESIZE_GAUSSIAN);
					break;
				case 6:
					gPixMap->Resize(w, h, G_RESIZE_SINC);
					break;
				case 7:
					gPixMap->Resize(w, h, G_RESIZE_BESSEL);
					break;
				case 8:
					gPixMap->Resize(w, h, G_RESIZE_BELL);
					break;
				case 9:
					gPixMap->Resize(w, h, G_RESIZE_HANNING);
					break;
				case 10:
					gPixMap->Resize(w, h, G_RESIZE_HAMMING);
					break;
				case 11:
					gPixMap->Resize(w, h, G_RESIZE_BLACKMAN);
					break;
				case 12:
					gPixMap->Resize(w, h, G_RESIZE_NORMAL);
					break;
			}
			ConvertPixmap();
			QApplication::restoreOverrideCursor();
			gPixmapViewer->repaintContents(FALSE);
		}
	}
	else
		QMessageBox::information(this, "Image resize", "There is no image loaded");
}

void ImageViewer::EdgeEnhance() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		QApplication::setOverrideCursor(waitCursor);
		gPixMap->EdgeEnhance();
		ConvertPixmap();
		QApplication::restoreOverrideCursor();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Edge enhancement", "There is no image loaded");
}

void ImageViewer::EdgePreservingSmooth() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {
		QApplication::setOverrideCursor(waitCursor);
		gPixMap->EdgePreservingSmooth();
		ConvertPixmap();
		QApplication::restoreOverrideCursor();
		gPixmapViewer->repaintContents(FALSE);
	}
	else
		QMessageBox::information(this, "Edge-preserving smooth", "There is no image loaded");
}

void ImageViewer::SaveFile() {

	if (gPixMap && gPixMap->PixelsCount() > 0) {

		if (gSaveFilter.length() == 0) {
			QMessageBox::critical(this, "Save image to file", "No input formats supported!");
			return;
		}

		GString filter = "Images (" + gSaveFilter + ")";

		QString newfilename = QFileDialog::getSaveFileName(StrUtils::ToAscii(gDataPath), StrUtils::ToAscii(filter), this);
		if (!newfilename.isEmpty()) {
			if (!LittleEndian)
				gPixMap->ReverseChannels(G_TRUE);
			gPixMap->Save(newfilename.ascii());
			if (!LittleEndian)
				gPixMap->ReverseChannels(G_TRUE);
			gPixmapViewer->repaintContents(G_FALSE);
		}
	}
	else
		QMessageBox::information(this, "Canvas resize", "There is no image loaded");
}
