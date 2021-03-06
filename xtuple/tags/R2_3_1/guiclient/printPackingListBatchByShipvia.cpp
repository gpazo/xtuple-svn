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

#include "printPackingListBatchByShipvia.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "mqlutil.h"

printPackingListBatchByShipvia::printPackingListBatchByShipvia(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  ParameterList params;
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery mql = mqlLoad(":/sr/forms/printPackingListBatchByShipvia/ShipVia.mql");
  q = mql.toQuery(params);

  _shipvia->populate(q);
  if (q.lastError().type() != QSqlError::None)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

printPackingListBatchByShipvia::~printPackingListBatchByShipvia()
{
    // no need to delete child widgets, Qt does it all for us
}

void printPackingListBatchByShipvia::languageChange()
{
    retranslateUi(this);
}

void printPackingListBatchByShipvia::sPrint()
{
  QPrinter printer;
  bool     setupPrinter = TRUE;

  XSqlQuery prtd;
  QString prts("UPDATE pack SET pack_printed=TRUE"
               " WHERE ((pack_head_id=<? value(\"head_id\") ?>) "
	       "   AND  (pack_head_type=<? value(\"head_type\") ?>)"
	       "<? if exists(\"shiphead_id\") ?>"
	       "   AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
	       "<? else ?>"
	       "   AND (pack_shiphead_id IS NULL)"
	       "<? endif ?>"
	       ");" );


  XSqlQuery packq;
  ParameterList params;
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  params.append("shipvia", _shipvia->currentText());
  MetaSQLQuery packm = mqlLoad(":/sr/forms/printPackingListBatchByShipvia/ToPrint.mql");
  packq = packm.toQuery(params);
  if (packq.lastError().type() != QSqlError::None)
  {
    systemError(this, packq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  while (packq.next())
  {
    // leave sohead_id and cosmisc_id for compatibility with existing reports
    ParameterList params;
    params.append("head_id",   packq.value("pack_head_id").toInt());
    params.append("head_type", packq.value("pack_head_type").toString());
    if (packq.value("pack_head_type").toString() == "SO")
      params.append("sohead_id", packq.value("pack_head_id").toInt());
    else if (packq.value("pack_head_type").toString() == "TO")
      params.append("tohead_id", packq.value("pack_head_id").toInt());
    if (! packq.value("pack_shiphead_id").isNull())
    {
      params.append("cosmisc_id", packq.value("pack_shiphead_id").toInt());
      params.append("shiphead_id",  packq.value("pack_shiphead_id").toInt());
    }
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    orReport report(packq.value( packq.value("pack_shiphead_id").isNull() ?
			      "pickform" : "packform").toString(), params);
    if (report.isValid())
    {
      if (report.print(&printer, setupPrinter))
        setupPrinter = FALSE;
      else
      {
	orReport::endMultiPrint(&printer);
        return;
      }
    }
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }

    MetaSQLQuery mql(prts);
    prtd = mql.toQuery(params);
    if (prtd.lastError().type() != QSqlError::None)
    {
      systemError(this, prtd.lastError().databaseText(), __FILE__, __LINE__);
      orReport::endMultiPrint(&printer);
      return;
    }

  }
  orReport::endMultiPrint(&printer);
}
