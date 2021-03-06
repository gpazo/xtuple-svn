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

#include "transferOrderItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "itemCharacteristicDelegate.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"
#include "taxDetail.h"

transferOrderItem::transferOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_cancel,	SIGNAL(clicked()),      this, SLOT(sCancel()));
  connect(_freight,     SIGNAL(valueChanged()), this, SLOT(sLookupTax()));
  connect(_freight,	SIGNAL(valueChanged()), this, SLOT(sChanged()));
  connect(_item,	SIGNAL(newId(int)),     this, SLOT(sChanged()));
  connect(_item,	SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo(int)));
  connect(_next,	SIGNAL(clicked()),      this, SLOT(sNext()));
  connect(_notes,	SIGNAL(textChanged()),  this, SLOT(sChanged()));
  connect(_prev,	SIGNAL(clicked()), this, SLOT(sPrev()));
  connect(_promisedDate,SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_qtyOrdered,  SIGNAL(lostFocus()), this, SLOT(sDetermineAvailability()));
  connect(_qtyOrdered,  SIGNAL(textChanged(const QString&)), this, SLOT(sChanged()));
  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sChanged()));
  connect(_scheduledDate, SIGNAL(newDate(const QDate&)), this, SLOT(sDetermineAvailability()));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_showAvailability, SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_showIndented,SIGNAL(toggled(bool)), this, SLOT(sDetermineAvailability()));
  connect(_taxLit, SIGNAL(leftClickedURL(QString)), this, SLOT(sTaxDetail()));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sChanged()));
  connect(_warehouse,	SIGNAL(newID(int)), this, SLOT(sDetermineAvailability()));

  _modified	= false;
  _canceling	= false;
  _error	= false;
  _originalQtyOrd	= 0.0;

  _availabilityLastItemid	= -1;
  _availabilityLastWarehousid	= -1;
  _availabilityLastSchedDate	= QDate();
  _availabilityLastShow		= false;
  _availabilityLastShowIndent	= false;
  _availabilityQtyOrdered	= 0.0;

  _item->setType(ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") ); // ItemLineEdit::cActive doesn't compare against the itemsite record

  _availability->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _availability->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter );
  _availability->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("On Order"),      _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight  );
  _availability->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight  );
  
  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  if (!_metrics->boolean("UsePromiseDate"))
  {
    _promisedDateLit->hide();
    _promisedDate->hide();
  }

  _showAvailability->setChecked(_preferences->boolean("ShowSOItemAvailability"));

  _qtyOrdered->setValidator(omfgThis->qtyVal());
  _comments->setType(Comments::TransferOrderItem);

  _dstwhsid	= -1;
  _itemsiteid	= -1;
  _transwhsid	= -1;
  _toheadid	= -1;
  _taxauthid	= -1;
  _taxCache.clear();
}

transferOrderItem::~transferOrderItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void transferOrderItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse transferOrderItem::set(const ParameterList &pParams)
{
  QVariant  param;
  bool      valid;

  _prev->setEnabled(true);
  _next->setEnabled(true);
  _next->setText(tr("Next"));

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _toheadid = param.toInt();

  param = pParams.value("srcwarehouse_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("taxauth_id", &valid);
  if (valid)
    _taxauthid = param.toInt();

  param = pParams.value("orderNumber", &valid);
  if (valid)
    _orderNumber->setText(param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
    _freight->setId(param.toInt());

  param = pParams.value("orderDate", &valid);
  if (valid)
    _freight->setEffective(param.toDate());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _save->setEnabled(FALSE);
      _next->setText(tr("New"));
      _comments->setEnabled(FALSE);
      _item->setReadOnly(false);
      _item->setFocus();

      _item->addExtraClause(QString("(item_id IN ("
				    "  SELECT itemsite_item_id"
				    "  FROM itemsite"
				    "  WHERE itemsite_warehous_id=%1))")
				    .arg(_warehouse->id()) );

      prepare();

      q.prepare("SELECT count(*) AS cnt"
                "  FROM toitem"
                " WHERE (toitem_tohead_id=:tohead_id);");
      q.bindValue(":tohead_id", _toheadid);
      q.exec();
      if(!q.first() || q.value("cnt").toInt() == 0)
        _prev->setEnabled(false);
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(true);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
    }
  }

  if(cView == _mode)
  {
    _item->setReadOnly(true);
    _qtyOrdered->setEnabled(false);
    _freight->setEnabled(false);
    _scheduledDate->setEnabled(false);
    _notes->setEnabled(false);
    _comments->setReadOnly(true);
    _itemcharView->setEnabled(false);
    _promisedDate->setEnabled(false);

    _save->hide();

    _close->setFocus();
  }

  param = pParams.value("toitem_id", &valid);
  if (valid)
  {
    _toitemid = param.toInt();

    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber "
	      " LIMIT 1;");
    q.bindValue(":id", _toitemid);
    q.exec();
    if(!q.first() || q.value("id").toInt() == _toitemid)
      _prev->setEnabled(false);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber DESC"
	      " LIMIT 1;");
    q.bindValue(":id", _toitemid);
    q.exec();
    if(q.first() && q.value("id").toInt() == _toitemid)
    {
      if(cView == _mode)
        _next->setEnabled(false);
      else
        _next->setText(tr("New"));
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  populate();	// TODO: should this go BEFORE pParams.value("item_id")?

  _modified = false;

  return NoError;
}

