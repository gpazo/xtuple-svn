/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSINGLELEVELBOM_H
#define DSPSINGLELEVELBOM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSingleLevelBOM.h"

class dspSingleLevelBOM : public XWidget, public Ui::dspSingleLevelBOM
{
    Q_OBJECT

public:
    dspSingleLevelBOM(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSingleLevelBOM();

    virtual bool setParams(ParameterList &params);

public slots:
    virtual void sEdit();
    virtual void sView();
    virtual void sPopulateMenu( QMenu * );
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPrint();
    virtual void sFillList();
    virtual void sFillList( int, bool );

protected slots:
    virtual void languageChange();

};

#endif // DSPSINGLELEVELBOM_H
