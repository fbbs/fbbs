CREATE OR REPLACE FUNCTION prop_record_before_insert_trigger() RETURNS TRIGGER AS $$
BEGIN
	PERFORM money FROM all_users WHERE id = NEW.user_id AND money >= NEW.price FOR UPDATE;
	IF FOUND THEN
		UPDATE all_users SET money = money - NEW.price WHERE id = NEW.user_id;
		INSERT INTO audit.money (user_id, delta, stamp, reason) VALUES (NEW.user_id, -NEW.price, current_timestamp, 'buy ' || NEW.item);
		RETURN NEW;
	ELSE
		RAISE 'insufficient money';
	END IF;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS prop_record_before_insert_trigger ON prop_records;
CREATE TRIGGER prop_record_before_insert_trigger AFTER INSERT ON prop_records
    FOR EACH ROW EXECUTE PROCEDURE prop_record_before_insert_trigger();

CREATE OR REPLACE FUNCTION buy_title_request(_uid INTEGER, _item INTEGER, _title TEXT, _granter INTEGER) RETURNS VOID AS $$
DECLARE
	_row RECORD;
	_record prop_records.id%TYPE;
BEGIN
	SELECT price, expire INTO _row FROM prop_items WHERE id = _item;
	IF NOT FOUND THEN
		RAISE 'no item found';
	END IF;
	IF _item <> 1 THEN
		PERFORM t.id FROM titles t JOIN prop_records r ON t.record_id = r.id
				WHERE t.user_id = _uid AND r.item <> 1;
		IF FOUND THEN
			RAISE 'title exists';
		END IF;
	END IF;
	INSERT INTO prop_records (user_id, item, price, order_time, expire)
			VALUES (_uid, _item, _row.price, current_timestamp, current_timestamp + _row.expire);
	SELECT lastval() INTO _record;
	IF _granter <> 0 THEN
		INSERT INTO titles (user_id, granter, title, record_id, approved)
				VALUES (_uid, _granter, _title, _record, TRUE);
		UPDATE all_users SET title =
			(SELECT string_agg(title, ' ') FROM titles WHERE user_id = _uid AND approved)
			WHERE id = _uid;
	ELSE
		INSERT INTO titles (user_id, granter, title, record_id, approved)
				VALUES (_uid, _uid, _title, _record, FALSE);
	END IF;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION title_after_update_trigger() RETURNS TRIGGER AS $$
BEGIN
	UPDATE all_users SET title =
		(SELECT string_agg(title, ' ') FROM titles WHERE user_id = NEW.user_id AND approved)
		WHERE id = NEW.user_id;
	IF NEW.approved AND NOT OLD.approved THEN
		UPDATE prop_records r SET order_time = current_timestamp, expire = current_timestamp + i.expire
		FROM prop_items i WHERE r.id = NEW.record_id AND r.item = i.id;
	END IF;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS title_after_update_trigger ON titles;
CREATE TRIGGER title_after_update_trigger AFTER UPDATE ON titles
	FOR EACH ROW EXECUTE PROCEDURE title_after_update_trigger();

CREATE OR REPLACE FUNCTION title_after_delete_trigger() RETURNS TRIGGER AS $$
DECLARE
	_delta INTEGER := 0;
BEGIN
	IF OLD.approved THEN
		SELECT GREATEST(0, price / 2 * (1 - EXTRACT(EPOCH FROM (current_timestamp - order_time)) / EXTRACT(EPOCH FROM (expire - order_time)))) INTO _delta FROM prop_records WHERE id = OLD.record_id;
	ELSE
		SELECT price * 0.9 INTO _delta FROM prop_records WHERE id = OLD.record_id;
	END IF;

	UPDATE all_users SET money = money + _delta;
	INSERT INTO audit.money (user_id, delta, stamp, reason) VALUES (NEW.user_id, _delta, current_timestamp, 'D title');

	DELETE FROM prop_records WHERE id = OLD.record_id;
	UPDATE all_users SET title =
        (SELECT string_agg(title, ' ') FROM titles WHERE user_id = OLD.user_id AND approved)
        WHERE id = OLD.user_id;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS title_after_delete_trigger ON titles;
CREATE TRIGGER title_after_delete_trigger AFTER DELETE ON titles
    FOR EACH ROW EXECUTE PROCEDURE title_after_delete_trigger();
