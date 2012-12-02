BEGIN;

DROP SCHEMA IF EXISTS posts CASCADE;
CREATE SCHEMA posts;

CREATE TABLE posts.base (
	id BIGSERIAL,
	reid BIGINT,
	tid BIGINT,
	fake_id INTEGER,
	owner INTEGER,
	board INTEGER,
	stamp TIMESTAMPTZ,
	last_timestamp TIMESTAMPTZ,

	digest BOOLEAN DEFAULT FALSE,
	marked BOOLEAN DEFAULT FALSE,
	locked BOOLEAN DEFAULT FALSE,
	imported BOOLEAN DEFAULT FALSE,
	water BOOLEAN DEFAULT FALSE,
	attachment BOOLEAN DEFAULT FALSE,

	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,
	score INTEGER DEFAULT 0,

	uname TEXT,
	title TEXT,
	content TEXT
);

CREATE TABLE posts.archives (
) INHERITS (posts.base);

CREATE TABLE posts.recent (
	junk BOOLEAN,
	sticky BOOLEAN DEFAULT FALSE
) INHERITS (posts.base);

CREATE TABLE posts.deleted (
	did BIGSERIAL,
	eraser INTEGER,
	deleted TIMESTAMPTZ,
	junk BOOLEAN DEFAULT FALSE,
	bm_visible BOOLEAN,
	ename TEXT
) INHERITS (posts.base);
CREATE INDEX ON posts.deleted(did);

CREATE TABLE posts.threads (
	tid BIGINT,
	board INTEGER,
	stamp TIMESTAMPTZ,
	last_id BIGINT DEFAULT 0,
	last_stamp TIMESTAMPTZ,
	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,
	owner INTEGER,
	uname TEXT,
	title TEXT
);

CREATE TABLE posts.board_archive (
	board INTEGER,
	archive INTEGER,
	min BIGINT,
	max BIGINT
);
CREATE INDEX ON posts.board_archive(board);

CREATE TABLE posts.hot (
	tid BIGINT,
	score INTEGER,
	board INTEGER,
	last_stamp TIMESTAMPTZ,
	owner INTEGER,
	uname TEXT,
	bname TEXT,
	title TEXT
);

CREATE OR REPLACE FUNCTION posts_archives_before_insert_trigger() RETURNS trigger AS $$
BEGIN
	EXECUTE 'INSERT INTO posts.archive_' || NEW.board || ' VALUES ($1.*)' USING NEW;
	RETURN NULL;
END; $$ LANGUAGE plpgsql;

CREATE TRIGGER posts_archives_before_insert_trigger
	BEFORE INSERT ON posts.archives
	FOR EACH ROW EXECUTE PROCEDURE posts_archives_before_insert_trigger();

CREATE OR REPLACE FUNCTION posts_recent_before_insert_trigger() RETURNS trigger AS $$
BEGIN
	EXECUTE 'INSERT INTO posts.recent_' || NEW.board || ' VALUES ($1.*)' USING NEW;
	RETURN NULL;
END; $$ LANGUAGE plpgsql;

CREATE TRIGGER posts_recent_before_insert_trigger
	BEFORE INSERT ON posts.recent
	FOR EACH ROW EXECUTE PROCEDURE posts_recent_before_insert_trigger();

CREATE OR REPLACE FUNCTION create_posts_recent_partition(_board INTEGER) RETURNS VOID AS $$
BEGIN
	EXECUTE 'CREATE TABLE posts.recent_' || _board || ' (CHECK (board = ' || _board || ')) INHERITS (posts.recent)';
	EXECUTE 'CREATE INDEX ON posts.recent_' || _board || ' (id)';
	EXECUTE 'CREATE INDEX ON posts.recent_' || _board || ' (board)';
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION create_posts_archive_partition(_board INTEGER) RETURNS VOID AS $$
BEGIN
	EXECUTE 'CREATE TABLE posts.archive_' || _board || ' (CHECK (board = ' || _board || ')) INHERITS (posts.archives)';
	EXECUTE 'CREATE INDEX ON posts.archive_' || _board || ' (id)';
	EXECUTE 'CREATE INDEX ON posts.archive_' || _board || ' (board)';
END;
$$ LANGUAGE plpgsql;

COMMIT;
