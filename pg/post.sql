BEGIN;

DROP SCHEMA IF EXISTS posts CASCADE;
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
	attachment BOOLEAN DEFAULT FALSE,

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

COMMIT;
