/****************************************************************************
** $file: amanith/tools/plugwizard/classtreeform.h   0.1.0.0   edited Jun 30 08:00
**
** Plugin wizard tree form definition.
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

#ifndef CLASSTREEFORM_H
#define CLASSTREEFORM_H

#include "classtreeformbase.h"
#include <qpushbutton.h>
// QT4 support
#ifdef USE_QT4
	#include <q3listview.h>
	#include <q3valuevector.h>
#else
	#include <qlistview.h>
	#include <qvaluevector.h>
#endif
#include <amanith/gglobal.h>
#include <amanith/gkernel.h>

using namespace Amanith;

class ClassTreeForm : public ClassTreeFormBase {

	Q_OBJECT

protected:
// QT4 support
#ifdef USE_QT4
	void SetChildItems(Q3ListViewItem &father, const GKernel &kernel, GDynArray<GProxyState> childs);
	void SetOpen(Q3ListViewItem &i);
#else
	void SetChildItems(QListViewItem &father, const GKernel &kernel, GDynArray<GProxyState> childs);
	void SetOpen(QListViewItem &i);
#endif

public:
	ClassTreeForm(QWidget *parent) : ClassTreeFormBase(parent, "ClassTreeForm", true) { 
		connect(expandButton, SIGNAL(clicked()), this, SLOT(OpenAll()));
	}

	GBool ShowClassTree(const GKernel &kernel, QString & selected);

public slots:
	void OpenAll();
};

#endif
