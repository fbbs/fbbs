BEGIN;

CREATE TABLE emails (
	id SERIAL PRIMARY KEY,
	addr TEXT
);

CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	nick TEXT,
	email INTEGER,
	options BIGINT,
	level INTEGER,
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

CREATE TABLE groups (
	id SERIAL PRIMARY KEY,
	name TEXT,
	description TEXT
);

CREATE TABLE roles (
	user_id INTEGER,
	group_id INTEGER,
	grant_type INTEGER
);
CREATE INDEX roles_user_id_idx ON roles (user_id);
-- unique index?

CREATE TABLE sectors (
	id SERIAL PRIMARY KEY,
	name TEXT
);

CREATE TABLE boards (
	id SERIAL PRIMARY KEY,
	name TEXT,
	description TEXT,
	category TEXT,
	sector INTEGER,
	parent INTEGER,
	property INTEGER
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
	property INTEGER, --?
	filename TEXT,
	time TIMESTAMPTZ
);

CREATE TABLE sessions (
	id BIGSERIAL PRIMARY KEY,
	user_id INTEGER,
	key BIGINTEGER,
	property INTEGER,
	start_time TIMESTAMPTZ,
	expire_time TIMESTAMPTZ
);

COMMIT;
