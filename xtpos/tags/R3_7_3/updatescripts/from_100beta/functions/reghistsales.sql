CREATE OR REPLACE FUNCTION xtpos.reghistSales(integer) RETURNS NUMERIC AS $$
DECLARE
  pReghistId 	ALIAS FOR $1;
  _returnVal	NUMERIC;
BEGIN
  SELECT xtpos.reghistSales(pReghistId, NULL) INTO _returnVal;
  
  RETURN _returnVal;
END;
$$ LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION xtpos.reghistSales(integer, text) RETURNS NUMERIC AS $$
DECLARE
  pReghistId 	ALIAS FOR $1;
  pType		ALIAS FOR $2;
  _totalSales  	NUMERIC;
  _creditSales	NUMERIC;
  _checkSales	NUMERIC;
  _returnVal	NUMERIC;

BEGIN
  -- Validate
  IF (pReghistId IS NULL) THEN
    RETURN 0;
  ELSIF (pType NOT IN (NULL,'S','K','C')) THEN
    RAISE EXCEPTION 'Invalid sale type % passed.  Valid types are NULL, "S" (cash), "K" (check), "C" (credit)',pType;
  END IF;
  
  -- Fetch values
  IF (pType IS NULL OR pType = 'S') THEN
    -- Sum of all sales
    SELECT COALESCE(SUM(round(saleitem_qty * saleitem_unitprice,2) + 
      saleitem_tax_ratea + saleitem_tax_rateb + saleitem_tax_ratec),0) 
      INTO _totalSales
    FROM xtpos.salehead
      JOIN xtpos.reghist ON ((salehead_reghist_id=reghist_id) 
                          AND (reghist_id=pReghistId))
      JOIN xtpos.saleitem ON (saleitem_salehead_id=salehead_id)
    WHERE (salehead_closed);
  END IF;

  IF (pType IS NULL) THEN
    _returnVal := _totalSales;
  ELSE
    -- Sum of non cash sales
    SELECT COALESCE(SUM(salehead_checkamt),0), COALESCE(SUM(COALESCE(ccpay_amount,0)),0) INTO _checkSales,_creditSales
    FROM xtpos.salehead
      JOIN xtpos.reghist ON ((salehead_reghist_id=reghist_id) 
                          AND (reghist_id=pReghistId))
      LEFT OUTER JOIN ccpay ON (salehead_ccpay_id=ccpay_id)
      WHERE (salehead_closed);

    IF (pType = 'S') THEN
      _returnVal := _totalSales - _creditSales - _checkSales;
    ELSIF (pType = 'C') THEN
      _returnVal := _creditSales;
    ELSE
      _returnVal := _checkSales;
    END IF;
  END IF;
  
  RETURN _returnVal;
END;
$$ LANGUAGE 'plpgsql';