void transferOrderItem::prepare()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT (COALESCE(MAX(toitem_linenumber), 0) + 1) AS _linenumber "
               "FROM toitem "
               "WHERE (toitem_tohead_id=:toitem_id)" );
    q.bindValue(":toitem_id", _toheadid);
    q.exec();
    if (q.first())
      _lineNumber->setText(q.value("_linenumber").toString());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MIN(toitem_schedshipdate) AS scheddate "
               "FROM toitem "
               "WHERE (toitem_tohead_id=:tohead_id);" );
    q.bindValue(":tohead_id", _toheadid);
    q.exec();
    if (q.first())
      _scheduledDate->setDate(q.value("scheddate").toDate());
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  _modified = false;
}

void transferOrderItem::clear()
{
  _item->setId(-1);
  _qtyOrdered->clear();
  _freight->reset();
  _scheduledDate->clear();
  _promisedDate->clear();
  _stdcost->clear();
  _shippedToDate->clear();
  _onHand->clear();
  _allocated->clear();
  _unallocated->clear();
  _onOrder->clear();
  _available->clear();
  _itemchar->removeRows(0, _itemchar->rowCount());
  _notes->clear();
  _comments->setId(-1);
  _warehouse->setId(_preferences->value("PreferredWarehouse").toInt());
  _originalQtyOrd = 0.0;
  _modified = false;
}

