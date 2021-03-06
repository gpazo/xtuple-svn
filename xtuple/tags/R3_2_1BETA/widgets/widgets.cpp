/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "widgets.h"

#include "plugins/addressclusterplugin.h"
#include "plugins/alarmsplugin.h"
#include "plugins/calendarcomboboxplugin.h"
#include "plugins/clineeditplugin.h"
#include "plugins/cmclusterplugin.h"
#include "plugins/commentsplugin.h"
#include "plugins/contactclusterplugin.h"
#include "plugins/crmacctclusterplugin.h"
#include "plugins/currclusterplugin.h"
#include "plugins/currdisplayplugin.h"
#include "plugins/custclusterplugin.h"
#include "plugins/custinfoplugin.h"
#include "plugins/dateclusterplugin.h"
#include "plugins/deptclusterplugin.h"
#include "plugins/dlineeditplugin.h"
#include "plugins/documentsplugin.h"
#include "plugins/empclusterplugin.h"
#include "plugins/expenseclusterplugin.h"
#include "plugins/expenselineeditplugin.h"
#include "plugins/fileclusterplugin.h"
#include "plugins/glclusterplugin.h"
#include "plugins/imageclusterplugin.h"
#include "plugins/invoiceclusterplugin.h"
#include "plugins/invoicelineeditplugin.h"
#include "plugins/incidentclusterplugin.h"
#include "plugins/itemclusterplugin.h"
#include "plugins/itemlineeditplugin.h"
#include "plugins/lotserialclusterplugin.h"
#include "plugins/orderclusterplugin.h"
#include "plugins/opportunityclusterplugin.h"
#include "plugins/parametergroupplugin.h"
#include "plugins/periodslistviewplugin.h"
#include "plugins/planordclusterplugin.h"
#include "plugins/planordlineeditplugin.h"
#include "plugins/poclusterplugin.h"
#include "plugins/polineeditplugin.h"
#include "plugins/projectclusterplugin.h"
#include "plugins/projectlineeditplugin.h"
#include "plugins/raclusterplugin.h"
#include "plugins/revisionclusterplugin.h"
#include "plugins/screenplugin.h"
#include "plugins/shiftclusterplugin.h"
#include "plugins/shipmentclusterplugin.h"
#include "plugins/shiptoclusterplugin.h"
#include "plugins/shiptoeditplugin.h"
#include "plugins/soclusterplugin.h"
#include "plugins/solineeditplugin.h"
#include "plugins/toclusterplugin.h"
#include "plugins/usernameclusterplugin.h"
#include "plugins/usernamelineeditplugin.h"
#include "plugins/vendorclusterplugin.h"
#include "plugins/vendorgroupplugin.h"
#include "plugins/vendorinfoplugin.h"
#include "plugins/vendorlineeditplugin.h"
#include "plugins/warehousegroupplugin.h"
#include "plugins/wcomboboxplugin.h"
#include "plugins/woclusterplugin.h"
#include "plugins/wolineeditplugin.h"
#include "plugins/womatlclusterplugin.h"
#include "plugins/workcenterclusterplugin.h"
#include "plugins/workcenterlineeditplugin.h"
#include "plugins/xcheckboxplugin.h"
#include "plugins/xcomboboxplugin.h"
#include "plugins/xlabelplugin.h"
#include "plugins/xlineeditplugin.h"
#include "plugins/xtreewidgetplugin.h"
#include "plugins/xtreeviewplugin.h"
#include "plugins/xspinboxplugin.h"
#include "plugins/xurllabelplugin.h"
#include "plugins/xtexteditplugin.h"

