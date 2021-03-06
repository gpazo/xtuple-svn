
CREATE OR REPLACE FUNCTION openAccountingPeriod(INTEGER) RETURNS INTEGER AS '
DECLARE
  pPeriodid ALIAS FOR $1;
  _r RECORD;

BEGIN

--  Check to make use that the period is closed
  IF ( ( SELECT (NOT period_closed)
         FROM period
         WHERE (period_id=pPeriodid) ) ) THEN
    RETURN -1;
  END IF;

--  Check to make use that the period is not frozen
  IF ( ( SELECT period_freeze
         FROM period
         WHERE (period_id=pPeriodid) ) ) THEN
    RETURN -2;
  END IF;

--  Reset the period_closed flag
  UPDATE period
  SET period_closed=FALSE
  WHERE (period_id=pPeriodid);

--  Post any unposted G/L Transactions into the new period
  FOR _r IN SELECT DISTINCT gltrans_sequence
            FROM gltrans, period
            WHERE ( (NOT gltrans_posted)
             AND (gltrans_date BETWEEN period_start AND period_end)
             AND (period_id=pPeriodid) ) LOOP
    PERFORM postIntoTrialBalance(_r.gltrans_sequence);
  END LOOP;

  RETURN pPeriodid;

END;
' LANGUAGE 'plpgsql';

