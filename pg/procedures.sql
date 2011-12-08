CREATE OR REPLACE FUNCTION title_before_insert_trigger() RETURNS TRIGGER AS $$
BEGIN
	PERFORM money FROM all_users WHERE id = NEW.user_id AND money > NEW.paid FOR UPDATE;
	IF FOUND THEN
		UPDATE all_users SET money = money - NEW.paid WHERE id = NEW.user_id;
		RETURN NEW;
	ELSE
		RAISE 'insufficient money';
		RETURN NULL;
	END IF;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS title_before_insert_trigger ON titles;
CREATE TRIGGER title_before_insert_trigger BEFORE INSERT ON titles
	FOR EACH ROW EXECUTE PROCEDURE title_before_insert_trigger();
