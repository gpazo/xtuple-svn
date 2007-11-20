BEGIN;

-- Sales Order Line

--DROP VIEW api.salesline;
CREATE VIEW api.salesline
AS 
  SELECT 
     cohead_number AS order_number,
     coitem_linenumber AS line_number,
     l.item_number AS item_number,
     coitem_custpn AS customer_pn,
     s.item_number AS substitute_for,
     warehous_code AS sold_from_whs,
     coitem_status AS status,
     coitem_qtyord AS qty_ordered,
     coitem_price AS net_unit_price,
     coitem_scheddate AS scheduled_date,
     COALESCE((
       SELECT taxtype_name
       FROM taxtype
       WHERE (taxtype_id=getItemTaxType(l.item_id, cohead_taxauth_id))),'None') AS tax_type,
     COALESCE(tax_code,'None') AS tax_code,
     CASE
       WHEN coitem_price = 0 THEN
         '100'
       WHEN coitem_custprice = 0 THEN
         'N/A'
       ELSE
         CAST(ROUND(((1 - coitem_price / coitem_custprice) * 100),4) AS text)
     END AS discount_pct_from_list,
     CASE
       WHEN (coitem_order_id = -1) THEN
         false
       ELSE
         true
     END AS create_order,
     coitem_prcost AS overwrite_po_price,
     coitem_memo AS notes
  FROM cohead, coitem
    LEFT OUTER JOIN itemsite isb ON (coitem_substitute_item_id=isb.itemsite_id)
    LEFT OUTER JOIN item s ON (isb.itemsite_item_id=s.item_id)
    LEFT OUTER JOIN tax ON (coitem_tax_id=tax_id),
  itemsite il, item l, whsinfo
  WHERE ((cohead_id=coitem_cohead_id)
  AND (coitem_itemsite_id=il.itemsite_id)
  AND (il.itemsite_item_id=l.item_id)
  AND (il.itemsite_warehous_id=warehous_id))
ORDER BY cohead_number,coitem_linenumber;
    

GRANT ALL ON TABLE api.salesline TO openmfg;
COMMENT ON VIEW api.salesline IS '
This view can be used as an interface to import Sales Order Line Items data directly  
into the system.  Required fields will be checked';

--Rules

CREATE OR REPLACE RULE "_INSERT" AS
    ON INSERT TO api.salesline DO INSTEAD

  INSERT INTO coitem (
    coitem_cohead_id,
    coitem_linenumber,
    coitem_itemsite_id,
    coitem_status,
    coitem_scheddate,
    coitem_promdate,
    coitem_qtyord,
    coitem_qtyshipped,
    coitem_unitcost,
    coitem_price,
    coitem_custprice,
    coitem_order_id,
    coitem_memo,
    coitem_imported,
    coitem_qtyreturned,
    coitem_custpn,
    coitem_order_type,
    coitem_substitute_item_id,
    coitem_prcost,
    coitem_tax_id)
  SELECT
    getSalesOrderId(NEW.order_number),
    COALESCE(NEW.line_number,(
      SELECT (COALESCE(MAX(coitem_linenumber), 0) + 1)
              FROM coitem
              WHERE (coitem_cohead_id=getSalesOrderId(NEW.order_number)))),
    itemsite_id,
    COALESCE(NEW.status,'O'),
    COALESCE(NEW.scheduled_date,(
      SELECT MIN(coitem_scheddate)
      FROM coitem
      WHERE (coitem_cohead_id=getSalesOrderId(NEW.order_number)))),
    endoftime(),
    NEW.qty_ordered,
    0,
    stdCost(item_id),
    COALESCE(NEW.net_unit_price,itemPrice(getItemId(NEW.item_number),cohead_cust_id,
             cohead_shipto_id,NEW.qty_ordered,cohead_curr_id,cohead_orderdate)),
    itemPrice(getItemId(NEW.item_number),cohead_cust_id,
             cohead_shipto_id,NEW.qty_ordered,cohead_curr_id,cohead_orderdate),
    -1,
    COALESCE(NEW.notes,''),
    true,
    0,
    NEW.customer_pn,
    CASE
      WHEN ((NEW.create_order  AND (item_type = 'M')) OR 
           ((NEW.create_order IS NULL) AND itemsite_createwo)) THEN
        'W'
      WHEN ((NEW.create_order AND (item_type = 'P')) OR 
           ((NEW.create_order IS NULL) AND itemsite_createpr)) THEN
        'R'
    END,
    getItemsiteId(warehous_code,NEW.substitute_for),
    COALESCE(NEW.overwrite_po_price,0),
    COALESCE(getTaxId(NEW.tax_code),(
             SELECT tax_id
             FROM tax
             WHERE ((tax_id=getTaxSelection(cohead_taxauth_id,
	           getItemTaxType(itemsite_item_id, cohead_taxauth_id))))))
  FROM cohead, itemsite, item, whsinfo
  WHERE ((cohead_id=getSalesOrderId(NEW.order_number))
  AND (itemsite_id=getItemsiteId(COALESCE(NEW.sold_from_whs,(
                                SELECT warehous_code 
                                FROM usrpref, whsinfo
                                WHERE ((warehous_id=CAST(usrpref_value AS INTEGER))
                                AND (warehous_active)
                                AND (usrpref_username=current_user)
                                AND (usrpref_name='PreferredWarehouse'))),''),
                                COALESCE(NEW.item_number,(
                                SELECT item_number
                                FROM item, itemalias
                                WHERE ((item_id=itemalias_item_id)
                                AND (itemalias_number=NEW.customer_pn)))),
                                'SOLD'))
  AND (itemsite_item_id=item_id)
  AND (warehous_id=itemsite_warehous_id));

CREATE OR REPLACE RULE "_UPDATE" AS 
    ON UPDATE TO api.salesline DO INSTEAD

  UPDATE coitem SET
    coitem_status=NEW.status,
    coitem_scheddate=NEW.scheduled_date,
    coitem_qtyord=NEW.qty_ordered,
    coitem_price=NEW.net_unit_price,
    coitem_memo=NEW.notes,
    coitem_order_type=
    CASE
      WHEN (NOT OLD.create_order AND NEW.create_order  AND (item_type = 'M')) THEN
        'W'
      WHEN (NOT OLD.create_order AND NEW.create_order AND (item_type = 'P')) THEN
        'R'     
    END,
    coitem_substitute_item_id=getItemsiteId(NEW.sold_from_whs,NEW.item_number),
    coitem_prcost=NEW.overwrite_po_price,
    coitem_tax_id=getTaxId(NEW.tax_code)
   FROM item
   WHERE ((item_id=getItemId(OLD.item_number))
   AND (coitem_cohead_id=getSalesOrderId(OLD.order_number))
   AND (coitem_linenumber=OLD.line_number));

CREATE OR REPLACE RULE "_DELETE" AS 
    ON DELETE TO api.salesline DO INSTEAD

  DELETE FROM coitem
  WHERE ((coitem_cohead_id=getSalesOrderId(OLD.order_number))
  AND (coitem_linenumber=OLD.line_number));

COMMIT;
