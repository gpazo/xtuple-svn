/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPlannedOrdersByItem.h"

#include <QMenu>
#include <QSqlError>

#include <openreports.h>
#include <metasql.h>
#include <parameter.h>

#include "deletePlannedOrder.h"
#include "dspRunningAvailability.h"
#include "plannedOrder.h"
#include "firmPlannedOrder.h"
#include "mqlutil.h"
#include "purchaseRequest.h"
#include "transferOrder.h"
#include "workOrder.h"

dspPlannedOrdersByItem::dspPlannedOrdersByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_planord, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _planord->addColumn(tr("Order #"),    _orderColumn, Qt::AlignCenter,true, "ordernum");
  _planord->addColumn(tr("Type"),       _uomColumn,   Qt::AlignCenter,true, "ordtype");
  _planord->addColumn(tr("Site"),       _whsColumn,   Qt::AlignCenter,true, "warehous_code");
  _planord->addColumn(tr("From Site"),  _whsColumn,   Qt::AlignCenter,true, "supply_warehous_code");
  _planord->addColumn(tr("Start Date"), _dateColumn,  Qt::AlignCenter,true, "planord_startdate");
  _planord->addColumn(tr("Due Date"),   _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  _planord->addColumn(tr("Qty"),        _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  _planord->addColumn(tr("Firm"),       _uomColumn,   Qt::AlignCenter,true, "planord_firm");
  _planord->addColumn(tr("Comments"),   -1,           Qt::AlignLeft,  true, "comments");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

dspPlannedOrdersByItem::~dspPlannedOrdersByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPlannedOrdersByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPlannedOrdersByItem::setParams(ParameterList &pParams)
{
  pParams.append("item_id", _item->id());
  _warehouse->appendValue(pParams);

  return true;
}

void dspPlannedOrdersByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("PlannedOrdersByItem", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspPlannedOrdersByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);
  pMenu->insertSeparator();
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
  if (!_privileges->check("CreatePlannedOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (pSelected->text(7) == "No")
  {
    menuItem = pMenu->insertItem(tr("Firm Order..."), this, SLOT(sFirmOrder()), 0);
    if (!_privileges->check("FirmPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Soften Order..."), this, SLOT(sSoftenOrder()), 0);
    if (!_privileges->check("SoftenPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Release Order..."), this, SLOT(sReleaseOrder()), 0);
  if ( (!_privileges->check("ReleasePlannedOrders")) ||
       ((pSelected->text(1) == "T/O") && (!_privileges->check("MaintainTransferOrders")) ) ||
       ((pSelected->text(1) == "W/O") && (!_privileges->check("MaintainWorkOrders")) ) ||
       ((pSelected->text(1) == "P/O") && (!_privileges->check("MaintainPurchaseRequests")) ) )
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Order..."), this, SLOT(sDeleteOrder()), 0);
  if (!_privileges->check("DeletePlannedOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPlannedOrdersByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _planord->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPlannedOrdersByItem::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByItem::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _planord->id());
  q.exec();

  sFillList();
}

void dspPlannedOrdersByItem::sReleaseOrder()
{
  if (_planord->currentItem()->text(1) == "W/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (_planord->currentItem()->text(1) == "P/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (_planord->currentItem()->text(1) == "T/O")
  {
    ParameterList params;
    params.append("mode", "releaseTO");
    params.append("planord_id", _planord->id());

    transferOrder *newdlg = new transferOrder();
    if(newdlg->set(params) == NoError)
      omfgThis->handleNewWindow(newdlg);
    else
      delete newdlg;
  }
  sFillList();
}

void dspPlannedOrdersByItem::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("planord_id", _planord->id());

  plannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspPlannedOrdersByItem::sDeleteOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  deletePlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("schedule", "plannedorders");
  q = mql.toQuery(params);
  _planord->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
