
CREATE OR REPLACE FUNCTION fetchARMemoNumber() RETURNS TEXT AS '
DECLARE
  _number TEXT;
  _test INTEGER;

BEGIN

  LOOP

    SELECT CAST(orderseq_number AS text) INTO _number
    FROM orderseq
    WHERE (orderseq_name=''ARMemoNumber'');
    IF (NOT FOUND) THEN
      RETURN -1;
    END IF;

    UPDATE orderseq
    SET orderseq_number = (orderseq_number + 1)
    WHERE (orderseq_name=''ARMemoNumber'');

    SELECT aropen_id INTO _test
    FROM aropen
    WHERE ( (aropen_doctype IN (''D'', ''C'', ''R''))
     AND (aropen_docnumber=_number) );

    IF (NOT FOUND) THEN
      EXIT;
    END IF;

  END LOOP;

  RETURN _number;

END;
' LANGUAGE 'plpgsql';

