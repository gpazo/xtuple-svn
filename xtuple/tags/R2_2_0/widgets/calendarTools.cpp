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

//  calendarTools.cpp
//  Created 03/04/2002 JSL
//  Copyright (c) 2002-2007, OpenMFG, LLC

#include <xsqlquery.h>

#include "calendarTools.h"


CalendarComboBox::CalendarComboBox(QWidget *pParent, const char *pName) :
  XComboBox(pParent, pName)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setAllowNull(TRUE);

  XSqlQuery q;
  q.exec( "SELECT calhead_id, calhead_name "
          "FROM calhead "
          "ORDER BY calhead_name;" );
  populate(q);

  connect(this, SIGNAL(newID(int)), this, SIGNAL(newCalendarId(int)));
}

void CalendarComboBox::load(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("calitem_id(0)", &valid);
  if (valid)
  {
    XSqlQuery q;
    q.prepare( "SELECT acalitem_calhead_id AS calheadid "
               "FROM acalitem "
               "WHERE (acalitem_id=:calitem_id) "
               "UNION SELECT rcalitem_calhead_id AS calheadid "
               "FROM rcalitem "
               "WHERE (rcalitem_id=:calitem_id);" );
    q.bindValue(":calitem_id", param.toInt());
    q.exec();
    if (q.first())
      setId(q.value("calheadid").toInt());

    emit select(pParams);
  }
}


PeriodsListView::PeriodsListView(QWidget *pParent, const char *pName) :
  XListView(pParent, pName)
{
  _calheadid = -1;

  addColumn(tr("Name"),             _itemColumn, Qt::AlignLeft   );
  addColumn(tr("Selected Periods"), -1,          Qt::AlignCenter );
  setSelectionMode(Q3ListView::Extended);
}

void PeriodsListView::populate(int pCalheadid)
{
  XSqlQuery caltype( QString( "SELECT calhead_type "
                              "FROM calhead "
                              "WHERE (calhead_id=%1);" )
                     .arg(pCalheadid) );
  if (caltype.first())
  {
    QString sql;

    _calheadid = pCalheadid;
    clear();

    if (caltype.value("calhead_type").toString() == "A")
      sql = QString( "SELECT acalitem_id, periodstart, periodend, acalitem_name,"
                     "       (formatDate(periodstart) || ' - ' || formatDate(periodend)) "
                     "FROM ( SELECT acalitem_id, acalitem_name,"
                     "              findPeriodStart(acalitem_id) AS periodstart,"
                     "              findPeriodEnd(acalitem_id) AS periodend "
                     "       FROM acalitem "
                     "       WHERE (acalitem_calhead_id=%1) ) AS data "
                     "ORDER BY periodstart;" )
            .arg(pCalheadid);

    else if (caltype.value("calhead_type").toString() == "R")
      sql = QString( "SELECT rcalitem_id, periodstart, periodend, rcalitem_name,"
                     "       (formatDate(periodstart) || ' - ' || formatDate(periodend)) "
                     "FROM ( SELECT rcalitem_id, rcalitem_name,"
                     "              findPeriodStart(rcalitem_id) AS periodstart,"
                     "              findPeriodEnd(rcalitem_id) AS periodend "
                     "       FROM rcalitem "
                     "       WHERE (rcalitem_calhead_id=%1) ) AS data "
                     "ORDER BY periodstart;" )
            .arg(pCalheadid);

    XSqlQuery query(sql);
    while (query.next())
    {
      XListViewItem *last = new PeriodListViewItem( this, this->lastItem(), query.value(0).toInt(),
                                                    query.value(1).toDate(), query.value(2).toDate(),
                                                    query.value(3).toString(), query.value(4).toString() );
      setSelected(last, TRUE);
    }
  }
  else
    _calheadid = -1;
}

void PeriodsListView::getSelected(ParameterList &pParams)
{
  XListViewItem *cursor = firstChild();
  int           counter = 0;

  while (cursor)
  {
    if (cursor->isSelected())
      pParams.append(QString("calitem_id(%1)").arg(counter++), cursor->id());

    cursor = cursor->nextSibling();
  }
}

bool PeriodsListView::isPeriodSelected()
{
  XListViewItem *cursor = firstChild();

  while (cursor)
  {
    if (cursor->isSelected())
      return TRUE;

    cursor = cursor->nextSibling();
  }

  return FALSE;
}

QString PeriodsListView::periodString()
{
  QString     returnString;
  XListViewItem *cursor = firstChild();

  while (cursor)
  {
    if (cursor->isSelected())
    {
      if (returnString.length())
        returnString += ",";

      returnString += QString("%1").arg(cursor->id());
    }

    cursor = cursor->nextSibling();
  }

  return returnString;
}

PeriodListViewItem *PeriodsListView::getSelected(int pIndex)
{
  XListViewItem *cursor;

  if ((cursor = firstChild()))
    do
      if (cursor->isSelected())
        if (--pIndex == 0)
          break;
    while ((cursor = cursor->nextSibling()) != 0);

  if (pIndex == 0)
    return (PeriodListViewItem *)cursor;
  else
    return NULL;
}

void PeriodsListView::load(ParameterList &pParams)
{
  clearSelection();

  XListViewItem *first;
  if ((first = firstChild()))
  {
    QVariant param;
    bool     valid = TRUE;
    for (unsigned int counter = 0; valid; counter++)
    {
      param = pParams.value(QString("calitem_id(%1)").arg(counter), &valid);
      if (valid)
      {
        XListViewItem *cursor = first;
        do
          if (cursor->id() == param.toInt())
            setSelected(cursor, TRUE);
        while ((cursor = cursor->nextSibling()));
      }
    }
  }
}

PeriodListViewItem::PeriodListViewItem( PeriodsListView *parent, XListViewItem *itm, int pId,
                                        QDate pStartDate, QDate pEndDate,
                                        QString s0, QString s1 ) :
  XListViewItem(parent, itm, pId, QVariant(s0), QVariant(s1))
{
  _startDate = pStartDate;
  _endDate = pEndDate;
}

