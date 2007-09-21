/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "rptSingleLevelBOM.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>

/*
 *  Constructs a rptSingleLevelBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptSingleLevelBOM::rptSingleLevelBOM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_showExpired, SIGNAL(toggled(bool)), _expiredDaysLit, SLOT(setEnabled(bool)));
    connect(_showExpired, SIGNAL(toggled(bool)), _expiredDays, SLOT(setEnabled(bool)));
    connect(_showFuture, SIGNAL(toggled(bool)), _effectiveDaysLit, SLOT(setEnabled(bool)));
    connect(_showFuture, SIGNAL(toggled(bool)), _effectiveDays, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptSingleLevelBOM::~rptSingleLevelBOM()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptSingleLevelBOM::languageChange()
{
    retranslateUi(this);
}


void rptSingleLevelBOM::init()
{
  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased);
  _item->setFocus();
}

enum SetResponse rptSingleLevelBOM::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("expiredDays", &valid);
  if (valid)
  {
    _showExpired->setChecked(TRUE);
    _expiredDays->setValue(param.toInt());
  }

  param = pParams.value("futureDays", &valid);
  if (valid)
  {
    _showFuture->setChecked(TRUE);
    _effectiveDays->setValue(param.toInt());
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError;
  }

  return NoError;
}

void rptSingleLevelBOM::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());

  if(_showFuture->isChecked())
    params.append("futureDays", _effectiveDays->value());

  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }

  if (_captive)
    accept();
  else
  {
    _item->setId(-1);
    _item->setFocus();
  }
}

