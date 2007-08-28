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

#include "dspBacklogByItem.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <qworkspace.h>
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"
#include "rptBacklogByItem.h"

/*
 *  Constructs a dspBacklogByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBacklogByItem::dspBacklogByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_soitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemSites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBacklogByItem::~dspBacklogByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBacklogByItem::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspBacklogByItem::init()
{
  statusBar()->hide();

  _item->setType(ItemLineEdit::cSold);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->addColumn(tr("S/O #"),     _orderColumn, Qt::AlignRight  );
  _soitem->addColumn(tr("#"),         _seqColumn,   Qt::AlignCenter );
  _soitem->addColumn(tr("Customer"),  -1,           Qt::AlignLeft   );
  _soitem->addColumn(tr("Ordered"),   _dateColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Scheduled"), _dateColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("Ordered"),   _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Shipped"),   _qtyColumn,   Qt::AlignRight  );
  _soitem->addColumn(tr("Balance"),   _qtyColumn,   Qt::AlignRight  );

  if ( (!_privleges->check("ViewCustomerPrices")) && (!_privleges->check("MaintainCustomerPrices")) )
    _showPrices->setEnabled(FALSE);

  _item->setFocus();
}

void dspBacklogByItem::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->addColumn(tr("Amount $"), _moneyColumn, Qt::AlignRight);
  else
    _soitem->removeColumn(8);

  sFillList();
}

void dspBacklogByItem::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("print");

  _warehouse->appendValue(params);

  if (_showPrices->isChecked())
    params.append("showPrices");

  rptBacklogByItem newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspBacklogByItem::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->id(), false);
}

void dspBacklogByItem::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->id());
}

void dspBacklogByItem::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", _soitem->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPopulateMenu(Q3PopupMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privleges->check("MaintainSalesOrders")) && (!_privleges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);


  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privleges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
  if ((!_privleges->check("MaintainSalesOrders")) && (!_privleges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  if (!_privleges->check("PrintPackingLists"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspBacklogByItem::sFillList()
{
  if (_item->isValid())
  {
    QString sql( "SELECT cohead_id, coitem_id, cohead_number, coitem_linenumber, cust_name, "
                 "       formatDate(cohead_orderdate),"
                 "       formatDate(coitem_scheddate),"
                 "       formatQty(coitem_qtyord),"
                 "       formatQty(coitem_qtyshipped),"
                 "       formatQty(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)),"
                 "       formatMoney(round(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * (coitem_price / item_invpricerat),2)),"
                 "       round(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * (coitem_price / item_invpricerat),2) AS backlog "
                 "FROM cohead, coitem, cust, itemsite, item "
                 "WHERE ( (coitem_cohead_id=cohead_id)"
                 " AND (cohead_cust_id=cust_id)"
                 " AND (coitem_status NOT IN ('C','X'))"
                 " AND (coitem_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (itemsite_item_id=:item_id)"
                 " AND (coitem_scheddate BETWEEN :startDate AND :endDate)" );

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") "
           "ORDER BY coitem_scheddate";

    q.prepare(sql);
    _warehouse->bindValue(q);
    _dates->bindValue(q);
    q.bindValue(":item_id", _item->id());
    q.exec();
    _soitem->populate(q, TRUE);

    if (_showPrices->isChecked())
    {
      double totalBacklog = 0.0;

      q.first();
      do
        totalBacklog += q.value("backlog").toDouble();
      while (q.next());

      new XListViewItem( _soitem, _soitem->lastItem(), -1, -1,
                         "", "", tr("Total Backlog"), "", "", "", "", "",
                         formatMoney(totalBacklog) );
    }
  }
  else
    _soitem->clear();
}
