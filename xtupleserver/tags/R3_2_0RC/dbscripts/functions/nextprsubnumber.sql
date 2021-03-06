CREATE OR REPLACE FUNCTION nextPrSubnumber(INTEGER) RETURNS INTEGER AS '
DECLARE
  pPrNumber ALIAS FOR $1;
  _subNumber INTEGER;

BEGIN

  SELECT MAX(pr_subnumber) INTO _subNumber
  FROM pr
  WHERE (pr_number=pPrNumber);

  IF (_subNumber IS NULL)
    THEN _subNumber := 0;
  END IF;

  RETURN (_subNumber + 1);

END;
' LANGUAGE 'plpgsql';
