/****************************************************************************
** $file: amanith/tools/plugwizard/mainform.cpp   0.1.0.0   edited Jun 30 08:00
**
** Plugin wizard main form implementation.
**
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

#include "mainform.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <quuid.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>

#include "classtreeform.h"

// constructor
MainForm::MainForm() : mainFormBase() {

	GInt32 i;
	GProxyState p;
	GError err;
	QString s;
	GInt32 numC;
	const QListBox *lbox;
	QListBoxItem *lbItem;

	gKernel = new GKernel();

	lbox = baseClass->listBox();
	numC = gKernel->ProxiesCount();

	for (i = 0; i < numC; i++) {
		err = gKernel->Proxy(i, p);
		if (err == G_NO_ERROR) {
			baseClass->insertItem(p.Proxy()->ClassID().IDName());
			lbItem = lbox->item(i);
			// if it's an external plugin we can't select it
			if (p.External())
				lbItem->setSelectable(FALSE);
		}
	}

	baseClass->listBox()->sort();
	i = lbox->index(lbox->findItem("GElement"));
	baseClass->setCurrentItem(i);

	// License
	gLicense = "";
	connect(dirButton, SIGNAL(clicked()), this, SLOT(selectDir()));
	connect(createButton, SIGNAL(clicked()), this, SLOT(create()));
	connect(classTreeButton, SIGNAL(clicked()), this, SLOT(classTree()));
	connect(dirEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changeHappened()));
	connect(classNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changeHappened()));
	connect(classNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(genUuid()));
}

// destructor
MainForm::~MainForm() {

	if (gKernel)
		delete gKernel;
}

// a field is changed
void MainForm::changeHappened() {
	if (classNameEdit->text().length() > 0 && dirEdit->text().length() > 0)
		createButton->setEnabled(true);
	else
		createButton->setEnabled(false);
}

// when class name change, gen a new UUID
void MainForm::genUuid() {

	if (classNameEdit->text().length() > 0) {
		QString uuid = QUuid::createUuid().toString();
		QString str_uuid, fp, sp, tp, fop;
		fp = uuid.mid(1, 8);
		sp = uuid.mid(10, 4);
		sp += uuid.mid(15, 4);
		fp = fp.upper();
		sp = sp.upper();
		tp = uuid.mid(20, 4);
		tp += uuid.mid(25, 4);
		fop = uuid.mid(29, 8);
		tp = tp.upper();
		fop = fop.upper();
		str_uuid += "GClassID(\"" + classNameEdit->text();
		str_uuid += "\", 0x";
		str_uuid += fp;
		str_uuid += ", 0x";
		str_uuid += sp;
		str_uuid += ", 0x";
		str_uuid += tp;
		str_uuid += ", 0x";
		str_uuid += fop;
		str_uuid += ")";
		uuidEdit->setText(str_uuid);
	}
	else
		uuidEdit->setText("");
}

// select the output directory
void MainForm::selectDir() {

	dirButton->setEnabled(false);
	QString dir = QFileDialog::getExistingDirectory(QDir::currentDirPath());
	dirEdit->setText(dir);
	dirButton->setEnabled(true);
}

// create files
void MainForm::create() {

	// first check if class name begins with 'G' or 'g'
	QString className = classNameEdit->text();
	if (className[0] != 'G') {
		if (QMessageBox::warning(this, "Coding standard violation",
								"Class name does not start with G letter. Would you like to continue?",
								QMessageBox::Yes, QMessageBox::No)== QMessageBox::No)
			return;
	}

	// Include license
	if (license->isChecked()) {
		QFile f("fileheader.txt");
		if (f.open(IO_ReadOnly)) {
			QTextStream stream(&f);
			gLicense = stream.read();
			f.close();
		}
	}
	else
		gLicense = "";
	// Create .h file
	createDotH();
	// Create .cpp file
	createDotCPP();
	QMessageBox::information(this, classNameEdit->text(), "Class created successfully.");
}

// create .h file
void MainForm::createDotH() {

	QString h = gLicense;
	GInt32 i, j;

	// License
	QString aux = "$file: " + dirEdit->text() + "/";
	aux += classNameEdit->text().lower() + ".h   " + QString(G_VERSION_STR) + "   edited " ;
	aux += QDate::currentDate().toString("MMM dd") + " ";
	aux += QTime::currentTime().toString("hh:mm");
	h.replace("[FILE_HEADER]", aux);

	// IfnDef
	aux = classNameEdit->text().upper();
	h += QString("\n") + "#ifndef " + aux + "_H\n";
	h += "#define " + classNameEdit->text().upper() + "_H\n";

	// Base class include
	h += "\n#include \"" + baseClass->currentText().lower() + ".h\"\n\n";

	// Doxygen file brief description
	h += "/*!\n";
	h += "\t\\file " + classNameEdit->text().lower() + ".h\n";
	h += "\t\\brief Header file for " + classNameEdit->text() + " class.\n";
	h += "*/\n";

	// Namespace
	h += "\nnamespace Amanith {\n";

	// Class 
	h += "\n\t// *********************************************************************\n";
	h += "\t//";
	for(i = 0, j = (70 - classNameEdit->text().length()) / 2; i < j; i++)
		h += " ";
	h += classNameEdit->text() + "\n";
	h += "\t// *********************************************************************\n\n";
	h += "\t//! " + classNameEdit->text() + " static class descriptor.\n";
	h += "\tstatic const GClassID G_" + classNameEdit->text().mid(1, classNameEdit->text().length() - 1).upper(); 
	h += "_CLASSID = " + uuidEdit->text() + ";\n";

	h += "\n\tclass G_EXPORT " + classNameEdit->text() + " : public " + baseClass->currentText() + " {\n\n";
	h += "\tprotected:\n";
	h += "\t\t//! Cloning function\n";
	h += "\t\tGError BaseClone(const GElement& Source);\n\n";
	h += "\tpublic:\n";
	h += "\t\t//! Default constructor\n";
	h += "\t\t" + classNameEdit->text() + "();\n";
	h += "\t\t//! Constructor with owner (kernel) specification\n";
	h += "\t\t" + classNameEdit->text() + "(const GElement* Owner);\n";
	h += "\t\t//! Destructor\n";
	h += "\t\t~" + classNameEdit->text() + "();\n";
	h += "\t\t//! Get class descriptor\n";
	h += "\t\tconst GClassID& ClassID() const {\n";
	h += "\t\t\treturn G_" + classNameEdit->text().mid(1, classNameEdit->text().length() - 1).upper();
	h += "_CLASSID;\n";
	h += "\t\t}\n";
	h += "\t\t//! Get base class (father class) descriptor\n";
	h += "\t\tconst GClassID& DerivedClassID() const {\n";
	h += "\t\t\treturn G_" + baseClass->currentText().mid(1, baseClass->currentText().length() - 1).upper();
	h += "_CLASSID;\n";
	h += "\t\t}\n";
	h += "\t}; // end class " + classNameEdit->text() + "\n\n";

	// Class proxy
	h += "\n\t// *********************************************************************\n";
	h += "\t//";
	for(i = 0, j = (67 - classNameEdit->text().length()) / 2; i < j; i++)
		h += " ";
	h += classNameEdit->text() + "Proxy\n";
	h += "\t// *********************************************************************\n\n";

	h += "\t/*!\n";
	h += "\t\t\\class " + classNameEdit->text() + "Proxy\n";
	h += "\t\t\\brief This class implements a " + classNameEdit->text() + " proxy (provider).\n";
	h += "\t*/\n";

	h += "\tclass G_EXPORT " + classNameEdit->text() + "Proxy : public GElementProxy {\n";
	h += "\tpublic:\n";

	h += "\t\t/*!\n";
	h += "\t\t\tCreate a new element.\n\n";
	h += "\t\t\t\\param Owner the owner (kernel) of the created element. Default value is NULL.\n";
	h += "\t\t\t\\return A new created element if operation succeeds, else a NULL pointer.\n";
	h += "\t\t*/\n";

	h += "\t\tGElement* CreateNew(const GElement* Owner = NULL) {\n";
	h += "\t\t\treturn new " + classNameEdit->text() + "(Owner);\n";
	h += "\t\t}\n";
	h += "\t\t//! Get class descriptor of elements type \"provided\" by this proxy.\n";
	h += "\t\tconst GClassID& ClassID() const {\n";
	h += "\t\t\treturn G_" + classNameEdit->text().mid(1, classNameEdit->text().length() - 1).upper();
	h += "_CLASSID;\n";
	h += "\t\t}\n";
	h += "\t\t//! Get base class (father class) descriptor of elements type \"provided\" by this proxy.\n";
	h += "\t\tconst GClassID& DerivedClassID() const {\n";
	h += "\t\t\treturn G_" + baseClass->currentText().mid(1, baseClass->currentText().length() - 1).upper();
	h += "_CLASSID;\n";
	h += "\t\t}\n";
	h += "\t}; // end class " + classNameEdit->text() + "Proxy\n\n";

	h += "\t//! Static proxy for " + classNameEdit->text() + " class.\n";
	h += "\tstatic const " + classNameEdit->text() + "Proxy G_" +  classNameEdit->text().mid(1, classNameEdit->text().length() - 1).upper();
	h += "_PROXY;\n";

	// End namespace
	h += "\n}; // end namespace Amanith\n\n";

	// EndIf
	h += "#endif\n";

	// Save file .h
	QString filePath = dirEdit->text() + QDir::separator() + classNameEdit->text().lower() + ".h";
	if(QFile::exists(filePath)) {
		i = QMessageBox::information(this, classNameEdit->text().lower() + ".h", "File already exists!\nDo you want overwrite it?", "Yes", "No");
		if(i == 1)
			return;
	}

	QFile file( filePath );
    if (file.open(IO_WriteOnly)) {
        QTextStream stream(&file);
		stream << h;
        file.close();
    }
}

