/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUBSTITUTEAVAILABILITYBYITEM_H
#define DSPSUBSTITUTEAVAILABILITYBYITEM_H

#include "guiclient.h"
#include "xwidget.h"

#include "parameter.h"

#include "ui_dspSubstituteAvailabilityByItem.h"

class dspSubstituteAvailabilityByItem : public XWidget, public Ui::dspSubstituteAvailabilityByItem
{
    Q_OBJECT

public:
    dspSubstituteAvailabilityByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSubstituteAvailabilityByItem();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPrint();
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _showByGroupInt;

};

#endif // DSPSUBSTITUTEAVAILABILITYBYITEM_H
