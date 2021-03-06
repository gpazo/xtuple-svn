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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "todoListCalendar.h"

#include "xdialog.h"
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>
#include <calendargraphicsitem.h>

#include "todoCalendarControl.h"
#include "storedProcErrorLookup.h"

todoListCalendar::todoListCalendar(QWidget* parent, Qt::WindowFlags f)
  : XWidget(parent, f)
{
  setupUi(this);

  todoCalendarControl * cc = new todoCalendarControl(this);
  QGraphicsScene * scene = new QGraphicsScene(this);
  CalendarGraphicsItem * calendar = new CalendarGraphicsItem(cc);
  calendar->setSelectedDay(QDate::currentDate());
  scene->addItem(calendar);

  _gview->setScene(scene);
  _gview->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

  _list->addColumn(tr("Type"),    _statusColumn,  Qt::AlignCenter, true, "type");
  _list->addColumn(tr("Seq"),        _seqColumn,  Qt::AlignRight,  true, "seq");
  _list->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  _list->addColumn(tr("User"),      _userColumn,  Qt::AlignLeft,   true, "usr");
  _list->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  _list->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "descrip");
  _list->addColumn(tr("Status"),  _statusColumn,  Qt::AlignLeft,   true, "status");
  _list->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  _list->addColumn(tr("Incident"), _orderColumn,  Qt::AlignLeft,   true, "incdt");
  _list->addColumn(tr("Customer"), _orderColumn,  Qt::AlignLeft,   true, "cust");
  _list->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");

  sFillList(QDate::currentDate());

  connect(cc, SIGNAL(selectedDayChanged(QDate)), this, SLOT(sFillList(QDate)));
}

void todoListCalendar::languageChange()
{
  retranslateUi(this);
}

enum SetResponse todoListCalendar::set(const ParameterList& /*pParams*/)
{
/*
  QVariant param;
  bool           valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    handlePrivs();
    sFillList();
  }
*/
  return NoError;
}

void todoListCalendar::sFillList(const QDate & date)
{
  QString sql = "SELECT todoitem_id AS id, todoitem_usr_id AS altId, todoitem_owner_username AS owner, "
                "       'T' AS type, incdtpriority_order AS seq, incdtpriority_name AS priority, "
                "       todoitem_name AS name, "
                "       firstLine(todoitem_description) AS descrip, "
                "       todoitem_status AS status, todoitem_due_date AS due, "
                "       usr_username AS usr, incdt_number AS incdt, cust_number AS cust, "
                "       CASE WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
                "            WHEN (todoitem_status != 'C'AND "
                "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
                "       END AS due_qtforegroundrole "
                "FROM usr, todoitem LEFT OUTER JOIN incdt ON (incdt_id=todoitem_incdt_id) "
                "                   LEFT OUTER JOIN crmacct ON (crmacct_id=todoitem_crmacct_id) "
                "                   LEFT OUTER JOIN cust ON (cust_id=crmacct_cust_id) "
                "                   LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
                "WHERE ( (todoitem_usr_id=usr_id)"
                "  AND   (todoitem_due_date = <? value(\"date\") ?>)"
                "  <? if not exists(\"completed\") ?>"
                "  AND   (todoitem_status != 'C')"
                "  <? endif ?>"
                "  <? if exists(\"usr_id\") ?> "
                "  AND (todoitem_usr_id=<? value(\"usr_id\") ?>) "
                "  <? elseif exists(\"usr_pattern\" ?>"
                "  AND (todoitem_usr_id IN (SELECT usr_id "
                "        FROM usr "
                "        WHERE (usr_username ~ <? value(\"usr_pattern\") ?>))) "
                "  <? endif ?>"
                "  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
                "       ) "
                "ORDER BY due, seq, usr;";

  ParameterList params;
  params.append("date", date);

  MetaSQLQuery mql(sql);
  XSqlQuery itemQ = mql.toQuery(params);

  _list->populate(itemQ, true);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void todoListCalendar::resizeEvent(QResizeEvent* event)
{
  XWidget::resizeEvent(event);

  _gview->setMinimumWidth(_gview->height() * (_gview->scene()->sceneRect().width() / _gview->scene()->sceneRect().height()));
  _gview->fitInView(_gview->scene()->sceneRect(), Qt::KeepAspectRatio);
}
