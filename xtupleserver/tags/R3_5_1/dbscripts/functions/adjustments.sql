CREATE OR REPLACE FUNCTION adjustments(TEXT) RETURNS BOOLEAN AS '
DECLARE
  pTransType ALIAS FOR $1;

BEGIN
  IF (pTransType IN (''CC'', ''AD'')) THEN
    RETURN TRUE;
  ELSE
    RETURN FALSE;
  END IF;

END;
' LANGUAGE 'plpgsql';
