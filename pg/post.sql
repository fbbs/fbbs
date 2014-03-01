BEGIN;

DROP SCHEMA IF EXISTS posts CASCADE;
CREATE SCHEMA posts;

CREATE SEQUENCE posts.post_id_seq;

CREATE OR REPLACE FUNCTION posts.next_id(OUT result BIGINT) AS $$
DECLARE
	seq_id BIGINT;
	now_millis BIGINT;
BEGIN
	SELECT nextval('posts.post_id_seq') % 2048 INTO seq_id;
	SELECT FLOOR(EXTRACT(EPOCH FROM clock_timestamp()) * 1000) INTO now_millis;
	result := (now_millis << 21) + seq_id;
END;
$$ LANGUAGE plpgsql;

CREATE TABLE posts.base (
	id BIGINT NOT NULL DEFAULT posts.next_id(),
	reply_id BIGINT,
	thread_id BIGINT,

	user_id INTEGER,
	real_user_id INTEGER,
	user_name TEXT,
	board_id INTEGER,
	board_name TEXT,

	digest BOOLEAN DEFAULT FALSE,
	marked BOOLEAN DEFAULT FALSE,
	locked BOOLEAN DEFAULT FALSE,
	imported BOOLEAN DEFAULT FALSE,
	water BOOLEAN DEFAULT FALSE,
	attachment BOOLEAN DEFAULT FALSE,
	sticky BOOLEAN DEFAULT FALSE,

	title TEXT
);

CREATE TABLE posts.recent (
) INHERITS (posts.base);
CREATE INDEX ON posts.recent (id);

CREATE TABLE posts.archive (
) INHERITS (posts.base);
CREATE INDEX ON posts.archive (id);

CREATE TABLE posts.content_base (
	post_id BIGINT,
	content TEXT
);

CREATE TABLE posts.content (
) INHERITS (posts.content_base);
CREATE INDEX ON posts.content (post_id);

CREATE TABLE posts.deleted (
	delete_stamp TIMESTAMPTZ,
	eraser_id INTEGER,
	eraser_name TEXT,
	junk BOOLEAN DEFAULT FALSE,
	bm_visible BOOLEAN
) INHERITS (posts.base);

CREATE OR REPLACE FUNCTION posts_recent_before_insert_trigger() RETURNS TRIGGER AS $$
BEGIN
	IF NEW.reply_id = 0 THEN
		NEW.reply_id := NEW.id;
		NEW.thread_id := NEW.id;
	END IF;
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS posts_recent_before_insert_trigger ON posts.recent;
CREATE TRIGGER posts_recent_before_insert_trigger BEFORE INSERT ON posts.recent
	FOR EACH ROW EXECUTE PROCEDURE posts_recent_before_insert_trigger();

COMMIT;
