CREATE OR REPLACE FUNCTION _poitemTrigger() RETURNS TRIGGER AS '
DECLARE
  _cmnttypeid INTEGER;
BEGIN

IF (TG_OP = ''INSERT'') THEN
       
--       Insert Event Start
       
          INSERT INTO evntlog ( evntlog_evnttime, evntlog_username, evntlog_evnttype_id,
                          evntlog_ordtype, evntlog_ord_id, evntlog_warehous_id, evntlog_number )
          SELECT CURRENT_TIMESTAMP, evntnot_username, evnttype_id,
                     ''P'', NEW.poitem_id, itemsite_warehous_id, (pohead_number || ''-'' || NEW.poitem_linenumber || '': '' || item_number)
          FROM evntnot, evnttype, itemsite, item, pohead
          WHERE ( (evntnot_evnttype_id=evnttype_id)
            AND (evntnot_warehous_id=itemsite_warehous_id)
            AND (itemsite_id=NEW.poitem_itemsite_id)
            AND (itemsite_item_id=item_id)
            AND (NEW.poitem_pohead_id=pohead_id)
            AND (NEW.poitem_duedate <= (CURRENT_DATE + itemsite_eventfence))
            AND (evnttype_name=''POitemCreate'') );

--       Insert Event End

END IF;

  IF ( SELECT (metric_value=''t'')
       FROM metric
       WHERE (metric_name=''POChangeLog'') ) THEN

--  Cache the cmnttype_id for ChangeLog
    SELECT cmnttype_id INTO _cmnttypeid
    FROM cmnttype
    WHERE (cmnttype_name=''ChangeLog'');
    IF (FOUND) THEN
      IF (TG_OP = ''INSERT'') THEN
        PERFORM postComment(_cmnttypeid, ''P'', NEW.poitem_pohead_id, (''Created Line #'' || NEW.poitem_linenumber::TEXT));

      ELSIF (TG_OP = ''UPDATE'') THEN
        IF (NEW.poitem_qty_ordered <> OLD.poitem_qty_ordered) THEN
          PERFORM postComment( _cmnttypeid, ''PI'', NEW.poitem_id,
                               ( ''Qty. Ordered Changed from '' || formatQty(OLD.poitem_qty_ordered) ||
                                 '' to '' || formatQty(NEW.poitem_qty_ordered ) ) );
        END IF;

      ELSIF (TG_OP = ''DELETE'') THEN
        PERFORM postComment(_cmnttypeid, ''P'', OLD.poitem_pohead_id, (''Deleted Line #'' || OLD.poitem_linenumber::TEXT));
      END IF;
    END IF;
  END IF;

  IF (TG_OP = ''DELETE'') THEN
    DELETE FROM comment
     WHERE ( (comment_source=''PI'')
       AND   (comment_source_id=OLD.poitem_id) );

    DELETE FROM charass
     WHERE ((charass_target_type=''PI'')
       AND  (charass_target_id=OLD.poitem_id));

    IF (OLD.poitem_status = ''O'') THEN
      IF ( (SELECT (count(*) < 1)
              FROM poitem
             WHERE ((poitem_pohead_id=OLD.poitem_pohead_id)
               AND  (poitem_id != OLD.poitem_id)
               AND  (poitem_status <> ''C'')) ) ) THEN
        UPDATE pohead SET pohead_status = ''C''
         WHERE ((pohead_id=OLD.poitem_pohead_id)
           AND  (pohead_status=''O''));
      END IF;
    END IF;

    RETURN OLD;
  ELSE
    IF (TG_OP = ''UPDATE'') THEN
      IF (OLD.poitem_status <> NEW.poitem_status) THEN
        IF ( (SELECT (count(*) < 1)
                FROM poitem
               WHERE ((poitem_pohead_id=NEW.poitem_pohead_id)
                 AND  (poitem_id != NEW.poitem_id)
                 AND  (poitem_status<>''C'')) ) AND (NEW.poitem_status=''C'') ) THEN
          UPDATE pohead SET pohead_status = ''C''
           WHERE ((pohead_id=NEW.poitem_pohead_id)
             AND  (pohead_status=''O''));
        ELSE
          UPDATE pohead SET pohead_status = ''O''
           WHERE ((pohead_id=NEW.poitem_pohead_id)
             AND  (pohead_status=''C''));
        END IF;
      END IF;
    END IF;

    RETURN NEW;
  END IF;

END;
' LANGUAGE 'plpgsql';

DROP TRIGGER poitemTrigger ON poitem;
CREATE TRIGGER poitemTrigger BEFORE INSERT OR UPDATE OR DELETE ON poitem FOR EACH ROW EXECUTE PROCEDURE _poitemTrigger();
