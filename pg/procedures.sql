CREATE OR REPLACE FUNCTION user_delete_trigger() RETURNS TRIGGER AS $user_delete_trigger$
BEGIN
	INSERT INTO all_users VALUES (OLD.*);
	RETURN OLD;
END;
$user_delete_trigger$ LANGUAGE plpgsql;

CREATE TRIGGER user_delete_trigger
BEFORE DELETE ON users
	FOR EACH ROW EXECUTE PROCEDURE user_delete_trigger();
