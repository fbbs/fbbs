BEGIN;

DROP TABLE IF EXISTS posts.threads;
DROP TABLE IF EXISTS posts.deleted;
DROP TABLE IF EXISTS posts.recent;
DROP TABLE IF EXISTS posts.base;
DROP SCHEMA IF EXISTS posts;

CREATE SCHEMA posts;

CREATE TABLE posts.base (
	id BIGSERIAL,
	reid BIGINT,
	tid BIGINT,
	owner INTEGER,
	stamp TIMESTAMPTZ,
	board INTEGER,

	digest BOOLEAN DEFAULT FALSE,
	marked BOOLEAN DEFAULT FALSE,
	locked BOOLEAN DEFAULT FALSE,
	imported BOOLEAN DEFAULT FALSE,
	water BOOLEAN DEFAULT FALSE,

	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,
	score INTEGER DEFAULT 0,

	uname TEXT,
	title TEXT,
	content TEXT
);

CREATE TABLE posts.recent (
	sticky BOOLEAN DEFAULT FALSE,
	PRIMARY KEY (id)
) INHERITS (posts.base);

CREATE INDEX ON posts.recent(id) WHERE sticky;
CREATE INDEX ON posts.recent(board);

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
	id BIGINT,
	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0
);

COMMIT;
