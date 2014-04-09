BEGIN;

DROP SCHEMA IF EXISTS post CASCADE;
CREATE SCHEMA post;

CREATE SEQUENCE post.post_id_seq;

CREATE OR REPLACE FUNCTION post.next_id(OUT result BIGINT) AS $$
DECLARE
	seq_id BIGINT;
	now_millis BIGINT;
BEGIN
	SELECT nextval('post.post_id_seq') % 2048 INTO seq_id;
	SELECT FLOOR(EXTRACT(EPOCH FROM clock_timestamp()) * 1000) INTO now_millis;
	result := (now_millis << 21) + seq_id;
END;
$$ LANGUAGE plpgsql;

CREATE TABLE post.base (
	id BIGINT NOT NULL DEFAULT post.next_id(),
	reply_id BIGINT,
	thread_id BIGINT,

	user_id INTEGER,
	user_id_replied INTEGER,
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

CREATE TABLE post.recent (
) INHERITS (post.base);
CREATE INDEX ON post.recent (id);

CREATE TABLE post.archive (
) INHERITS (post.base);
CREATE INDEX ON post.archive (id);

CREATE TABLE post.content_base (
	post_id BIGINT,
	content TEXT
);

CREATE TABLE post.content (
) INHERITS (post.content_base);
CREATE INDEX ON post.content (post_id);

CREATE TABLE post.deleted (
	delete_stamp TIMESTAMPTZ,
	eraser_id INTEGER,
	eraser_name TEXT,
	junk BOOLEAN DEFAULT FALSE,
	bm_visible BOOLEAN
) INHERITS (post.base);

CREATE TABLE post.reply_base (
	post_id BIGINT,
	reply_id BIGINT,
	thread_id BIGINT,

	user_id_replied INTEGER,
	user_id INTEGER,
	user_name TEXT,
	board_id INTEGER,
	board_name TEXT,

	title TEXT
);

CREATE TABLE post.reply_0 (
) INHERITS (post.reply_base);
CREATE INDEX ON post.reply_0 (post_id);
CREATE INDEX ON post.reply_0 (user_id_replied);

CREATE TABLE post.mention_0 (
) INHERITS (post.reply_base);
CREATE INDEX ON post.mention_0 (post_id);
CREATE INDEX ON post.mention_0 (user_id_replied);

CREATE OR REPLACE FUNCTION post_recent_before_insert_trigger() RETURNS TRIGGER AS $$
BEGIN
	IF NEW.reply_id = 0 THEN
		NEW.reply_id := NEW.id;
		NEW.thread_id := NEW.id;
	END IF;
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS post_recent_before_insert_trigger ON post.recent;
CREATE TRIGGER post_recent_before_insert_trigger BEFORE INSERT ON post.recent
	FOR EACH ROW EXECUTE PROCEDURE post_recent_before_insert_trigger();

COMMIT;
