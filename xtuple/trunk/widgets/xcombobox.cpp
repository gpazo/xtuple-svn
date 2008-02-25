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

//  xcombobox.cpp
//  Created 03/16/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QLabel>
#include <QMouseEvent>

#include <xsqlquery.h>
#include "xcombobox.h"

XComboBox::XComboBox(QWidget *pParent, const char *pName) :
  QComboBox(pParent, pName)
{
  _type = Adhoc;
  _lastId = -1;
  setAllowNull(false);
  _nullStr = "";
  _label = 0;

  connect(this, SIGNAL(activated(int)), this, SLOT(sHandleNewIndex(int)));

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

XComboBox::XComboBox(bool pEditable, QWidget *pParent, const char *pName) :
  QComboBox(pEditable, pParent, pName)
{
  _type = Adhoc;
  _lastId = -1;
  setAllowNull(false);
  _label = 0;

  connect(this, SIGNAL(activated(int)), this, SLOT(sHandleNewIndex(int)));

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

enum XComboBox::XComboBoxTypes XComboBox::type()
{
  return _type;
}

void XComboBox::setType(XComboBoxTypes pType)
{
  if (_type == pType)
    return;

  _type = pType;

  if (_x_metrics == 0)
    return;

  XSqlQuery query;

  switch (pType)
  {
    case Adhoc:
      break;

    case UOMs:
      setAllowNull(TRUE);
      query.exec( "SELECT uom_id, uom_name "
                  "FROM uom "
                  "ORDER BY uom_name;" );
    break;

    case ClassCodes:
      query.exec( "SELECT classcode_id, (classcode_code || '-' || classcode_descrip) "
                  "FROM classcode "
                  "ORDER BY classcode_code;" );
      break;

    case ItemGroups:
      query.exec( "SELECT itemgrp_id, itemgrp_name "
                  "FROM itemgrp "
                  "ORDER BY itemgrp_name;" );
      break;

    case CostCategories:
      query.exec( "SELECT costcat_id, (costcat_code || '-' || costcat_descrip) "
                  "FROM costcat "
                  "ORDER BY costcat_code;" );
      break;

    case ProductCategories:
      query.exec( "SELECT prodcat_id, (prodcat_code || ' - ' || prodcat_descrip) "
                  "FROM prodcat "
                  "ORDER BY prodcat_code;" );
      break;

    case PlannerCodes:
      query.exec( "SELECT plancode_id, (plancode_code || '-' || plancode_name) "
                  "FROM plancode "
                  "ORDER BY plancode_code;" );
      break;

    case CustomerTypes:
      query.exec( "SELECT custtype_id, (custtype_code || '-' || custtype_descrip) "
                  "FROM custtype "
                  "ORDER BY custtype_code;" );
      break;

    case CustomerGroups:
      query.exec( "SELECT custgrp_id, custgrp_name "
                  "FROM custgrp "
                  "ORDER BY custgrp_name;" );
      break;

    case VendorTypes:
      query.exec( "SELECT vendtype_id, (vendtype_code || '-' || vendtype_descrip) "
                  "FROM vendtype "
                  "ORDER BY vendtype_code;" );
      break;

    case VendorGroups:
      query.exec( "SELECT vendgrp_id, vendgrp_name "
                  "FROM vendgrp "
                  "ORDER BY vendgrp_name;" );
      break;

    case SalesRepsActive:
      query.exec( "SELECT salesrep_id, (salesrep_number || '-' || salesrep_name) "
                  "FROM salesrep "
                  "WHERE (salesrep_active) "
                  "ORDER by salesrep_number;" );
      break;

    case ShipVias:
      setAllowNull(TRUE);
      setEditable(TRUE);
      query.exec( "SELECT shipvia_id, (shipvia_code || '-' || shipvia_descrip) "
                  "FROM shipvia "
                  "ORDER BY shipvia_code;" );
      break;

    case SalesReps:
      query.exec( "SELECT salesrep_id, (salesrep_number || '-' || salesrep_name) "
                  "FROM salesrep "
                  "ORDER by salesrep_number;" );
      break;

    case ShippingCharges:
      query.exec( "SELECT shipchrg_id, (shipchrg_name || '-' || shipchrg_descrip) "
                  "FROM shipchrg "
                  "ORDER by shipchrg_name;" );
      break;

    case ShippingForms:
      query.exec( "SELECT shipform_id, shipform_name "
                  "FROM shipform "
                  "ORDER BY shipform_name;" );
      break;

    case Terms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip) "
                  "FROM terms "
                  "ORDER by terms_code;" );
      break;

    case ARTerms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip) "
                  "FROM terms "
                  "WHERE (terms_ar) "
                  "ORDER by terms_code;" );
      break;

    case APTerms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip) "
                  "FROM terms "
                  "WHERE (terms_ap) "
                  "ORDER by terms_code;" );
      break;

    case ARBankAccounts:
      query.exec( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip) "
                  "FROM bankaccnt "
                  "WHERE (bankaccnt_ar) "
                  "ORDER BY bankaccnt_name;" );
      break;

    case APBankAccounts:
      query.exec( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip) "
                  "FROM bankaccnt "
                  "WHERE (bankaccnt_ap) "
                  "ORDER BY bankaccnt_name;" );
      break;

    case AccountingPeriods:
      query.exec( "SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end)) "
                  "FROM period "
                  "ORDER BY period_start DESC;" );
      break;

    case FinancialLayouts:
      query.exec( "SELECT flhead_id, flhead_name "
                  "FROM flhead "
                  "WHERE (flhead_active) "
                  "ORDER BY flhead_name;" );
      break;

    case FiscalYears:
      query.exec( "SELECT yearperiod_id, formatdate(yearperiod_start) || '-' || formatdate(yearperiod_end)"
                  "  FROM yearperiod"
                  " ORDER BY yearperiod_start DESC;" );
      break;

    case SoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name) "
                  "FROM prj "
                  "WHERE (prj_so) "
                  "ORDER BY prj_name;" );
      break;

    case WoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name) "
                  "FROM prj "
                  "WHERE (prj_wo) "
                  "ORDER BY prj_name;" );
      break;

    case PoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name) "
                  "FROM prj "
                  "WHERE (prj_po) "
                  "ORDER BY prj_name;" );
      break;

    case Currencies:
      query.exec( "SELECT curr_id, currConcat(curr_abbr, curr_symbol)"
                  " FROM curr_symbol "
                  "ORDER BY curr_base DESC, curr_abbr;" );
      break;

    case CurrenciesNotBase:
      query.exec( "SELECT curr_id, currConcat(curr_abbr, curr_symbol)"
                  " FROM curr_symbol "
                  " WHERE curr_base = FALSE "
                  "ORDER BY curr_abbr;" );
      break;

    case Companies:
      query.exec( "SELECT company_id, company_number "
                  "FROM company "
                  "ORDER BY company_number;" );
      break;

    case ProfitCenters:
      setEditable(_x_metrics->boolean("GLFFProfitCenters"));
      query.exec( "SELECT prftcntr_id, prftcntr_number "
                  "FROM prftcntr "
                  "ORDER BY prftcntr_number;" );
      break;

    case Subaccounts:
      setEditable(_x_metrics->boolean("GLFFSubaccounts"));
      query.exec( "SELECT subaccnt_id, subaccnt_number "
                  "FROM subaccnt "
                  "ORDER BY subaccnt_number;" );
      break;

    case CustomerCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'C')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case VendorCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'V')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case ItemCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'I')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case ProjectCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'J')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case LotSerialCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'L')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case AllCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name "
                  "FROM cmnttype "
                  "ORDER BY cmnttype_name;" );
      break;

    case AllProjects:
      query.exec( "SELECT prj_id, prj_name "
                  "FROM prj "
                  "ORDER BY prj_name;" );
      break;

    case Users:
      query.exec( "SELECT usr_id, usr_username "
                  "FROM usr "
                  "ORDER BY usr_username;" );
      break;

    case SalesCategories:
      query.exec( "SELECT salescat_id, (salescat_name || '-' || salescat_descrip) "
                  "FROM salescat "
                  "ORDER BY salescat_name;" );
      break;

    case ExpenseCategories:
      query.exec( "SELECT expcat_id, (expcat_code || '-' || expcat_descrip) "
                  "FROM expcat "
                  "ORDER BY expcat_code;" );
      break;

    case ReasonCodes:
      query.exec( "SELECT rsncode_id, (rsncode_code || '-' || rsncode_descrip)"
                  "  FROM rsncode "
                  "ORDER BY rsncode_code;" );
      break;

    case TaxCodes:
      query.exec( "SELECT tax_id, (tax_code || '-' || tax_descrip)"
                  "  FROM tax "
                  "ORDER BY tax_code;" );
      break;

    case WorkCenters:
      query.exec( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip)"
                  "  FROM wrkcnt "
                  "ORDER BY wrkcnt_code;" );
      break;

    case CRMAccounts:
      setAllowNull(TRUE);
      query.exec( "SELECT crmacct_id, (crmacct_number || '-' || crmacct_name)"
                  "  FROM crmacct "
                  "ORDER BY crmacct_number;" );
      break;

    case Honorifics:
      setAllowNull(TRUE);
      query.exec( "SELECT hnfc_id, hnfc_code"
                  "  FROM hnfc "
                  "ORDER BY hnfc_code;" );
      break;

    case IncidentSeverity:
      query.exec( "SELECT incdtseverity_id, incdtseverity_name"
                  "  FROM incdtseverity"
                  " ORDER BY incdtseverity_order, incdtseverity_name;" );
      break;

    case IncidentPriority:
      query.exec( "SELECT incdtpriority_id, incdtpriority_name"
                  "  FROM incdtpriority"
                  " ORDER BY incdtpriority_order, incdtpriority_name;" );
      break;

    case IncidentResolution:
      query.exec( "SELECT incdtresolution_id, incdtresolution_name"
                  "  FROM incdtresolution"
                  " ORDER BY incdtresolution_order, incdtresolution_name;" );
      break;

    case IncidentCategory:
      query.exec( "SELECT incdtcat_id, incdtcat_name"
                  "  FROM incdtcat"
                  " ORDER BY incdtcat_order, incdtcat_name;" );
      break;

    case TaxAuths:
      query.exec( "SELECT taxauth_id, taxauth_code"
                  "  FROM taxauth"
                  " ORDER BY taxauth_code;" );
      break;

    case TaxTypes:
      query.exec( "SELECT taxtype_id, taxtype_name"
                  "  FROM taxtype"
                  " ORDER BY taxtype_name;" );
      break;

    case Agent:
      query.exec( "SELECT usr_id, usr_username "
                  "  FROM usr"
                  " WHERE (usr_agent) "
                  " ORDER BY usr_username;" );
      break;

    case Reports:
      query.exec( "SELECT a.report_id, a.report_name "
                  "FROM report a, "
                  "    (SELECT MIN(report_grade) AS report_grade, report_name "
                  "     FROM report "
                  "     GROUP BY report_name) b "
                  "WHERE ((a.report_name=b.report_name)"
                  "  AND  (a.report_grade=b.report_grade)) "
                  "ORDER BY report_name;" );
      break;

    case OpportunityStages:
      query.exec("SELECT opstage_id, opstage_name "
                 "  FROM opstage"
                 " ORDER BY opstage_order;");
      break;

    case OpportunitySources:
      query.exec("SELECT opsource_id, opsource_name "
                 "  FROM opsource;");
      break;

    case OpportunityTypes:
      query.exec("SELECT optype_id, optype_name "
                 "  FROM optype;");
      break;

  }

  populate(query);

  switch (pType)
  {
    case SoProjects:
    case WoProjects:
    case PoProjects:
      setEnabled(count() > 1);
      break;

    case Currencies:
      if (count() <= 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    case CurrenciesNotBase:
      if (count() < 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setLabel(QLabel* pLab)
{
  _label = pLab;

  switch (_type)
  {
    case Currencies:
      if (count() <= 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    case CurrenciesNotBase:
      if (count() < 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setId(int pTarget)
{
  // reports are a special case: they should really be stored by name, not id
  if (_type == Reports)
  {
    XSqlQuery query;
    query.prepare("SELECT report_id "
                  "FROM report "
                  "WHERE (report_name IN (SELECT report_name "
                  "                       FROM report "
                  "                       WHERE (report_id=:report_id)));");
    query.bindValue(":report_id", pTarget);
    query.exec();
    while (query.next())
    {
      int id = query.value("report_id").toInt();
      for (int counter = 0; counter < count(); counter++)
      {
        if (_ids.at(counter) == id)
        {
          setCurrentItem(counter);

          if(_lastId!=id)
          {
            _lastId = id;
            emit newID(pTarget);
            emit valid(TRUE);

            if (allowNull())
              emit notNull(TRUE);
          }

          return;
        }
      }
    }
  }
  else
  {
    for (int counter = 0; counter < count(); counter++)
    {
      if (_ids.at(counter) == pTarget)
      {
        setCurrentItem(counter);

        if(_lastId!=pTarget)
        {
          _lastId = pTarget;
          emit newID(pTarget);
          emit valid(TRUE);

          if (allowNull())
            emit notNull(TRUE);
        }

        return;
      }
    }
  }

  setNull();
}

void XComboBox::setText(QVariant &pVariant)
{
  XComboBox::setText(pVariant.toString());
}

void XComboBox::setText(const QString &pString)
{
  if (count())
  {
    for (int counter = ((allowNull()) ? 1 : 0); counter < count(); counter++)
  {
      if (text(counter) == pString)
      {
        setCurrentItem(counter);
        return;
      }
    }
  }

  if (editable())
  {
    setId(-1);
    setCurrentText(pString);
  }
}

void XComboBox::setAllowNull(bool pAllowNull)
{
  _allowNull = pAllowNull;
}

void XComboBox::setNull()
{
  if (allowNull())
  {
    _lastId = -1;

    setCurrentItem(0);
    emit newID(-1);
    emit valid(FALSE);
    emit notNull(FALSE);
  }
}

void XComboBox::setNullStr(const QString& pNullStr)
{
  _nullStr = pNullStr;
  if (allowNull())
    setItemText(0, pNullStr);
}

void XComboBox::setText(const QVariant &pVariant)
{
  setText(pVariant.toString());
}

void XComboBox::setEditable(bool pEditable)
{
/*
  QWidget    *before = NULL;
  QWidget    *after  = NULL;

// Find the focus proxy before and after this
  QFocusData *fd = focusData();
  for (QWidget *cursor = fd->first();; cursor=fd->next())
  {
    if (cursor == this)
    {
      before = fd->prev();
      fd->next();
      after = fd->next();
      break;
    }
    else if (cursor == fd->last())
      break;
  }
*/

//  Set the editable state
  QComboBox::setEditable(pEditable);

/*
//  If possible, reset the tab order
  if (before)
  {
    setTabOrder(before, this);
    setTabOrder(this, after);
  }
*/
}

bool XComboBox::editable() const
{
  return QComboBox::editable();
}

void XComboBox::clear()
{
  QComboBox::clear();

  if (_ids.count())
    _ids.clear();

  if (allowNull())
    append(-1, _nullStr);
}

// a hack to repopulate the combobox to fix MC bug 3698
void XComboBox::populate()
{
    enum XComboBoxTypes tmpType = _type;
    setType(Adhoc);
    setType(tmpType);
}

void XComboBox::populate(XSqlQuery &pQuery, int pSelected)
{
  int selected = 0;
  int counter  = 0;

//  Clear any old data
  clear();

  if (allowNull())
    counter++;

//  Load the combobox with the contents of the passed query, if any
  if (pQuery.first())
  {
    do
    {
      append(pQuery.value(0).toInt(), pQuery.value(1).toString());

      if (pQuery.value(0).toInt() == pSelected)
        selected = counter;

      counter++;
    }
    while(pQuery.next());
  }

  setCurrentItem(selected);
  if (_ids.count())
  {
    _lastId = _ids.at(selected);

    if (allowNull())
      emit notNull(TRUE);
  }
  else
  {
    _lastId = -1;

    if (allowNull())
      emit notNull(FALSE);
  }

  emit newID(_lastId);
  emit valid((_lastId != -1));
}

void XComboBox::populate(const char *pSql, int pSelected)
{
  qApp->setOverrideCursor(Qt::waitCursor);
  XSqlQuery query(pSql);
  populate(query, pSelected);
  qApp->restoreOverrideCursor();
}

void XComboBox::append(int pId, const QString &pText)
{
  _ids.append(pId);
  insertItem(pText);
}

int XComboBox::id(int pIndex) const
{
  if ((pIndex >= 0) && (pIndex < count()))
  {
    if ( (allowNull()) && (currentItem() <= 0) )
      return -1;
    else
      return _ids.at(pIndex);
  }
  else
    return -1;
}

int XComboBox::id() const
{
  if (_ids.count())
  {
    if ( (allowNull()) && (currentItem() <= 0) )
      return -1;
    else
      return _ids.at(currentItem());
  }
  else
    return -1;
}

bool XComboBox::isValid() const
{
  if ((allowNull()) && (id() == -1))
    return FALSE;
  else
    return TRUE;
}

void XComboBox::sHandleNewIndex(int pIndex)
{
  if ((pIndex >= 0) && (pIndex < _ids.count()) && (_ids.at(pIndex) != _lastId))
  {
    _lastId = _ids.at(pIndex);
    emit newID(_lastId);
    
    if (allowNull())
    {
      emit valid((pIndex != 0));
      emit notNull((pIndex != 0));
    }
  }
}

void XComboBox::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QComboBox::mousePressEvent(event);
}

QSize XComboBox::sizeHint() const
{
  QSize s = QComboBox::sizeHint();
#ifdef Q_WS_MAC
  s.setWidth(s.width() + 12);
#endif
  return s;
}

