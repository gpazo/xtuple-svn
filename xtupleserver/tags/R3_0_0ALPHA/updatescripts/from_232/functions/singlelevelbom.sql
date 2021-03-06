CREATE OR REPLACE FUNCTION singlelevelBOM(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS SETOF bomdata AS '
DECLARE
  pItemid ALIAS FOR $1;
  pRevisionid ALIAS FOR $2;
  pExpiredDays ALIAS FOR $3;
  pFutureDays ALIAS FOR $4;
  _row bomdata%ROWTYPE;
  _bomworksetid INTEGER;
  _x RECORD;
  _check CHAR(1);
  _inactive BOOLEAN;

BEGIN

  _inactive := FALSE;

  IF (pRevisionid != -1) THEN
    --Is this a deactivated revision?
    SELECT rev_status INTO _check
    FROM rev
    WHERE ((rev_id=pRevisionid)
    AND (rev_status=''I''));
    IF (FOUND) THEN
      _inactive := TRUE;
    END IF;
  END IF;
 
  IF NOT (_inactive) THEN
    FOR _x IN
        SELECT bomitem_id, bomitem_seqnumber, item_id, item_number, uom_name,
               item_descrip1, item_descrip2,
               (item_descrip1 || '' '' || item_descrip2) AS itemdescription,
               formatQtyper(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper)) AS f_qtyper,
               formatScrap(bomitem_scrap) AS f_scrap,
               formatBoolYN(bomitem_createwo) as createchild,
               CASE WHEN (bomitem_issuemethod=''S'') THEN ''Push''
                 WHEN (bomitem_issuemethod=''L'') THEN ''Pull''
                 WHEN (bomitem_issuemethod=''M'') THEN ''Mixed''
                 ELSE ''Special''
               END AS issuemethod,
               formatDate(bomitem_effective, ''Always'') AS f_effective,
               formatDate(bomitem_expires,''Never'') AS f_expires,
               CASE WHEN (bomitem_expires <= CURRENT_DATE) THEN TRUE
                 ELSE FALSE
               END AS expired,
               CASE WHEN (bomitem_effective > CURRENT_DATE) THEN TRUE
                 ELSE FALSE
               END AS future,
               actcost(bomitem_item_id) AS actunitcost,
               stdcost(bomitem_item_id) AS stdunitcost,
               (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * actcost(bomitem_item_id)) AS actextendedcost,
               (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * stdcost(bomitem_item_id)) AS stdextendedcost,
               bomitem_effective,
               bomitem_char_id,
               bomitem_value
       FROM bomitem(pItemid,pRevisionid), item, uom 
       WHERE ( (item_inv_uom_id=uom_id)
       AND (bomitem_item_id=item_id)
       AND (bomitem_expires > (CURRENT_DATE - pExpiredDays))
       AND (bomitem_effective <= (CURRENT_DATE + pFutureDays)) )
       UNION
       SELECT -1, -1, -1, costelem_type AS bomdata_item_number, '''','''', '''', '''',
              '''', '''', '''', '''', '''', '''',false,false,
              currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS actunitcost,
              itemcost_stdcost AS stdunitcost,
              currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS actextendedcost,
              itemcost_stdcost AS stdextendedcost,
              startoftime(), NULL, NULL
       FROM itemcost, costelem 
       WHERE ( (itemcost_costelem_id=costelem_id)
       AND (NOT itemcost_lowlevel)
       AND (itemcost_item_id=pItemid) )
       ORDER BY bomitem_seqnumber, bomitem_effective, item_number
    LOOP
        _row.bomdata_bomitem_id := _x.bomitem_id;
        _row.bomdata_bomwork_seqnumber := _x.bomitem_seqnumber;
        _row.bomdata_item_id := _x.item_id;
        _row.bomdata_item_number := _x.item_number;
        _row.bomdata_uom_name := _x.uom_name;
        _row.bomdata_item_descrip1 := _x.item_descrip1;
        _row.bomdata_item_descrip2 := _x.item_descrip2;
        _row.bomdata_itemdescription := _x.itemdescription;
        _row.bomdata_qtyper := _x.f_qtyper;
        _row.bomdata_scrap := _x.f_scrap;
        _row.bomdata_createchild := _x.createchild;
        _row.bomdata_issuemethod := _x.issuemethod;
        _row.bomdata_effective := _x.f_effective;
        _row.bomdata_expires := _x.f_expires;
        _row.bomdata_expired := _x.expired;
        _row.bomdata_future := _x.future;
        _row.bomdata_actunitcost := _x.actunitcost;
        _row.bomdata_stdunitcost := _x.stdunitcost;
        _row.bomdata_actextendedcost := _x.actextendedcost;
        _row.bomdata_stdextendedcost := _x.stdextendedcost;
        _row.bomdata_char_id := _x.bomitem_char_id;
        _row.bomdata_value := _x.bomitem_value;
        RETURN NEXT _row;
    END LOOP;

   ELSE

