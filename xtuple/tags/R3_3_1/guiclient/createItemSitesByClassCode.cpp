/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createItemSitesByClassCode.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

createItemSitesByClassCode::createItemSitesByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _locationGroupInt = new QButtonGroup(this);
  _locationGroupInt->addButton(_location);
  _locationGroupInt->addButton(_miscLocation);

  connect(_create, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_locationControl, SIGNAL(toggled(bool)), this, SLOT(sHandleMLC(bool)));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(populateLocations()));
  connect(_controlMethod, SIGNAL(activated(int)), this, SLOT(sHandleControlMethod()));
  connect(_poSupply, SIGNAL(toggled(bool)), this, SLOT(sHandlePOSupply(bool)));
  connect(_woSupply, SIGNAL(toggled(bool)), this, SLOT(sHandleWOSupply(bool)));

  _reorderLevel->setValidator(omfgThis->qtyVal());
  _orderUpToQty->setValidator(omfgThis->qtyVal());
  _minimumOrder->setValidator(omfgThis->qtyVal());
  _maximumOrder->setValidator(omfgThis->qtyVal());
  _orderMultiple->setValidator(omfgThis->qtyVal());
  _safetyStock->setValidator(omfgThis->qtyVal());

  _classCode->setType(ParameterGroup::ClassCode);

  _plannerCode->setAllowNull(TRUE);
  _plannerCode->setType(XComboBox::PlannerCodes);

  _costcat->setAllowNull(TRUE);
  _costcat->setType(XComboBox::CostCategories);

  _controlMethod->insertItem("None");
  _controlMethod->insertItem("Regular");
  _controlMethod->insertItem("Lot #");
  _controlMethod->insertItem("Serial #");
  _controlMethod->setCurrentIndex(-1);

  _reorderLevel->setDouble(0.0);
  _orderUpToQty->setDouble(0.0);
  _minimumOrder->setDouble(0.0);
  _maximumOrder->setDouble(0.0);
  _orderMultiple->setDouble(0.0);
  _safetyStock->setDouble(0.0);

  _cycleCountFreq->setValue(0);
  _leadTime->setValue(0);

  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());
  _costcat->setEnabled(_metrics->boolean("InterfaceToGL"));
  
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  else
  {
    _warehouse->setAllowNull(TRUE);
    _warehouse->setNull();
  }

  _costAvg->setVisible(_metrics->boolean("AllowAvgCostMethod"));
  _costStd->setVisible(_metrics->boolean("AllowStdCostMethod"));
}

createItemSitesByClassCode::~createItemSitesByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void createItemSitesByClassCode::languageChange()
{
  retranslateUi(this);
}

