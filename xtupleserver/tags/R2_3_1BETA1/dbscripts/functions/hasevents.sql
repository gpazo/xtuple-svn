
CREATE OR REPLACE FUNCTION hasEvents() RETURNS BOOLEAN AS '
BEGIN

  PERFORM evntlog_id
  FROM evntlog
  WHERE ( (evntlog_dispatched IS NULL)
   AND (evntlog_username=CURRENT_USER) )
  LIMIT 1;
  RETURN FOUND;

END;
' LANGUAGE 'plpgsql';

