BEGIN;

DROP TABLE IF EXISTS payment;

CREATE TABLE payment (
	user_id INTEGER PRIMARY KEY,
	posts BIGINT DEFAULT 0,   -- paid posts in cents
	logins BIGINT DEFAULT 0,  -- paid logins in cents
	awards BIGINT DEFAULT 0   -- paid awards
);

CREATE OR REPLACE FUNCTION user_after_insert_trigger() RETURNS TRIGGER AS $$
BEGIN
	INSERT INTO payment (user_id) VALUES (NEW.id);
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS user_after_insert_trigger ON users;
CREATE TRIGGER user_after_insert_trigger AFTER INSERT ON users
	FOR EACH ROW EXECUTE PROCEDURE user_after_insert_trigger();

CREATE OR REPLACE FUNCTION payment_after_update_trigger() RETURNS TRIGGER AS $$
DECLARE
	_delta BIGINT;
BEGIN
	SELECT NEW.posts + NEW.logins + NEW.awards - OLD.posts - OLD.logins - OLD.awards INTO _delta;
	UPDATE users SET contrib = contrib + _delta, money = money + _delta WHERE id = NEW.user_id;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS payment_after_update_trigger ON payment;
CREATE TRIGGER payment_after_update_trigger AFTER UPDATE ON payment
	FOR EACH ROW EXECUTE PROCEDURE payment_after_update_trigger();

COMMIT;