xTuplePlugin::xTuplePlugin(QObject * parent) : QObject(parent)
{
  m_plugins.append(new AddressClusterPlugin(this));
  m_plugins.append(new AlarmsPlugin(this));
  m_plugins.append(new CalendarComboBoxPlugin(this));
  m_plugins.append(new CLineEditPlugin(this));
  m_plugins.append(new CmClusterPlugin(this));
  m_plugins.append(new CommentsPlugin(this));
  m_plugins.append(new ContactClusterPlugin(this));
  m_plugins.append(new CRMAcctClusterPlugin(this));
  m_plugins.append(new CurrClusterPlugin(this));
  m_plugins.append(new CurrDisplayPlugin(this));
  m_plugins.append(new CustClusterPlugin(this));
  m_plugins.append(new CustInfoPlugin(this));
  m_plugins.append(new DateClusterPlugin(this));
  m_plugins.append(new DeptClusterPlugin(this));
  m_plugins.append(new DLineEditPlugin(this));
  m_plugins.append(new DocumentsPlugin(this));
  m_plugins.append(new EmpClusterPlugin(this));
  m_plugins.append(new ExpenseClusterPlugin(this));
  m_plugins.append(new ExpenseLineEditPlugin(this));
  m_plugins.append(new FileClusterPlugin(this));
  m_plugins.append(new GLClusterPlugin(this));
  m_plugins.append(new ImageClusterPlugin(this));
  m_plugins.append(new InvoiceClusterPlugin(this));
  m_plugins.append(new InvoiceLineEditPlugin(this));
  m_plugins.append(new IncidentClusterPlugin(this));
  m_plugins.append(new ItemClusterPlugin(this));
  m_plugins.append(new ItemLineEditPlugin(this));
  m_plugins.append(new LotserialClusterPlugin(this));
  m_plugins.append(new OrderClusterPlugin(this));
  m_plugins.append(new OpportunityClusterPlugin(this));
  m_plugins.append(new ParameterGroupPlugin(this));
  m_plugins.append(new PeriodsListViewPlugin(this));
  m_plugins.append(new PlanOrdClusterPlugin(this));
  m_plugins.append(new PlanOrdLineEditPlugin(this));
  m_plugins.append(new PoClusterPlugin(this));
  m_plugins.append(new PoLineEditPlugin(this));
  m_plugins.append(new ProjectClusterPlugin(this));
  m_plugins.append(new ProjectLineEditPlugin(this));
  m_plugins.append(new RaClusterPlugin(this));
  m_plugins.append(new RevisionClusterPlugin(this));
  m_plugins.append(new ScreenPlugin(this));
  m_plugins.append(new ShiftClusterPlugin(this));
  m_plugins.append(new ShipmentClusterPlugin(this));
  m_plugins.append(new ShiptoClusterPlugin(this));
  m_plugins.append(new ShiptoEditPlugin(this));
  m_plugins.append(new SoClusterPlugin(this));
  m_plugins.append(new SoLineEditPlugin(this));
  m_plugins.append(new ToClusterPlugin(this));
  m_plugins.append(new UsernameClusterPlugin(this));
  m_plugins.append(new UsernameLineEditPlugin(this));
  m_plugins.append(new VendorClusterPlugin(this));
  m_plugins.append(new VendorGroupPlugin(this));
  m_plugins.append(new VendorInfoPlugin(this));
  m_plugins.append(new VendorLineEditPlugin(this));
  m_plugins.append(new WarehouseGroupPlugin(this));
  m_plugins.append(new WComboBoxPlugin(this));
  m_plugins.append(new WoClusterPlugin(this));
  m_plugins.append(new WoLineEditPlugin(this));
  m_plugins.append(new WomatlClusterPlugin(this));
  m_plugins.append(new WorkCenterClusterPlugin(this));
  m_plugins.append(new WorkCenterLineEditPlugin(this));
  m_plugins.append(new XCheckBoxPlugin(this));
  m_plugins.append(new XComboBoxPlugin(this));
  m_plugins.append(new XLabelPlugin(this));
  m_plugins.append(new XLineEditPlugin(this));
  m_plugins.append(new XSpinBoxPlugin(this));
  m_plugins.append(new XTreeWidgetPlugin(this));
  m_plugins.append(new XTreeViewPlugin(this));
  m_plugins.append(new XURLLabelPlugin(this));
  m_plugins.append(new XTextEditPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> xTuplePlugin::customWidgets() const
{
  return m_plugins;
}

#ifndef QT_STATICPLUGIN
Q_EXPORT_PLUGIN(xTuplePlugin)
#else
Q_EXPORT_STATIC_PLUGIN(xTuplePlugin)
#endif

Preferences *_x_preferences = 0;
Metrics     *_x_metrics = 0;
QWorkspace  *_x_workspace = 0;
Privileges  *_x_privileges = 0;

void initializePlugin(Preferences *pPreferences, Metrics *pMetrics, Privileges *pPrivileges, QWorkspace *pWorkspace)
{
  _x_preferences = pPreferences;
  _x_metrics = pMetrics;
  _x_workspace = pWorkspace;
  _x_privileges = pPrivileges;
}
