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

#ifndef UUIDGENFRM_H
#define UUIDGENFRM_H

#include "uuidgenfrmbase.h"
#include <quuid.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qstring.h>

class UuidGenFrm : public UuidGenFrmBase
{
	Q_OBJECT

public:
	UuidGenFrm() : UuidGenFrmBase(0, 0, FALSE, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu) 
	{
		connect(GenerateButton, SIGNAL(clicked()), this, SLOT(generateUuid()));
		generateUuid();
	}

public slots:
	void generateUuid()
	{
		QString uuid = QUuid::createUuid().toString();
		lineEdit1->setText(uuid);
		QString str_uuid, fp, sp, tp, fop;
		fp = uuid.mid(1,8);
		sp = uuid.mid(10,4);
		sp += uuid.mid(15,4);
		fp = fp.upper();
		sp = sp.upper();
	
		tp = uuid.mid(20,4);
		tp += uuid.mid(25,4);
		fop = uuid.mid(29,8);
		tp = tp.upper();
		fop = fop.upper();

		str_uuid += "GClassID(0x";
		str_uuid += fp;
		str_uuid += ", 0x";
		str_uuid += sp;
		str_uuid += ", 0x";
		str_uuid += tp;
		str_uuid += ", 0x";
		str_uuid += fop;
		str_uuid += ")";

		lineEdit2->setText(str_uuid);
		lineEdit2->setFocus();
		lineEdit2->selectAll();
	}
};

#endif
