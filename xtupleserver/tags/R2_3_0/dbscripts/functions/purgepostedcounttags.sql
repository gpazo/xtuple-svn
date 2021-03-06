CREATE OR REPLACE FUNCTION purgePostedCountTags(DATE, INTEGER) RETURNS INTEGER AS '
DECLARE
  pCutoffDate ALIAS FOR $1;
  pWarehousid ALIAS FOR $2;

BEGIN

  IF (pWarehousid = -1) THEN
    DELETE FROM cntslip
    WHERE (cntslip_cnttag_id IN ( SELECT invcnt_id
                                  FROM invcnt
                                  WHERE ( (invcnt_posted)
                                   AND (invcnt_postdate <= pCutoffDate) ) ) );

    DELETE FROM invcnt
    WHERE ((invcnt_posted)
     AND (invcnt_postdate <= pCutoffDate));

  ELSE
    DELETE FROM cntslip
    WHERE (cntslip_cnttag_id IN ( SELECT invcnt_id
                                  FROM invcnt, itemsite
                                  WHERE ( (invcnt_posted)
                                   AND (invcnt_itemsite_id=itemsite_id)
                                   AND (invcnt_postdate <= pCutoffDate)
                                   AND (itemsite_warehous_id=pWarehousid) ) ) );

    DELETE FROM invcnt
    WHERE (invcnt_id IN ( SELECT invcnt_id 
                          FROM invcnt, itemsite
                          WHERE ( (invcnt_posted)
                           AND (invcnt_itemsite_id=itemsite_id)
                           AND (invcnt_postdate <= pCutoffDate)
                           AND (itemsite_warehous_id=pWarehousid) ) ) );
  END IF;

  RETURN 1;

END;
' LANGUAGE 'plpgsql';