void createItemSitesByClassCode::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Select a Site"),
                           tr("<p>You must select a Site for this Item Site "
                              "before creating it." ) );
    _warehouse->setFocus();
    return;
  }

  if ( (_metrics->boolean("InterfaceToGL")) && (_costcat->id() == -1) )
  {
    QMessageBox::critical( this, tr("Cannot Create Item Sites"),
                           tr("<p>You must select a Cost Category for these "
                              "Item Sites before you may create them.") );
    _costcat->setFocus();
    return;
  } 

  if (_plannerCode->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Create Item Sites"),
                           tr("<p>You must select a Planner Code for these "
                              "Item Sites before you may create them.") );
    _plannerCode->setFocus();
    return;
  } 

  if (_controlMethod->currentIndex() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Create Item Sites"),
                           tr("<p>You must select a Control Method for these "
                              "Item Sites before you may create them.") );
    _controlMethod->setFocus();
    return;
  }

  if(!_costNone->isChecked() && !_costAvg->isChecked()
   && !_costStd->isChecked())
  {
    QMessageBox::critical(this, tr("Cannot Save Item Site"),
                          tr("<p>You must select a Cost Method for this "
                             "Item Site before you may save it.") );
    return;
  }

  if (_stocked->isChecked() && _reorderLevel->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Site"),
                           tr("<p>You must set a reorder level "
			      "for a stocked item before you may save it.") );
    _reorderLevel->setFocus();
    return;
  }
  
  if ( _locationControl->isChecked() )
  {
    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location "
                        "WHERE (location_warehous_id=:warehous_id)"
                        "LIMIT 1;" );
    locationid.bindValue(":warehous_id", _warehouse->id());
    locationid.exec();
    if (!locationid.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Item Site"),
                             tr( "You must first create at least one valid "
				 "Location for this Site before items may be "
				 "multiply located." ) );
      return;
    }
  }

  QString sql( "INSERT INTO itemsite "
               "( itemsite_item_id,"
               "  itemsite_warehous_id, itemsite_qtyonhand, itemsite_value,"
               "  itemsite_useparams, itemsite_useparamsmanual,"
               "  itemsite_reorderlevel, itemsite_ordertoqty,"
               "  itemsite_minordqty, itemsite_maxordqty, itemsite_multordqty,"
               "  itemsite_safetystock, itemsite_cyclecountfreq,"
               "  itemsite_leadtime, itemsite_eventfence, itemsite_plancode_id, itemsite_costcat_id,"
               "  itemsite_posupply, itemsite_wosupply,"
               "  itemsite_createpr, itemsite_createwo,"
               "  itemsite_sold, itemsite_soldranking,"
               "  itemsite_stocked,"
               "  itemsite_controlmethod, itemsite_perishable, itemsite_active,"
               "  itemsite_loccntrl, itemsite_location_id, itemsite_location,"
               "  itemsite_location_comments, itemsite_notes,"
               "  itemsite_abcclass, itemsite_freeze, itemsite_datelastused,"
               "  itemsite_ordergroup, itemsite_ordergroup_first, itemsite_mps_timefence, "
               "  itemsite_autoabcclass, itemsite_costmethod ) "
               "SELECT item_id,"
               "       :warehous_id, 0.0, 0.0,"
               "       :itemsite_useparams, :itemsite_useparamsmanual,"
               "       :itemsite_reorderlevel, :itemsite_ordertoqty,"
               "       :itemsite_minordqty, :itemsite_maxordqty, :itemsite_multordqty,"
               "       :itemsite_safetystock, :itemsite_cyclecountfreq,"
               "       :itemsite_leadtime, :itemsite_eventfence, :itemsite_plancode_id, :itemsite_costcat_id,"
               "       :itemsite_posupply, :itemsite_wosupply,"
               "       :itemsite_createpr, :itemsite_createwo,"
               "       :itemsite_sold, :itemsite_soldranking,"
               "       :itemsite_stocked,"
               "       :itemsite_controlmethod, :itemsite_perishable, TRUE,"
               "       :itemsite_loccntrl, :itemsite_location_id, :itemsite_location,"
               "       :itemsite_location_comments, '',"
               "       :itemsite_abcclass, FALSE, startOfTime(),"
               "       :itemsite_ordergroup, :itemsite_ordergroup_first, :itemsite_mps_timefence, "
               "       FALSE, CASE WHEN(item_type='R') THEN 'N' WHEN(item_type='J') THEN 'J' ELSE :itemsite_costmethod END "
               "FROM item "
               "WHERE ( (item_id NOT IN ( SELECT itemsite_item_id"
               "                          FROM itemsite"
               "                          WHERE (itemsite_warehous_id=:warehous_id) ) )" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += ");";

  q.prepare(sql);
  q.bindValue(":itemsite_reorderlevel", _reorderLevel->toDouble());
  q.bindValue(":itemsite_ordertoqty", _orderUpToQty->toDouble());
  q.bindValue(":itemsite_minordqty", _minimumOrder->toDouble());
  q.bindValue(":itemsite_maxordqty", _maximumOrder->toDouble());
  q.bindValue(":itemsite_multordqty", _orderMultiple->toDouble());
  q.bindValue(":itemsite_safetystock", _safetyStock->toDouble());
  q.bindValue(":itemsite_cyclecountfreq", _cycleCountFreq->value());
  q.bindValue(":itemsite_leadtime", _leadTime->value());
  q.bindValue(":itemsite_eventfence", _eventFence->value());
  q.bindValue(":itemsite_plancode_id", _plannerCode->id());
  q.bindValue(":itemsite_costcat_id", _costcat->id());
  q.bindValue(":itemsite_useparams",     QVariant(_useParameters->isChecked()));
  q.bindValue(":itemsite_useparamsmanual", QVariant(_useParametersOnManual->isChecked()));
  q.bindValue(":itemsite_posupply",        QVariant(_poSupply->isChecked()));
  q.bindValue(":itemsite_wosupply",        QVariant(_woSupply->isChecked()));
  q.bindValue(":itemsite_createpr",      QVariant(_createPr->isChecked()));
  q.bindValue(":itemsite_createwo",      QVariant(_createWo->isChecked()));
  q.bindValue(":itemsite_sold",          QVariant(_sold->isChecked()));
  q.bindValue(":itemsite_stocked",       QVariant(_stocked->isChecked()));
  q.bindValue(":itemsite_loccntrl",      QVariant(_locationControl->isChecked()));
  q.bindValue(":itemsite_perishable",    QVariant(_perishable->isChecked()));
  q.bindValue(":itemsite_soldranking", _soldRanking->value());
  q.bindValue(":itemsite_location_comments", _locationComments->text().trimmed());
  q.bindValue(":itemsite_abcclass", _abcClass->currentText());

  q.bindValue(":itemsite_ordergroup", _orderGroup->value());
  q.bindValue(":itemsite_ordergroup_first", QVariant(_orderGroupFirst->isChecked()));
  q.bindValue(":itemsite_mps_timefence", _mpsTimeFence->value());

  if (_useDefaultLocation->isChecked())
  {
    if (_location->isChecked())
    {
      q.bindValue(":itemsite_location", "");
      q.bindValue(":itemsite_location_id", _locations->id());
    }
    else if (_miscLocation->isChecked())
    {
      q.bindValue(":itemsite_location", _miscLocationName->text().trimmed());
      q.bindValue(":itemsite_location_id", -1);
    }
  }
  else
  {
    q.bindValue(":itemsite_location", "");
    q.bindValue(":itemsite_location_id", -1);
  }

  if (_controlMethod->currentIndex() == 0)
    q.bindValue(":itemsite_controlmethod", "N");
  else if (_controlMethod->currentIndex() == 1)
    q.bindValue(":itemsite_controlmethod", "R");
  else if (_controlMethod->currentIndex() == 2)
    q.bindValue(":itemsite_controlmethod", "L");
  else if (_controlMethod->currentIndex() == 3)
    q.bindValue(":itemsite_controlmethod", "S");

  if(_costNone->isChecked())
    q.bindValue(":itemsite_costmethod", "N");
  else if(_costAvg->isChecked())
    q.bindValue(":itemsite_costmethod", "A");
  else if(_costStd->isChecked())
    q.bindValue(":itemsite_costmethod", "S");

  q.bindValue(":warehous_id", _warehouse->id());
  _classCode->bindValue(q);
  q.exec();

  omfgThis->sItemsitesUpdated();

  accept();
}

