#include <stdio.h>
#include <stdlib.h>
#include <amanith/gkernel.h>
#include <amanith/gerror.h>
#include <amanith/geometry/gxform.h>
#include <amanith/2d/gpixelmap.h>
#include <amanith/geometry/gdistance.h>
#include <amanith/geometry/gintersect.h>
#include <amanith/support/gutilities.h>
#include <amanith/gpluglib.h>

using namespace Amanith;

GKernel *kernel;
GString dataPath;

void TestStrUtils() {

	GString path1 = "./";
	GString path2 = "../../10.5.7/hello/";
	GString file1 = "../../10.5.7/hello/file";
	GString file2 = "../..\\10.5.7/hello\\hello.txt";
	GString file3 = "zeus.tar.gz";
	GString s;

	s = StrUtils::ExtractFilePath(path1);
	printf("File path of %s is %s\n", StrUtils::ToAscii(path1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFilePath(path2);
	printf("File path of %s is %s\n", StrUtils::ToAscii(path2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFilePath(file1);
	printf("File path of %s is %s\n", StrUtils::ToAscii(file1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFilePath(file2);
	printf("File path of %s is %s\n", StrUtils::ToAscii(file2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFilePath(file3);
	printf("File path of %s is %s\n", StrUtils::ToAscii(file3), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileExt(path1);
	printf("File ext of %s is %s\n", StrUtils::ToAscii(path1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileExt(path2);
	printf("File ext of %s is %s\n", StrUtils::ToAscii(path2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileExt(file1);
	printf("File ext of %s is %s\n", StrUtils::ToAscii(file1), StrUtils::ToAscii(s));
 	s = StrUtils::ExtractFileExt(file2);
	printf("File ext of %s is %s\n", StrUtils::ToAscii(file2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileExt(file3);
	printf("File ext of %s is %s\n", StrUtils::ToAscii(file3), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(path1);
	printf("File name of %s is %s\n", StrUtils::ToAscii(path1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(path2);
	printf("File name of %s is %s\n", StrUtils::ToAscii(path2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file1);
	printf("File name of %s is %s\n", StrUtils::ToAscii(file1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file2);
	printf("File name of %s is %s\n", StrUtils::ToAscii(file2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file3);
	printf("File name of %s is %s\n", StrUtils::ToAscii(file3), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(path1, G_FALSE);
	printf("Base file name of %s is %s\n", StrUtils::ToAscii(path1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(path2, G_FALSE);
	printf("Base file name of %s is %s\n", StrUtils::ToAscii(path2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file1, G_FALSE);
	printf("Base file name of %s is %s\n", StrUtils::ToAscii(file1), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file2, G_FALSE);
	printf("Base file name of %s is %s\n", StrUtils::ToAscii(file2), StrUtils::ToAscii(s));
	s = StrUtils::ExtractFileName(file3, G_FALSE);
	printf("Base file name of %s is %s\n", StrUtils::ToAscii(file3), StrUtils::ToAscii(s));
}

void TestSysInfo() {

	GSystemInfo sInfo;
	GString encGUID, decGUID;

	if (SysUtils::SystemInfo(sInfo)) {
		printf("\n\nSystem info:\n");
		printf("    Word size: %d\n", sInfo.WordSize);
		if (sInfo.EndianType == G_BIG_ENDIAN)
			printf("    Endian type: Big Endian\n");
		else
			printf("    Endian type: Little Endian\n");
		printf("    Path delimiter: %c\n", sInfo.TrailerPathDelimiter);
	}
	else
		printf("\n\nI cannot find a correct system info! Strange machine...\n\n");

	printf("Amanith is installed in: %s\n", StrUtils::ToAscii(SysUtils::AmanithPath()));
	printf("Amanith plugins directory is: %s\n\n", StrUtils::ToAscii(SysUtils::AmanithPluginsPath()));
}


void TestPixelMap() {

	GPixelMap *img;
	GError err;
	GString fName = dataPath + "spiral.png";

	img = (GPixelMap *)kernel->CreateNew(G_PIXELMAP_CLASSID);
	if (!img) {
		printf("Error creating GPixelMap: used kernel does not support it, or plugin not found.");
		return;
	}
	// try loading a jpeg image
	err = img->Load(StrUtils::ToAscii(fName), NULL, "png");

	if (err != G_NO_ERROR)
		printf("Error loading spiral.png. Error is: %s\n", StrUtils::ToAscii(ErrorUtils::ErrToString(err)));
	else {
		printf("Load spiral.png ok!\n");
		printf("    Width: %d\n", img->Width());
		printf("    Height: %d\n", img->Height());
		printf("    ColorDepth: %d\n", img->BitsPerPixel());
	}
	delete img;
}

void TestIntersect() {

	GPoint2 p1, p2, p3;
	GPoint3 q1, q2, q3;
	GVector2 v1, v2, v3;
	GVector3 w1, w2, w3;
	GRay2 r2, R2;
	GRay3 r3, R3;
	GLineSegment2 ls2;
	GLineSegment3 ls3;
	GAABox2 aabox2, AABOX2;
	GAABox3 aabox3, AABOX3;
	GOOBox2 oobox2;
	GOOBox3 oobox3, OOBOX3;
	GSphere2 sph2, SPH2;
	GSphere3 sph3, SPH3;
	GPlane pln, PLN;
	GBool b;
	GUInt32 f;
	GReal locParams[2];
	GBoxSide boxSide;
	GReal a, c;

	c = -1;
	a = GMath::Abs(c);

	// aabox - oobox (2D)
	aabox2.SetMinMax(GPoint2(-6.5, 6.5), GPoint2(-5.5, 7.5));
	oobox2.SetCenter(GPoint2(-4, 5));
	oobox2.SetHalfDimension(G_X, 2 * GMath::Sqrt((GReal)2));
	oobox2.SetHalfDimension(G_Y, GMath::Sqrt((GReal)2));
	oobox2.SetAxis(G_X, GVector2(1, 1));
	oobox2.SetAxis(G_Y, GVector2(-1, 1));
	b = Intersect(aabox2, oobox2);

	// aabox - oobox (3D)
	aabox3.SetMinMax(GPoint3(0, 0, 0), GPoint3(10, 10, 10));

	oobox3.SetCenter(GPoint3(-1, 0, 5));
	oobox3.SetHalfDimension(G_X, (GReal)1.4143 / 2);
	oobox3.SetHalfDimension(G_Y, (GReal)1.4143 / 2);
	oobox3.SetHalfDimension(G_Z, (GReal)1.4143 / 2);
	oobox3.SetAxis(G_X, GVector3(1, 1, 0));
	oobox3.SetAxis(G_Y, GVector3(-1, 1, 0));
	oobox3.SetAxis(G_Z, GVector3(0, 0, 1));
	b = Intersect(aabox3, oobox3);

	// line segment - aabox (2D)
	ls2.SetStartPoint(GPoint2(0.5, 0));
	ls2.SetEndPoint(GPoint2(-0.5, 1));
	aabox2.SetMinMax(GPoint2(-1, -1), GPoint2(1, 1));
	b = Intersect(ls2, aabox2, f, locParams, boxSide);

	// plane - aabox (3D)
	aabox3.SetMinMax(GPoint3(0, 0, 0), GPoint3(1000, 1000, 1000));
	pln.SetNormal(GVector3(0, 1, 0));
	pln.SetOffset(-G_EPSILON);
	b = Intersect(pln, aabox3);

}

void TestDistance() {

	GPoint2 p1, p2, p3;
	GPoint3 q1, q2, q3;
	GVector2 v1, v2, v3;
	GVector3 w1, w2, w3;
	GRay2 r2, R2;
	GRay3 r3, R3;
	GAABox2 aabox2, AABOX2;
	GAABox3 aabox3, AABOX3;
	GOOBox2 oobox2;
	GOOBox3 oobox3;
	GSphere2 sph2, SPH2;
	GSphere3 sph3, SPH3;
	GPlane pln, PLN;
	GReal d;

	// point - point
	p1.Set(2, 2);
	p2.Set(-1, -2);
	d = Distance(p1, p2);

	// point - ray (2D)
	p1.Set(G_EPSILON, -4);
	r2.SetOrigin(GPoint2(-1, -2));
	r2.SetDirection(GVector2(-1, 2));
	r2.Normalize();
	d = SignedDistance(p1, r2);

	// point - ray (3D)
	q1.Set(-2, 0, 1);
	r3.SetOrigin(GPoint3(-1, -2, 1));
	r3.SetDirection(GVector3(-1, 2, 0));
	r3.Normalize();
	d = Distance(q1, r3);

	// ray - ray (2D)
	r2.SetOrigin(GPoint2(2, 2));
	r2.SetDirection(GVector2(1, 1));
	r2.Normalize();
	R2.SetOrigin(GPoint2(3, 1));
	R2.SetDirection(GVector2(1, 1));
	R2.Normalize();
	d = SignedDistance(r2, R2);

	// ray - ray (3D)
	r3.SetOrigin(GPoint3(0, 0, 0));
	r3.SetDirection(GVector3(1, 0, 0));
	r3.Normalize();
	R3.SetOrigin(GPoint3(2, 2, 2));
	R3.SetDirection(GVector3(0, -1, -1));
	R3.Normalize();
	d = SignedDistance(r3, R3);

	// point - plane
	q1.Set(0, 0, -2);
	pln.SetNormal(GVector3(0, 0, -1));
	pln.SetOffset(1);
	d = SignedDistance(q1, pln);

	// plane - ray
	r3.SetOrigin(GPoint3(1, 0, -1));
	r3.SetDirection(GVector3(1, 0, 0));
	r3.Normalize();
	pln.SetNormal(GVector3(0, 1, 0));
	pln.SetOffset(0);
	d = SignedDistance(r3, pln);

	// plane - plane
	pln.SetNormal(GVector3(0, 1, 0));
	pln.SetOffset(0);
	PLN.SetNormal(GVector3(1, 0, 0));
	PLN.SetOffset(0);
	d = Distance(pln, PLN);

	// point - sphere (2D & 3D)
	p1.Set(-1, 2);
	sph2.SetCenter(GPoint2(2, 2));
	sph2.SetRadius(2);
	d = SignedDistance(p1, sph2);

	// ray - sphere (2D & 3D)
	r2.SetOrigin(GPoint2(2, 2));
	r2.SetDirection(GVector2(0, 1));
	r2.Normalize();
	sph2.SetCenter(GPoint2(2, 2));
	sph2.SetRadius(2);
	d = SignedDistance(r2, sph2);

	// sphere - plane
	pln.SetNormal(GVector3(0, -1, 0));
	pln.SetOffset(4);
	sph3.SetCenter(GPoint3(0, 0, 0));
	sph3.SetRadius(2);
	d = SignedDistance(pln, sph3);

	// sphere - sphere (2D & 3D)
	sph2.SetCenter(GPoint2(5, 5));
	sph2.SetRadius(1);
	SPH2.SetCenter(GPoint2(5, 2));
	SPH2.SetRadius(1);
	d = SignedDistance(sph2, SPH2);

	// point - aabox (2D)
	p1.Set((GReal)0.8, (GReal)0.2);
	aabox2.SetMinMax(GPoint2(-1, -1), GPoint2(1, 1));
	d = SignedDistance(p1, aabox2);

	// point - aabox (3D)
	q1.Set((GReal)0.5, (GReal)0.5, (GReal)0.5);
	aabox3.SetMinMax(GPoint3(-1, -1, -1), GPoint3(1, 1, 1));
	d = SignedDistance(q1, aabox3);

	// ray - aabox (2D)
	r2.SetOrigin(GPoint2(-1, 3));
	r2.SetDirection(GVector2(1, -1));
	r2.Normalize();
	aabox2.SetMinMax(GPoint2(-1, -1), GPoint2(3, 3));
	d = SignedDistance(r2, aabox2);

	r2.SetOrigin(GPoint2((GReal)-3000000000.0, (GReal)3000000000.0));
	r2.SetDirection(GVector2(1.0, -1.0));
	r2.Normalize();
	aabox2.SetMinMax(GPoint2(-3.0, -3.0), GPoint2(3.0, 3.0));
	d = SignedDistance(r2, aabox2);


	// ray - aabox (3D)
	r3.SetOrigin(GPoint3(-10, 0.5, -0.5));
	r3.SetDirection(GVector3(1, 0, 0));
	r3.Normalize();
	aabox3.SetMinMax(GPoint3(-1, -1, -1), GPoint3(1, 1, 1));
	d = SignedDistance(r3, aabox3);

	// plane - aabox (3D)
	pln.SetNormal(GVector3(1, 1, 0));
	pln.SetOffset(G_EPSILON);
	aabox3.SetMinMax(GPoint3(-1, -1, -1), GPoint3(1, 1, 1));
	d = SignedDistance(pln, aabox3);

	// sphere - aabox (2D & 3D)
	sph2.SetCenter(GPoint2(2, 2));
	sph2.SetRadius(2);
	aabox2.SetMinMax(GPoint2(5, 4), GPoint2(7, 8));
	d = SignedDistance(sph2, aabox2);

	// aabox - aabox (2D)
	aabox2.SetMinMax(GPoint2(1, 1), GPoint2(4, 4));
	AABOX2.SetMinMax(GPoint2(-1, 1), GPoint2(0, 2));
	d = SignedDistance(AABOX2, aabox2);

	// aabox - aabox (3D)
	aabox3.SetMinMax(GPoint3(-2, -2, -2), GPoint3(2, 2, 2));
	AABOX3.SetMinMax(GPoint3(2, 2, 0), GPoint3(6, 6, -6));
	d = SignedDistance(AABOX3, aabox3);

	// point - oobbox (2D)
	p1.Set(1, 0);
	oobox2.SetCenter(GPoint2(0, 0));
	oobox2.SetHalfDimension(G_X, 1);
	oobox2.SetHalfDimension(G_Y, 1);
	oobox2.SetAxis(G_X, GVector2(1, 1));
	oobox2.SetAxis(G_Y, GVector2(-1, 1));
	d = SignedDistance(p1, oobox2);

	// ray - oobox (2D)
	r2.SetOrigin(GPoint2(-1, 1));
	r2.SetDirection(GVector2(-1, 1));
	r2.Normalize();
	oobox2.SetCenter(GPoint2(0, 0));
	oobox2.SetHalfDimension(G_X, 1);
	oobox2.SetHalfDimension(G_Y, 1);
	oobox2.SetAxis(G_X, GVector2(1, 1));
	oobox2.SetAxis(G_Y, GVector2(-1, 1));
	d = SignedDistance(r2, oobox2);

	// plane - oobox
	pln.SetNormal(GVector3(0, 1, 0));
	pln.SetOffset(0);
	oobox3.SetCenter(GPoint3(0, 2, 0));
	oobox3.SetHalfDimension(G_X, 1);
	oobox3.SetHalfDimension(G_Y, 1);
	oobox3.SetHalfDimension(G_Z, 1);
	oobox3.SetAxis(G_X, GVector3(1, 1, 0));
	oobox3.SetAxis(G_Y, GVector3(-1, 1, 0));
	oobox3.SetAxis(G_Z, GVector3(0, 0, 1));
	d = SignedDistance(pln, oobox3);
}

int main(void) {

	kernel = new GKernel();
	if (!kernel)
		return -1;

	dataPath = SysUtils::AmanithPath();
	if (dataPath.length() <= 0)
		return -2;

	dataPath += "data/";
	printf("\n\n");
	TestStrUtils();
	TestSysInfo();
	TestPixelMap();

	TestIntersect();
	delete kernel;
	return 0;
}

