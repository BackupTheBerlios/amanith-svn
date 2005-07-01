/****************************************************************************
** $file: amanith/tools/plugwizard/classtreeform.h   0.1.0.0   edited Jun 30 08:00
**
** Plugin wizard tree form implementation.
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

#include "classtreeform.h"
#include <qpushbutton.h>
#include <qlistview.h>
#include <qvaluevector.h>
#include <qdialog.h>
#include <qheader.h>
#include <qwidget.h>

GBool ClassTreeForm::ShowClassTree(const GKernel &kernel, QString &selected) {

	GDynArray<GProxyState> roots;
	GDynArray<GProxyState> childs;
	QListViewItem *lvi;
	GInt32 i, j, k;

	// QListView settings (list view of classes)
	classView->setRootIsDecorated(true);
	classView->setColumnWidthMode(0, QListView::Maximum);
	classView->header()->setHidden(true);

	kernel.RootClassProxies(roots); // get root class proxies
	k = (GInt32)roots.size();
	for(i = 0; i < k; i++) {
		lvi = new QListViewItem(classView);
		// if it's an external plugin we can't select it
		if (roots[i].External())
			lvi->setEnabled(FALSE);
		lvi->setText(0, roots[i].Proxy()->ClassID().IDName());
		childs.clear();
		kernel.ChildClassProxies(roots[i].Proxy()->ClassID(), childs);
		j = childs.size(); 
		if (j != 0)
			SetChildItems(*lvi, kernel, childs);
	}

	lvi = classView->findItem(selected, 0);
	if(lvi != NULL) {
		classView->setSelected(lvi, true);
		do {
			lvi->setOpen(true);
			lvi = (QListViewItem*)lvi->parent();
		} while(lvi != NULL);
	}

	if(exec() == QDialog::Accepted) { // if something is selected ...
		if (!classView->selectedItem())
			return false;
		selected = classView->selectedItem()->text(0); //... set it
		return true; // is selected
	}
	return false; // cancel (no selected)
}

// set list view
void ClassTreeForm::SetChildItems(QListViewItem &father, 
								  const GKernel &kernel,
								  GDynArray<GProxyState> childs) {
	
	GDynArray<GProxyState> newChilds;
	QListViewItem *lvi;
	GInt32 i, j, k;

	k = (GInt32)childs.size();
	for (i = 0; i < k; i++) {
		lvi = new QListViewItem(&father);
		// if it's an external plugin we can't select it
		if (childs[i].External())
			lvi->setEnabled(FALSE);
		lvi->setText(0, childs[i].Proxy()->ClassID().IDName());
		newChilds.clear();
		kernel.ChildClassProxies(childs[i].Proxy()->ClassID(), newChilds);
		j = newChilds.size();
		if (j != 0)
			SetChildItems(*lvi, kernel, newChilds);
	}
}

void ClassTreeForm::SetOpen(QListViewItem &i) {

	QListViewItem *aux;

	i.setOpen(true);
	aux = i.firstChild();
	while(aux != NULL) {
		SetOpen(*aux);
		aux = aux->nextSibling();
	}
}

void ClassTreeForm::OpenAll() {

	QListViewItem *aux;

	aux = classView->firstChild();
	while(aux != NULL) {
		SetOpen(*aux);
		aux = aux->nextSibling();
	}
}
