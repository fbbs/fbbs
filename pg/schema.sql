BEGIN;

CREATE TABLE domains (
	id SERIAL PRIMARY KEY,
	trust BOOLEAN NOT NULL DEFAULT FALSE,
	domain TEXT UNIQUE NOT NULL,
	description TEXT
);
INSERT INTO domains (id, trust, domain, description)
		VALUES (DEFAULT, FALSE, '', 'unknown domain');

CREATE TABLE emails (
	id SERIAL PRIMARY KEY,
	domain INTEGER,
	name TEXT,
	UNIQUE (name, domain)
);
INSERT INTO emails (id, domain, name)
		VALUES (DEFAULT, 1, '');

CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	nick TEXT,
	email INTEGER,
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
INSERT INTO users (id, name, email)
		VALUES (DEFAULT, 'SYSOP', 1);

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

CREATE TABLE catagories (
	id SERIAL PRIMARY KEY,
	name TEXT
);
CREATE TABLE sectors (
	id SERIAL PRIMARY KEY,
	name TEXT
);

CREATE TABLE boards (
	id SERIAL PRIMARY KEY,
	name TEXT,
	description TEXT,
	category INTEGER,
	sector INTEGER,
	parent INTEGER,
	property INTEGER
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
	filename TEXT
);

COMMIT;
