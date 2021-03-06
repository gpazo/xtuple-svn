CREATE OR REPLACE FUNCTION releaseNumber(TEXT, INTEGER) RETURNS INTEGER AS '
DECLARE
  psequence	ALIAS FOR $1;
  pnumber	ALIAS FOR $2;
  _test		INTEGER;
  _number	INTEGER;
  _table	TEXT;
  _numcol	TEXT;

BEGIN
  -- get the current state of the sequence
  SELECT orderseq_number, orderseq_table, orderseq_numcol
      INTO _number, _table, _numcol
  FROM orderseq
  WHERE (orderseq_name=psequence);
  IF (NOT FOUND) THEN
    RAISE EXCEPTION ''Invalid orderseq_name %'', psequence;
  END IF;

  -- check if an order exists with the given order number
  EXECUTE ''SELECT '' || quote_ident(_numcol) ||
	  '' FROM ''  || quote_ident(_table) ||
	  '' WHERE ('' || quote_ident(_numcol) || ''='' || _number || '');''
  INTO _test;

  IF (NOT FOUND) THEN
    RETURN 0;
  END IF;

  -- check if order seq has been incremented past the given order number
  -- S/O code reads: IF ((_test - 1) <> pSoNumber) THEN
  -- but the following /should/ address bug 4020 (can''t reproduce it to test)
  IF ((_test - 1) > pnumber) THEN
    RETURN 0;
  END IF;

  -- release the given order number for reuse
  UPDATE orderseq
  SET orderseq_number = (orderseq_number - 1)
  WHERE (orderseq_name=psequence);

  RETURN 1;

END;
' LANGUAGE 'plpgsql';
