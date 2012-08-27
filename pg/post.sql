BEGIN;

DROP TABLE IF EXISTS threads;
DROP TABLE IF EXISTS posts_sticked;
DROP TABLE IF EXISTS posts_deleted;
DROP TABLE IF EXISTS posts;
DROP TABLE IF EXISTS posts_base;

CREATE TABLE posts_base (
	id BIGSERIAL,
	reid BIGINT,
	tid BIGINT,
	owner INTEGER,
	stamp TIMESTAMPTZ,
	board INTEGER,

	digest BOOLEAN,
	marked BOOLEAN,
	locked BOOLEAN,
	imported BOOLEAN,

	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,
	score INTEGER DEFAULT 0,

	uname TEXT,
	title TEXT,
	content TEXT
);

CREATE TABLE posts (
	sticky BOOLEAN,
	PRIMARY KEY (id)
) INHERITS (posts_base);

CREATE INDEX ON posts(id) WHERE sticky;
CREATE INDEX ON posts(board);

CREATE TABLE posts_deleted (
	eraser INTEGER,
	deleted TIMESTAMPTZ,
	junk BOOLEAN DEFAULT FALSE,
	bm_visible BOOLEAN
) INHERITS (posts_base);

CREATE TABLE threads (
	id BIGINT,
	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0
);

COMMIT;
