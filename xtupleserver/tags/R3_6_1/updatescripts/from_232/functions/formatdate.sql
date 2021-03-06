
CREATE OR REPLACE FUNCTION formatDate(TIMESTAMP WITH TIME ZONE) RETURNS TEXT IMMUTABLE AS '
SELECT TO_CHAR($1, COALESCE((SELECT locale_dateformat
                             FROM locale, usr
                             WHERE ((usr_locale_id=locale_id)
                              AND (usr_username=CURRENT_USER)) ),
                            ''yyyy-mm-dd'' )) AS result
' LANGUAGE 'sql';


CREATE OR REPLACE FUNCTION formatDate(DATE) RETURNS TEXT IMMUTABLE AS '
SELECT TO_CHAR($1, COALESCE((SELECT locale_dateformat
                             FROM locale, usr
                             WHERE ((usr_locale_id=locale_id)
                              AND (usr_username=CURRENT_USER)) ),
                            ''yyyy-mm-dd'') ) AS result
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION formatDate(DATE, TEXT) RETURNS TEXT IMMUTABLE AS '
DECLARE
  pDate ALIAS FOR $1;
  pString ALIAS FOR $2;

BEGIN

  IF ( (pDate = startOfTime()) OR
       (pDate = endOfTime()) OR
       (pDate IS NULL) ) THEN
    RETURN pString;
  ELSE
    RETURN formatDate(pDate);
  END IF;

END;
' LANGUAGE 'plpgsql';

