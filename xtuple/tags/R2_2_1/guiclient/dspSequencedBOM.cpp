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

#include "dspSequencedBOM.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include "rptSequencedBOM.h"

/*
 *  Constructs a dspSequencedBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSequencedBOM::dspSequencedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSequencedBOM::~dspSequencedBOM()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSequencedBOM::languageChange()
{
    retranslateUi(this);
}


void dspSequencedBOM::init()
{
  statusBar()->hide();

  _item->setType(ItemLineEdit::cGeneralManufactured  | ItemLineEdit::cGeneralPurchased);

  _bomitem->addColumn(tr("BOO Seq. #"),  _qtyColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("BOM Seq. #"),  _qtyColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter );

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

enum SetResponse dspSequencedBOM::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSequencedBOM::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  rptSequencedBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspSequencedBOM::sFillList()
{
  if (_item->isValid())
    sFillList(_item->id(), FALSE);
}

void dspSequencedBOM::sFillList(int pItemid, bool)
{
  if (pItemid == _item->id())
  {
    _bomitem->clear();

    QString sql( "SELECT bomitem_id, TEXT(booitem_seqnumber) AS seqnumber, bomitem_seqnumber, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS f_descrip, item_invuom,"
                 "       formatQtyper(bomitem_qtyper) AS f_qtyper,"
                 "       formatScrap(bomitem_scrap) AS f_scrap,"
                 "       formatDate(bomitem_effective, :always) AS f_effective,"
                 "       formatDate(bomitem_expires, :never) AS f_expires,"
                 "       bomitem_effective,"
                 "       CASE WHEN(bomitem_expires <= CURRENT_DATE) THEN TRUE"
                 "            ELSE FALSE"
                 "       END AS expired,"
                 "       CASE WHEN(bomitem_effective > CURRENT_DATE) THEN TRUE"
                 "            ELSE FALSE"
                 "       END AS future "
                 "FROM bomitem, booitem, item "
                 "WHERE ( (bomitem_item_id=item_id)"
                 " AND (bomitem_parent_item_id=:item_id)"
                 " AND (bomitem_booitem_id=booitem_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (bomitem_expires>CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bomitem_effective<=CURRENT_DATE)";

    sql += " ) "
           "UNION SELECT bomitem_id, '' AS seqnumber, bomitem_seqnumber, item_number,"
           "             (item_descrip1 || ' ' || item_descrip2), item_invuom,"
           "             formatQtyper(bomitem_qtyper),"
           "             formatScrap(bomitem_scrap),"
           "             formatDate(bomitem_effective, :always),"
           "             formatDate(bomitem_expires, :never),"
           "             bomitem_effective,"
           "             CASE WHEN(bomitem_expires <= CURRENT_DATE) THEN TRUE"
           "                  ELSE FALSE"
           "             END AS expired,"
           "             CASE WHEN(bomitem_effective > CURRENT_DATE) THEN TRUE"
           "                  ELSE FALSE"
           "             END AS future "
           "FROM bomitem, item "
           "WHERE ( (bomitem_item_id=item_id)"
           " AND (bomitem_parent_item_id=:item_id)"
           " AND (bomitem_booitem_id=-1)";

    if (!_showExpired->isChecked())
      sql += " AND (bomitem_expires>CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bomitem_effective<=CURRENT_DATE)";

    sql += " ) "
           "ORDER BY seqnumber, bomitem_seqnumber, bomitem_effective;";

    q.prepare(sql);
    q.bindValue(":always", tr("Always"));
    q.bindValue(":never", tr("Never"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    XListViewItem *last = 0;
    while (q.next())
    {
      last = new XListViewItem( _bomitem, last, q.value("bomitem_id").toInt(), q.value("seqnumber"),
                                q.value("bomitem_seqnumber"), q.value("item_number"),
                                q.value("f_descrip"), q.value("item_invuom"),
                                q.value("f_qtyper"), q.value("f_scrap"),
                                q.value("f_effective"), q.value("f_expires") );

      if (q.value("expired").toBool())
        last->setColor("red");
      else if (q.value("future").toBool())
        last->setColor("blue");
    }
  }
}
