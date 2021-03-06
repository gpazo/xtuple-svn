/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTACTS_H
#define CONTACTS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_contacts.h"

class contacts : public XWidget, public Ui::contacts
{
    Q_OBJECT

public:
    contacts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~contacts();

    virtual void setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem* = NULL);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPrint();
    virtual void sFillList();
    virtual void sAttach();
    virtual void sDetach();

protected slots:
    virtual void languageChange();

};

#endif // CONTACTS_H
