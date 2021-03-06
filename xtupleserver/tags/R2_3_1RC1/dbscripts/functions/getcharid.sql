CREATE OR REPLACE FUNCTION getCharId(text, text) RETURNS INTEGER AS '
DECLARE
  pChar ALIAS FOR $1;
  pType ALIAS FOR $2;
  _returnVal INTEGER;
BEGIN
  IF (pChar IS NULL) THEN
    RETURN NULL;
  END IF;

  SELECT char_id INTO _returnVal
  FROM char
  WHERE ((char_name=pChar)
  AND ((pType=''C'' AND char_customers)
    OR (pType IN (''I'',''SI'',''QI'',''W'') AND char_items)
    OR (pType=''CRMACCT'' AND char_crmaccounts)
    OR (pType=''ADDR'' AND char_addresses)
    OR (pType=''CNTCT'' AND char_contacts))) LIMIT 1;

  IF (_returnVal IS NULL) THEN
	RAISE EXCEPTION ''Characteristic % not found.'', pChar;
  END IF;

  RETURN _returnVal;
END;
' LANGUAGE 'plpgsql';
