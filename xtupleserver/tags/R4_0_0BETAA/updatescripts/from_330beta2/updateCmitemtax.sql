BEGIN;

-- All Credit Memo taxes should be negative
UPDATE cmitemtax SET taxhist_tax = (taxhist_tax * -1.0)
WHERE (taxhist_tax > 0);

COMMIT;