-- Use historical snapshot for inactive revisions
    FOR _x IN
        SELECT bomitem_id, bomitem_seqnumber, item_id, item_number, uom_name,
               item_descrip1, item_descrip2,
               (item_descrip1 || '' '' || item_descrip2) AS itemdescription,
               formatQtyper(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper)) AS f_qtyper,
               formatScrap(bomitem_scrap) AS f_scrap,
               formatBoolYN(bomitem_createwo) as createchild,
               CASE WHEN (bomitem_issuemethod=''S'') THEN ''Push''
                 WHEN (bomitem_issuemethod=''L'') THEN ''Pull''
                 WHEN (bomitem_issuemethod=''M'') THEN ''Mixed''
                 ELSE ''Special''
               END AS issuemethod,
               formatDate(bomitem_effective, ''Always'') AS f_effective,
               formatDate(bomitem_expires,''Never'') AS f_expires,
               CASE WHEN (bomitem_expires <= CURRENT_DATE) THEN TRUE
                 ELSE FALSE
               END AS expired,
               CASE WHEN (bomitem_effective > CURRENT_DATE) THEN TRUE
                 ELSE FALSE
               END AS future,
               actcost(bomitem_item_id) AS actunitcost,
               stdcost(bomitem_item_id) AS stdunitcost,
               (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * actcost(bomitem_item_id)) AS actextendedcost,
               (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * stdcost(bomitem_item_id)) AS stdextendedcost,
               bomitem_effective,
               bomitem_char_id,
               bomitem_value
       FROM bomitem(pItemid,pRevisionid), item, uom 
       WHERE ( (item_inv_uom_id=uom_id)
       AND (bomitem_item_id=item_id)
       AND (bomitem_expires > (CURRENT_DATE - pExpiredDays))
       AND (bomitem_effective <= (CURRENT_DATE + pFutureDays)) )
       UNION
       SELECT -1, -1, -1, costelem_type AS bomdata_item_number, '''','''', '''', '''',
              '''', '''', '''', '''', '''', '''',false,false,
              bomhist_actunitcost AS actunitcost,
              bomhist_stdunitcost AS stdunitcost,
              bomhist_actunitcost AS actextendedcost,
              bomhist_stdunitcost AS stdextendedcost,
              startoftime(), NULL, NULL
       FROM bomhist, costelem 
       WHERE ( (bomhist_item_id=costelem_id)
       AND (bomhist_item_type=''E'')
       AND (bomhist_rev_id=pRevisionid) )
       ORDER BY bomitem_seqnumber, bomitem_effective, item_number
    LOOP
        _row.bomdata_bomitem_id := _x.bomitem_id;
        _row.bomdata_bomwork_seqnumber := _x.bomitem_seqnumber;
        _row.bomdata_item_id := _x.item_id;
        _row.bomdata_item_number := _x.item_number;
        _row.bomdata_uom_name := _x.uom_name;
        _row.bomdata_item_descrip1 := _x.item_descrip1;
        _row.bomdata_item_descrip2 := _x.item_descrip2;
        _row.bomdata_itemdescription := _x.itemdescription;
        _row.bomdata_qtyper := _x.f_qtyper;
        _row.bomdata_scrap := _x.f_scrap;
        _row.bomdata_createchild := _x.createchild;
        _row.bomdata_issuemethod := _x.issuemethod;
        _row.bomdata_effective := _x.f_effective;
        _row.bomdata_expires := _x.f_expires;
        _row.bomdata_expired := _x.expired;
        _row.bomdata_future := _x.future;
        _row.bomdata_actunitcost := _x.actunitcost;
        _row.bomdata_stdunitcost := _x.stdunitcost;
        _row.bomdata_actextendedcost := _x.actextendedcost;
        _row.bomdata_stdextendedcost := _x.stdextendedcost;
        _row.bomdata_char_id := _x.bomitem_char_id;
        _row.bomdata_value := _x.bomitem_value;
        RETURN NEXT _row;
    END LOOP;
  END IF;

  RETURN;
END;
' LANGUAGE 'plpgsql';


