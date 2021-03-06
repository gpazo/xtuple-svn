/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMS_H
#define ITEMS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_items.h"

class items : public XWidget, public Ui::items
{
    Q_OBJECT

public:
    items(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~items();

public slots:
    virtual void sPopulateMenu( QMenu * );
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList( int pItemid, bool pLocal );
    virtual void sFillList();
    virtual void sSearch( const QString & pTarget );
    virtual void sCopy();

protected slots:
    virtual void languageChange();

};

#endif // ITEMS_H
