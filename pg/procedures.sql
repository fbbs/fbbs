CREATE OR REPLACE FUNCTION follow_before_trigger() RETURNS TRIGGER AS $$
DECLARE
	_followings all_users.followings%TYPE;
BEGIN
	SELECT followings INTO _followings FROM all_users WHERE id = NEW.follower;
	IF _followings >= 300 THEN
		RAISE 'following limit (300) reached';
	END IF;

	PERFORM * FROM follows WHERE user_id = NEW.follower AND follower = NEW.user_id AND is_friend = FALSE;
	IF FOUND THEN
		UPDATE follows SET is_friend = TRUE WHERE user_id = NEW.follower AND follower = NEW.user_id;
		UPDATE all_users SET friends = friends + 1 WHERE id = NEW.user_id OR id = NEW.follower;
		NEW.is_friend := TRUE;
	END IF;
	NEW.follow_time := current_timestamp;
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS follow_before_trigger ON follows;
CREATE TRIGGER follow_before_trigger BEFORE INSERT ON follows
	FOR EACH ROW EXECUTE PROCEDURE follow_before_trigger();

CREATE OR REPLACE FUNCTION follow_after_trigger() RETURNS TRIGGER AS $$
BEGIN
	UPDATE all_users SET followings = followings + 1 WHERE id = NEW.follower;
	UPDATE all_users SET followers = followers + 1 WHERE id = NEW.user_id;
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS follow_after_trigger ON follows;
CREATE TRIGGER follow_after_trigger AFTER INSERT ON follows
	FOR EACH ROW EXECUTE PROCEDURE follow_after_trigger();

CREATE OR REPLACE FUNCTION unfollow_trigger() RETURNS TRIGGER AS $$
BEGIN
	UPDATE all_users SET followers = followers - 1 WHERE id = OLD.user_id;
	UPDATE all_users SET followings = followings - 1 WHERE id = OLD.follower;
	PERFORM * FROM follows WHERE user_id = OLD.follower AND follower = OLD.user_id;
	IF FOUND THEN
		UPDATE follows SET is_friend = FALSE WHERE user_id = OLD.follower AND follower = OLD.user_id;
		UPDATE all_users SET friends = friends - 1 WHERE id = OLD.user_id OR id = OLD.follower;
	END IF;
	RETURN OLD;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS unfollow_trigger ON follows;
CREATE TRIGGER unfollow_trigger AFTER DELETE ON follows
	FOR EACH ROW EXECUTE PROCEDURE unfollow_trigger();
