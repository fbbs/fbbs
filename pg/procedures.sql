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

CREATE OR REPLACE FUNCTION title_after_update_trigger() RETURNS TRIGGER AS $$
BEGIN
	UPDATE all_users SET title =
		(SELECT string_agg(title, ' ') FROM titles WHERE user_id = NEW.user_id AND approved)
		WHERE id = NEW.user_id;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS title_after_update_trigger ON titles;
CREATE TRIGGER title_after_update_trigger AFTER UPDATE ON titles
	FOR EACH ROW EXECUTE PROCEDURE title_after_update_trigger();

CREATE OR REPLACE FUNCTION title_after_delete_trigger() RETURNS TRIGGER AS $$
BEGIN
	IF OLD.paid > 0 AND NOT OLD.approved THEN
		UPDATE all_users SET money = money + OLD.paid WHERE id = OLD.user_id;
	END IF;
	UPDATE all_users SET title =
        (SELECT string_agg(title, ' ') FROM titles WHERE user_id = NEW.user_id AND approved)
        WHERE id = OLD.user_id;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS title_after_delete_trigger ON titles;
CREATE TRIGGER title_after_delete_trigger AFTER DELETE ON titles
    FOR EACH ROW EXECUTE PROCEDURE title_after_delete_trigger();
