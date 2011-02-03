BEGIN;

CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	nick TEXT,
	email TEXT,
	flag BIGINT,
	perm INTEGER,
	logins INTEGER DEFAULT 0,
	posts INTEGER DEFAULT 0,
	stay INTEGER DEFAULT 0,
	medals INTEGER DEFAULT 0,
	money INTEGER DEFAULT 0,
	birth TIMESTAMPTZ,
	gender CHAR,
	creation TIMESTAMPTZ,
	lastlogin TIMESTAMPTZ,
	lastlogout TIMESTAMPTZ,
	lasthost TEXT
);
CREATE UNIQUE INDEX user_name_idx ON users (lower(name));

CREATE TABLE sectors (
	id SERIAL PRIMARY KEY,
	name TEXT
);

CREATE TABLE boards (
	id SERIAL PRIMARY KEY,
	name TEXT,
	description TEXT,
	category TEXT,
	sector SMALLINT,
	parent INTEGER,
	flag INTEGER
);

CREATE TABLE managers (
	user_id INTEGER,
	board_id INTEGER,
	appointed TIMESTAMPTZ
);

CREATE TABLE posts (
	id BIGSERIAL PRIMARY KEY,
	reid BIGINT,
	gid BIGINT,
	board_id INTEGER,
	user_id INTEGER,
	user_name TEXT, -- for compatability
	title TEXT,
	flag INTEGER, --?
	time TIMESTAMPTZ,
	t2 TIMESTAMPTZ,
	itype SMALLINT
);

COMMIT;
