/****************************************************************************
** $file: amanith/tools/plugwizard/mainform.h   0.1.0.0   edited Jun 30 08:00
**
** Plugin wizard main form definition.
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

#ifndef MAINFORM_H
#define MAINFORM_H

#include "mainformbase.h"
#include <amanith/gkernel.h>

using namespace Amanith;

class MainForm : public mainFormBase {
	Q_OBJECT

private:
	QString gLicense;
	GKernel *gKernel;

protected:
	void createDotH();
	void createDotCPP();

public:
	MainForm();
	~MainForm();

public slots:
	void changeHappened();
	void genUuid();
	void selectDir();
	void create();
	void classTree();
};

#endif
