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
	UuidGenFrm() : UuidGenFrmBase() 
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