void transferOrderItem::sSave()
{
  _save->setFocus();

  _error = true;
  if (!(_qtyOrdered->toDouble() > 0))
  {
    QMessageBox::warning( this, tr("Cannot Save Transfer Order Item"),
                          tr("<p>You must enter a valid Quantity Ordered "
			     "before saving this Transfer Order Item.")  );
    _qtyOrdered->setFocus();
    return;
  }

  if (!(_scheduledDate->isValid()))
  {
    QMessageBox::warning( this, tr("Cannot Save Transfer Order Item"),
                          tr("<p>You must enter a valid Schedule Date before "
			     "saving this Transfer Order Item.") );
    _scheduledDate->setFocus();
    return;
  }

  if (_qtyOrdered->toDouble() < _shippedToDate->text().toDouble())
  {
    QMessageBox::warning(this, tr("Cannot Save Transfer Order Item"),
			 tr("<p>You cannot set the quantity of the order to "
			    "a value less than the quantity that has already "
			    "been shipped."));
    _qtyOrdered->setFocus();
    return;
  }

  _error = false;

  QDate promiseDate;

  if (_metrics->boolean("UsePromiseDate"))
  {
    if (_promisedDate->isValid())
    {
      if (_promisedDate->isNull())
        promiseDate = omfgThis->endOfTime();
      else
        promiseDate = _promisedDate->date();
    }
  }
  else
    promiseDate = omfgThis->endOfTime();

  if (_mode == cNew || _mode == cEdit)
  {
    if (itemSite::createItemSite(this, _itemsiteid, _transwhsid, false) < 0 ||
	itemSite::createItemSite(this, _itemsiteid, _dstwhsid, true) < 0)
      return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('toitem_toitem_id_seq') AS toitem_id");
    if (q.first())
      _toitemid = q.value("toitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      reject();
      return;
    }

    q.prepare( "INSERT INTO toitem "
               "( toitem_id, toitem_tohead_id, toitem_linenumber,"
               "  toitem_item_id, toitem_status, toitem_duedate,"
	       "  toitem_schedshipdate, toitem_schedrecvdate,"
               "  toitem_qty_ordered, toitem_notes,"
               "  toitem_uom, toitem_stdcost, toitem_prj_id,"
               "  toitem_freight, toitem_freight_curr_id,"
	       "  toitem_freighttax_id, toitem_freighttax_pcta,"
	       "  toitem_freighttax_pctb, toitem_freighttax_pctc,"
	       "  toitem_freighttax_ratea, toitem_freighttax_rateb,"
	       "  toitem_freighttax_ratec ) "
               "VALUES "
	       "( :toitem_id, :toitem_tohead_id, :toitem_linenumber,"
               "  :toitem_item_id, 'O', :toitem_duedate,"
	       "  :toitem_schedshipdate, :toitem_schedrecvdate,"
               "  :toitem_qty_ordered, :toitem_notes,"
               "  :toitem_uom, stdCost(:toitem_item_id), :toitem_prj_id,"
               "  :toitem_freight, :toitem_freight_curr_id,"
	       "  :toitem_freighttax_id, :toitem_freighttax_pcta,"
	       "  :toitem_freighttax_pctb, :toitem_freighttax_pctc,"
	       "  :toitem_freighttax_ratea, :toitem_freighttax_rateb,"
	       "  :toitem_freighttax_ratec );" );

  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE toitem SET"
               "  toitem_duedate=:toitem_duedate,"
	       "  toitem_schedshipdate=:toitem_schedshipdate,"
	       "  toitem_schedrecvdate=:toitem_schedrecvdate,"
               "  toitem_qty_ordered=:toitem_qty_ordered,"
	       "  toitem_notes=:toitem_notes,"
	       "  toitem_prj_id=:toitem_prj_id,"
               "  toitem_freight=:toitem_freight,"
	       "  toitem_freight_curr_id=:toitem_freight_curr_id,"
	       "  toitem_freighttax_id=:toitem_freighttax_id,"
	       "  toitem_freighttax_pcta=:toitem_freighttax_pcta,"
	       "  toitem_freighttax_pctb=:toitem_freighttax_pctb,"
	       "  toitem_freighttax_pctc=:toitem_freighttax_pctc,"
	       "  toitem_freighttax_ratea=:toitem_freighttax_ratea,"
	       "  toitem_freighttax_rateb=:toitem_freighttax_rateb,"
	       "  toitem_freighttax_ratec=:toitem_freighttax_ratec,"
	       "  toitem_lastupdated=CURRENT_TIMESTAMP "
               "WHERE (toitem_id=:toitem_id);" );
  }

  q.bindValue(":toitem_id",		  _toitemid);
  q.bindValue(":toitem_tohead_id",	  _toheadid);
  q.bindValue(":toitem_linenumber",	  _lineNumber->text().toInt());
  q.bindValue(":toitem_item_id",	  _item->id());
  q.bindValue(":toitem_duedate",	  _scheduledDate->date());
  q.bindValue(":toitem_schedshipdate",    _scheduledDate->date()); // ??
  q.bindValue(":toitem_schedrecvdate",    promiseDate);
  q.bindValue(":toitem_qty_ordered",	  _qtyOrdered->toDouble());
  q.bindValue(":toitem_notes",		  _notes->text());
  q.bindValue(":toitem_uom",		  _item->uom());
  // TODO: q.bindValue(":toitem_prj_id",		);
  q.bindValue(":toitem_freight",	  _freight->localValue());
  q.bindValue(":toitem_freight_curr_id",  _freight->id());
  if (_taxCache.freightId() > 0)
    q.bindValue(":toitem_freighttax_id",  _taxCache.freightId());
  q.bindValue(":toitem_freighttax_pcta",  _taxCache.freightPct(0));
  q.bindValue(":toitem_freighttax_pctb",  _taxCache.freightPct(1));
  q.bindValue(":toitem_freighttax_pctc",  _taxCache.freightPct(2));
  q.bindValue(":toitem_freighttax_ratea", _taxCache.freight(0));
  q.bindValue(":toitem_freighttax_rateb", _taxCache.freight(1));
  q.bindValue(":toitem_freighttax_ratec", _taxCache.freight(2));

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode != cView)
  {
    QString type = "TI";

    q.prepare("SELECT updateCharAssignment(:target_type, :target_id, :char_id, :char_value) AS result;");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_type", type);
      q.bindValue(":target_id", _toitemid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("updateCharAssignment", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

  omfgThis->sTransferOrdersUpdated(_toheadid);

  if ((!_canceling) && (cNew == _mode))
  {
    clear();
    prepare();
    _prev->setEnabled(true);
    _item->setFocus();
  }

  _modified = false;
}

void transferOrderItem::sPopulateItemInfo(int pItemid)
{
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (pItemid != -1)
  {
    q.prepare("SELECT stdcost(:item_id) AS stdcost, itemsite_id "
	      "FROM itemsite "
	      "WHERE ((itemsite_item_id=:item_id)"
	      "  AND  (itemsite_warehous_id=:whsid));");
    q.bindValue(":item_id", pItemid);
    q.bindValue(":whsid", _warehouse->id());
    q.exec();
    if (q.first())
    {
      _stdcost->setBaseValue(q.value("stdcost").toDouble());
      _itemsiteid = q.value("itemsite_id").toInt();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value,"
	       "               (SELECT c.charass_value"
	       "                FROM charass c"
	       "                WHERE ((c.charass_target_type='I')"
	       "                  AND  (c.charass_target_id=:item_id)"
	       "                  AND  (c.charass_default)"
	       "                  AND  (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type=:totype"
               "      AND b.charass_target_id=:toitem_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":totype", "TI");
    q.bindValue(":toitem_id", _toitemid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrderItem::sDetermineAvailability()
{
  if(  (_item->id()==_availabilityLastItemid)
    && (_warehouse->id()==_availabilityLastWarehousid)
    && (_scheduledDate->date()==_availabilityLastSchedDate)
    && (_showAvailability->isChecked()==_availabilityLastShow)
    && (_showIndented->isChecked()==_availabilityLastShowIndent)
    && (_qtyOrdered->toDouble()==_availabilityQtyOrdered) )
    return;

  _availabilityLastItemid	= _item->id();
  _availabilityLastWarehousid	= _warehouse->id();
  _availabilityLastSchedDate	= _scheduledDate->date();
  _availabilityLastShow		= _showAvailability->isChecked();
  _availabilityLastShowIndent	= _showIndented->isChecked();
  _availabilityQtyOrdered	= _qtyOrdered->toDouble();

  _availability->clear();

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       formatQty(qoh) AS f_qoh,"
                          "       formatQty(allocated) AS f_allocated,"
                          "       formatQty(noNeg(qoh - allocated)) AS f_unallocated,"
                          "       formatQty(ordered) AS f_ordered,"
                          "       (qoh - allocated + ordered) AS available,"
                          "       formatQty(qoh - allocated + ordered) AS f_available "
                          "FROM ( SELECT itemsite_id, itemsite_qtyonhand AS qoh,"
                          "              qtyAllocated(itemsite_id, DATE(:date)) AS allocated,"
                          "              qtyOrdered(itemsite_id, DATE(:date)) AS ordered "
                          "       FROM itemsite, item "
                          "       WHERE ((itemsite_item_id=item_id)"
                          "        AND (item_id=:item_id)"
                          "        AND (itemsite_warehous_id=:warehous_id)) ) AS data;" );
    availability.bindValue(":date", _scheduledDate->date());
    availability.bindValue(":item_id", _item->id());
    availability.bindValue(":warehous_id", _warehouse->id());
    availability.exec();
    if (availability.first())
    {
      _onHand->setText(availability.value("f_qoh").toString());
      _allocated->setText(availability.value("f_allocated").toString());
      _unallocated->setText(availability.value("f_unallocated").toString());
      _onOrder->setText(availability.value("f_ordered").toString());
      _available->setText(availability.value("f_available").toString());

      if (availability.value("available").toDouble() < _qtyOrdered->toDouble())
        _available->setPaletteForegroundColor(QColor("red"));
      else
        _available->setPaletteForegroundColor(QColor("black"));

      if ( (_item->itemType() == "M") )
      {
        if(_showIndented->isChecked())
        {
          availability.prepare("SELECT indentedBOM(:item_id) AS bomwork_set_id;");
          availability.bindValue(":item_id", _item->id());
          availability.exec();
          if(availability.first())
          {
            int _worksetid = availability.value("bomwork_set_id").toInt();
	    if (_worksetid < 0)
	    {
	      systemError(this, storedProcErrorLookup("indentedBOM", _worksetid),
			  __FILE__, __LINE__);
	      return;
	    }
            availability.prepare("SELECT itemsiteid, reorderlevel,"
                                 "       bomwork_level, bomwork_id, bomwork_parent_id,"
                                 "       bomwork_seqnumber AS bomitem_seqnumber,"
                                 "       item_number, item_descrip, uom_name,"
                                 "       pendalloc, formatQty(pendalloc) AS f_pendalloc,"
                                 "       ordered, formatQty(ordered) AS f_ordered,"
                                 "       qoh, formatQty(qoh) AS f_qoh,"
                                 "       formatQty(totalalloc + pendalloc) AS f_totalalloc,"
                                 "       (qoh + ordered - (totalalloc + pendalloc)) as totalavail,"
                                 "       formatQty(qoh + ordered - (totalalloc + pendalloc)) as f_totalavail"
                                 "  FROM ( SELECT itemsite_id AS itemsiteid,"
                                 "                CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                 "                bomwork_id, bomwork_parent_id,"
                                 "                bomwork_level, bomwork_seqnumber,"
                                 "                item_number, uom_name,"
                                 "                (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
                                 "                ((bomwork_qtyper * (1 + bomwork_scrap)) * :qty) AS pendalloc,"
                                 "                (qtyAllocated(itemsite_id, DATE(:schedDate)) - ((bomwork_qtyper * (1 + bomwork_scrap)) * :origQtyOrd)) AS totalalloc,"
                                 "                noNeg(itemsite_qtyonhand) AS qoh,"
                                 "                qtyOrdered(itemsite_id, DATE(:schedDate)) AS ordered"
                                 "           FROM bomwork, item, itemsite, uom"
                                 "          WHERE ( (itemsite_item_id=item_id)"
                                 "            AND   (itemsite_warehous_id=:warehous_id)"
                                 "            AND   (bomwork_item_id=item_id)"
                                 "            AND   (item_inv_uom_id=uom_id)"
                                 "            AND   (bomwork_set_id=:bomwork_set_id)"
                                 "                )"
                                 "       ) AS data "
                                 "ORDER BY bomwork_level, bomwork_seqnumber DESC;");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.bindValue(":warehous_id", _warehouse->id());
            availability.bindValue(":qty", _qtyOrdered->toDouble());
            availability.bindValue(":schedDate", _scheduledDate->date());
            availability.bindValue(":origQtyOrd", _originalQtyOrd);
            availability.exec();
	    XTreeWidgetItem *last = 0;
            while(availability.next())
            {

              //  If the current bomwork is top level, make it a child of the XListView
              if (availability.value("bomwork_parent_id").toInt() == -1)
                last = new XTreeWidgetItem( _availability, availability.value("bomwork_id").toInt(),
                                          availability.value("bomitem_seqnumber"), availability.value("item_number"),
                                          availability.value("item_descrip"), availability.value("uom_name"),
                                          availability.value("f_pendalloc"), availability.value("f_totalalloc"),
                                          availability.value("f_ordered"), availability.value("f_qoh"),
                                          availability.value("f_totalavail")  );
  
              else
              {
                //  March though the existing list, looking for the parent for the current bomwork
		for (int i = 0; i < _availability->topLevelItemCount(); i++)
		{
		  XTreeWidgetItem *cursor = _availability->topLevelItem(i);
		  if (cursor->id() == availability.value("bomwork_parent_id").toInt())
		  {
		    //  Found it, add the current bomwork as a child of its parent
		    last = new XTreeWidgetItem(cursor, availability.value("bomwork_id").toInt(),
					      availability.value("bomitem_seqnumber"), availability.value("item_number"),
					      availability.value("item_descrip"), availability.value("uom_name"),
					      availability.value("f_pendalloc"), availability.value("f_totalalloc"),
					      availability.value("f_ordered"), availability.value("f_qoh"),
					      availability.value("f_totalavail")  );
		    break;
		  }
		}
	      }

              if (availability.value("qoh").toDouble() < availability.value("pendalloc").toDouble())
                last->setTextColor(7, "red");
    
              if (availability.value("totalavail").toDouble() < 0.0)
                last->setTextColor(8, "red");
              else if (availability.value("totalavail").toDouble() <= availability.value("reorderlevel").toDouble())
                last->setTextColor(8, "orange");
            }
      
            //  All done with the bomwork set, delete it
            availability.prepare("SELECT deleteBOMWorkset(:bomwork_set_id) AS result;");
            availability.bindValue(":bomwork_set_id", _worksetid);
            availability.exec();
	    if (availability.first())
	    {
	      int result = availability.value("result").toInt();
	      if (result < 0)
	      {
		systemError(this, storedProcErrorLookup("deleteBOMWorkset", result),
			    __FILE__, __LINE__);
		return;
	      }
	    }
	    else if (availability.lastError().type() != QSqlError::None)
	    {
	      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
          }
        }
        else
        {
          int itemsiteid = availability.value("itemsite_id").toInt();
          availability.prepare( "SELECT itemsiteid, reorderlevel,"
                                "       bomitem_seqnumber, item_number, item_descrip, uom_name,"
                                "       pendalloc, formatQty(pendalloc) AS f_pendalloc,"
                                "       ordered, formatQty(ordered) AS f_ordered,"
                                "       qoh, formatQty(qoh) AS f_qoh,"
                                "       formatQty(totalalloc + pendalloc) AS f_totalalloc,"
                                "       (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
                                "       formatQty(qoh + ordered - (totalalloc + pendalloc)) AS f_totalavail "
                                "FROM ( SELECT cs.itemsite_id AS itemsiteid,"
                                "              CASE WHEN(cs.itemsite_useparams) THEN cs.itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
                                "              bomitem_seqnumber, item_number,"
                                "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
                                "              ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap))) * :qty) AS pendalloc,"
                                "              (qtyAllocated(cs.itemsite_id, DATE(:schedDate)) - ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap))) * :origQtyOrd)) AS totalalloc,"
                                "              noNeg(cs.itemsite_qtyonhand) AS qoh,"
                                "              qtyOrdered(cs.itemsite_id, DATE(:schedDate)) AS ordered "
                                "       FROM itemsite AS cs, itemsite AS ps, item, bomitem, uom "
                                "       WHERE ( (bomitem_item_id=item_id)"
                                "        AND (item_inv_uom_id=uom_id)"
                                "        AND (cs.itemsite_item_id=item_id)"
                                "        AND (cs.itemsite_warehous_id=ps.itemsite_warehous_id)"
                                "        AND (bomitem_parent_item_id=ps.itemsite_item_id)"
                                "        AND (:schedDate BETWEEN bomitem_effective AND (bomitem_expires-1))"
                                "        AND (ps.itemsite_id=:itemsite_id) ) ) AS data "
                                "ORDER BY bomitem_seqnumber;" );
          availability.bindValue(":itemsite_id", itemsiteid);
          availability.bindValue(":qty", _qtyOrdered->toDouble());
          availability.bindValue(":schedDate", _scheduledDate->date());
          availability.bindValue(":origQtyOrd", _originalQtyOrd);
          availability.exec();
	  XTreeWidgetItem *last = 0;
          while (availability.next())
          {
	    last = new XTreeWidgetItem(_availability, last, availability.value("itemsiteid").toInt(),
				       availability.value("bomitem_seqnumber"), availability.value("item_number"),
				       availability.value("item_descrip"), availability.value("uom_name"),
				       availability.value("f_pendalloc"), availability.value("f_totalalloc"),
				       availability.value("f_ordered"), availability.value("f_qoh"),
				       availability.value("f_totalavail")  );
  
            if (availability.value("qoh").toDouble() < availability.value("pendalloc").toDouble())
              last->setTextColor(7, "red");
  
            if (availability.value("totalavail").toDouble() < 0.0)
              last->setTextColor(8, "red");
            else if (availability.value("totalavail").toDouble() <= availability.value("reorderlevel").toDouble())
              last->setTextColor(8, "orange");
          }
        }
      }
      else
        _availability->setEnabled(FALSE);
    }
    else if (availability.lastError().type() != QSqlError::None)
    {
      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _onHand->clear();
    _allocated->clear();
    _unallocated->clear();
    _onOrder->clear();
    _available->clear();
  }
}

void transferOrderItem::populate()
{
  XSqlQuery item;

  if (_mode == cNew)
  {
    item.prepare("SELECT tohead.*, warehous_id, warehous_code "
		 "FROM whsinfo, tohead "
		 "WHERE ((tohead_src_warehous_id=warehous_id)"
		 "  AND  (tohead_id=:id));") ;
    item.bindValue(":id", _toheadid);
  }
  else
  {
    item.prepare("SELECT toitem.*, tohead.*,"
		 "       warehous_id, warehous_code,"
		 "       stdCost(toitem_item_id) AS stdcost,"
		 "       itemsite_id,"
		 "       (SELECT COALESCE(SUM(shipitem_qty), 0)"
		 "          FROM shipitem, shiphead"
		 "         WHERE ((shipitem_shiphead_id=shiphead_id)"
		 "           AND  (shiphead_order_type='TO')"
		 "           AND  (shipitem_orderitem_id=toitem_id))) AS shipitem_qty "
		 "FROM toitem, whsinfo, tohead, itemsite "
		 "WHERE ((toitem_tohead_id=tohead_id)"
		 "  AND  (tohead_src_warehous_id=warehous_id)"
		 "  AND  (itemsite_item_id=toitem_item_id)"
		 "  AND  (itemsite_warehous_id=tohead_src_warehous_id)"
		 "  AND  (toitem_id=:id));");
    item.bindValue(":id", _toitemid);
  }

  item.exec();
  if (item.first())
  {
    _toheadid	= item.value("tohead_id").toInt();
    _taxauthid	= item.value("tohead_taxauth_id").toInt();
    _transwhsid	= item.value("tohead_trns_warehous_id").toInt();
    _dstwhsid	= item.value("tohead_dest_warehous_id").toInt();
    _orderNumber->setText(item.value("tohead_number").toString());

    _warehouse->clear();
    _warehouse->append(item.value("warehous_id").toInt(),
		       item.value("warehous_code").toString());

    if (_mode == cEdit || _mode == cView)
    {
      _itemsiteid	= item.value("itemsite_id").toInt();
      _comments->setId(_toitemid);
      _lineNumber->setText(item.value("toitem_linenumber").toString());
      _stdcost->setBaseValue(item.value("stdcost").toDouble());
      _shippedToDate->setText(formatQty(item.value("shipitem_qty").toDouble()));

      // do tax stuff before _qtyOrdered so signal cascade has data to work with
      _taxCache.setFreightId(item.value("toitem_freighttax_id").toInt());
      _qtyOrdered->setText(formatQty(item.value("toitem_qty_ordered").toDouble()));
      _scheduledDate->setDate(item.value("toitem_schedshipdate").toDate());
      _notes->setText(item.value("toitem_notes").toString());
      if (!item.value("toitem_schedrecvdate").isNull() && _metrics->boolean("UsePromiseDate"))
	_promisedDate->setDate(item.value("toitem_schedrecvdate").toDate());
      _freight->set(item.value("toitem_freight").toDouble(),
		    item.value("tohead_freight_curr_id").toInt(),
		    item.value("tohead_orderdate").toDate());
      _originalQtyOrd	= _qtyOrdered->toDouble();

      //  Set the _item here to tickle signal cascade
      _item->setId(item.value("toitem_item_id").toInt());

      sLookupTax();
      sDetermineAvailability();
    }
  }
  else if (item.lastError().type() != QSqlError::None)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if( (cNew == _mode) || ((cEdit == _mode) &&
			  (item.value("toitem_status").toString() == "O")) )
    _cancel->setEnabled(_shippedToDate->text().toDouble()==0.0);
  else
    _cancel->setEnabled(false);
}

void transferOrderItem::sNext()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();

  if(cNew == _mode)
  {
    _modified = false;
    return;
  }

  q.prepare("SELECT a.toitem_id AS id"
	    "  FROM toitem AS a, toitem AS b"
	    " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	    "   AND  (a.toitem_linenumber > b.toitem_linenumber)"
	    "   AND  (b.toitem_id=:id))"
	    " ORDER BY a.toitem_linenumber"
	    " LIMIT 1;");
  q.bindValue(":id", _toitemid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("toitem_id", q.value("id").toInt());

    if(cNew == _mode || cEdit == _mode)
      params.append("mode", "edit");
    else
      params.append("mode", "view");

    set(params);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (cView != _mode)
  {
    ParameterList params;
    params.append("tohead_id", _toheadid);
    params.append("mode", "new");
    set(params);
  }
}


void transferOrderItem::sPrev()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }

  clear();
  prepare();
  _item->setFocus();


  if(cNew == _mode)
    q.prepare("SELECT toitem_id AS id"
	      "  FROM toitem"
	      " WHERE (toitem_tohead_id=:tohead_id)"
	      " ORDER BY toitem_linenumber DESC"
	      " LIMIT 1;");
  else
    q.prepare("SELECT a.toitem_id AS id"
	      "  FROM toitem AS a, toitem AS b"
	      " WHERE ((a.toitem_tohead_id=b.toitem_tohead_id)"
	      "   AND  (a.toitem_linenumber < b.toitem_linenumber)"
	      "   AND  (b.toitem_id=:id))"
	      " ORDER BY a.toitem_linenumber DESC"
	      " LIMIT 1;");

  q.bindValue(":id", _toitemid);
  q.bindValue(":tohead_id", _toheadid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("toitem_id", q.value("id").toInt());

    if(cNew == _mode || cEdit == _mode)
      params.append("mode", "edit");
    else
      params.append("mode", "view");

    set(params);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrderItem::sChanged()
{
  _modified = true;
}

void transferOrderItem::reject()
{
  if(_modified)
  {
    switch( QMessageBox::question( this, tr("Unsaved Changed"),
              tr("<p>You have made some changes which have not yet been saved! "
                 "Would you like to save them now?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No,
              QMessageBox::Cancel | QMessageBox::Escape ) )
    {
      case QMessageBox::Yes:
        sSave();
        if(_modified) // catch an error saving
          return;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return;
    }
  }
  QDialog::reject();
}

void transferOrderItem::sCancel()
{
  _canceling = true;
  
  sSave();
  if(_error) 
    return;

  q.prepare("UPDATE toitem SET toitem_status='X' WHERE (toitem_id=:toitem_id);");
  q.bindValue(":toitem_id", _toitemid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  clear();
  prepare();
  _prev->setEnabled(true);
  _item->setFocus();

  _modified = false;
  _canceling = false;
}

void transferOrderItem::sLookupTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT tax_ratea, tax_rateb, tax_ratec, tax_id,"
		"       calculateTax(tax_id, :ext, 0, 'A') AS tax_a,"
		"       calculateTax(tax_id, :ext, 0, 'B') AS tax_b,"
		"       calculateTax(tax_id, :ext, 0, 'C') AS tax_c "
		"FROM tax "
		"WHERE (tax_id=getFreightTaxSelection(:auth));");

  calcq.bindValue(":auth", _taxauthid);
  calcq.bindValue(":ext",  _freight->localValue());
  calcq.exec();
  if (calcq.first())
  {
    _taxCache.setFreightId(calcq.value("tax_id").toInt());
    _taxCache.setFreightPct(calcq.value("tax_ratea").toDouble(),
			    calcq.value("tax_rateb").toDouble(),
			    calcq.value("tax_ratec").toDouble());
    _taxCache.setFreight(calcq.value("tax_a").toDouble(),
			 calcq.value("tax_b").toDouble(),
			 calcq.value("tax_c").toDouble());
    _tax->setLocalValue(_taxCache.total());
  }
  else if (calcq.lastError().type() != QSqlError::None)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrderItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("tax_id",   _taxCache.freightId());
  params.append("curr_id",  _tax->id());
  params.append("valueA",   _taxCache.freight(0));
  params.append("valueB",   _taxCache.freight(1));
  params.append("valueC",   _taxCache.freight(2));
  params.append("pctA",	    _taxCache.freightPct(0));
  params.append("pctB",	    _taxCache.freightPct(1));
  params.append("pctC",	    _taxCache.freightPct(2));
  params.append("subtotal", CurrDisplay::convert(_freight->id(), _tax->id(),
						 _freight->localValue(),
						 _freight->effective()));
  if(cView == _mode)
    params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    _taxCache.setFreight(newdlg.amountA(), newdlg.amountB(), newdlg.amountC());
    _taxCache.setFreightPct(newdlg.pctA(), newdlg.pctB(),    newdlg.pctC());

    if (_taxCache.freightId() != newdlg.tax())
      _taxCache.setFreightId(newdlg.tax());

    _tax->setLocalValue(_taxCache.total());
  }
}