void createItemSitesByClassCode::sHandlePOSupply(bool pSupplied)
{
  if (pSupplied)
    _createPr->setEnabled(TRUE);
  else
  {
    _createPr->setEnabled(FALSE);
    _createPr->setChecked(FALSE);
  }
} 

void createItemSitesByClassCode::sHandleWOSupply(bool pSupplied)
{
  if (pSupplied)
    _createWo->setEnabled(TRUE);
  else
  {
    _createWo->setEnabled(FALSE);
    _createWo->setChecked(FALSE);
  }
} 

void createItemSitesByClassCode::sHandleMLC(bool pMLC)
{
  if (pMLC)
  {
    _location->setChecked(TRUE);
    _miscLocation->setEnabled(FALSE);
    _miscLocationName->setEnabled(FALSE);
  }
  else
    _miscLocation->setEnabled(TRUE);
}

void createItemSitesByClassCode::sHandleControlMethod()
{
  if (_controlMethod->currentIndex() == 0)
  {
    _costNone->setChecked(true);
    _costNone->setEnabled(true);
    _costAvg->setEnabled(false);
    _costStd->setEnabled(false);
  }
  else
  {
    if(_costStd->isVisibleTo(this) && !_costAvg->isChecked())
      _costStd->setChecked(true);
    else
      _costAvg->setChecked(true);
    _costNone->setEnabled(false);
    _costAvg->setEnabled(true);
    _costStd->setEnabled(true);
  }

  if ( (_controlMethod->currentIndex() == 2) ||
       (_controlMethod->currentIndex() == 3) )
    _perishable->setEnabled(TRUE);
  else
  {
    _perishable->setChecked(FALSE);
    _perishable->setEnabled(FALSE);
  }
}

void createItemSitesByClassCode::populateLocations()
{
  q.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname "
             "FROM location "
             "WHERE ( (location_warehous_id=:warehous_id)"
             " AND (NOT location_restrict) ) "
             "ORDER BY locationname;" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _locations->populate(q);

  if (_locations->count())
  {
    _location->setEnabled(TRUE);
    _locations->setEnabled(TRUE);
  }
  else
  {
    _location->setEnabled(FALSE);
    _locations->setEnabled(FALSE);
  }
}

void createItemSitesByClassCode::clear()
{
  _useParameters->setChecked(FALSE);
  _useParametersOnManual->setChecked(FALSE);
  _reorderLevel->setDouble(0.0);
  _orderUpToQty->setDouble(0.0);
  _minimumOrder->setDouble(0.0);
  _maximumOrder->setDouble(0.0);
  _orderMultiple->setDouble(0.0);
  _safetyStock->setDouble(0.0);

  _orderGroup->setValue(1);
  _orderGroupFirst->setChecked(FALSE);

  _cycleCountFreq->setValue(0);
  _leadTime->setValue(0);
  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());

  _controlMethod->setCurrentIndex(1);
  sHandleControlMethod();
  _poSupply->setChecked(TRUE);
  _woSupply->setChecked(TRUE);
  _sold->setChecked(TRUE);
  _stocked->setChecked(FALSE);

  _locationControl->setChecked(FALSE);
  _useDefaultLocation->setChecked(FALSE);
  _miscLocationName->clear();
  _locationComments->clear();

  _costcat->setId(-1);

  populateLocations();
}