// create .cpp file
void MainForm::createDotCPP() {

	QString cpp = gLicense;
	GInt32 i, j;

	// License
	QString aux = "$file: " + dirEdit->text() + "/";
	aux += classNameEdit->text().lower() + ".cpp   0.1.0   edited " ;
	aux += QDate::currentDate().toString("MMM dd") + " ";
	aux += QTime::currentTime().toString("hh:mm");
	cpp.replace("[FILE_HEADER]", aux);

	// Include
	cpp += "\n#include \"" + classNameEdit->text().lower() + ".h\"\n\n";

	// Doxygen file brief description
	cpp += "/*!\n";
	cpp += "\t\\file " + classNameEdit->text().lower() + ".cpp\n";
	cpp += "\t\\brief Implementation file for " + classNameEdit->text() + " class.\n";
	cpp += "*/\n";

	// Namespace
	cpp += "\nnamespace Amanith {\n";
	cpp += "\n// *********************************************************************\n";
	cpp += "//";
	for (i = 0, j = (70 - classNameEdit->text().length()) / 2; i < j; i++)
		cpp += " ";
	cpp += classNameEdit->text() + "\n";
	cpp += "// *********************************************************************\n";

	// Class implementation
	cpp += "\n// constructor\n";
	cpp += classNameEdit->text() + "::" + classNameEdit->text() + "() : " + baseClass->currentText() + "() {\n"; 
	cpp += "}\n";

	cpp += "\n// constructor\n";
	cpp += classNameEdit->text() + "::" + classNameEdit->text() + "(const GElement* Owner) : " + baseClass->currentText() + "(Owner) {\n"; 
	cpp += "}\n";

	cpp += "\n// destructor\n";
	cpp += classNameEdit->text() + "::~" + classNameEdit->text() + "() {\n"; 
	cpp += "}\n\n";

	cpp += "// cloning function\n";
	cpp += "GError " + classNameEdit->text() + "::BaseClone(const GElement& Source) {\n\n"; 
	cpp += "\t// don't forget to do a real implementation!\n";
	cpp += "\tconst " + classNameEdit->text() + "& src = (const " + classNameEdit->text() + "&)Source;\n\n";
	cpp += "\t// continue cloning passing the control to base (father) class\n";
	cpp += "\t" + baseClass->currentText() + "::BaseClone(Source);\n";
	cpp += "}\n";

	// End namespace
	cpp += "\n}; // end namespace Amanith\n";

	// Save file
	QString filePath = dirEdit->text() + QDir::separator() + classNameEdit->text().lower() + ".cpp";
	if (QFile::exists(filePath)) {
		i = QMessageBox::information(this, classNameEdit->text().lower() + ".cpp", "File already exists!\nDo you want overwrite it?", "Yes", "No");
		if (i == 1)
			return;
	}

	QFile file(filePath);
    if (file.open(IO_WriteOnly)) {
        QTextStream stream( &file );
		stream << cpp;
        file.close();
    }
}

void MainForm::classTree() {

	ClassTreeForm form(this);
	GInt32 i;
	QString s = baseClass->currentText();

	// Open the dialog e select a class
	if (form.ShowClassTree(*gKernel, s)) {
		for (i = 0; i < baseClass->count(); i++) {
			if (s == baseClass->text(i)) {
				baseClass->setCurrentItem(i);
				break;
			}
		}
	}
}
