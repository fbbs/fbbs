BEGIN;

CREATE TABLE emails (
	id SERIAL PRIMARY KEY,
	addr TEXT
);
CREATE UNIQUE INDEX ON emails(addr);

CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	email INTEGER REFERENCES emails,
	alive BOOLEAN DEFAULT TRUE,
	money BIGINT DEFAULT 0,
	rank REAL DEFAULT 0,
	paid_posts INTEGER DEFAULT 0,
	title TEXT,
);
CREATE OR REPLACE VIEW alive_users AS
	SELECT * FROM users WHERE alive = TRUE;
CREATE UNIQUE INDEX ON users (lower(name)) WHERE alive = TRUE;

CREATE TABLE board_sectors (
	id SERIAL PRIMARY KEY,
	name CHAR,
	descr TEXT,
	short_descr TEXT
);
CREATE TABLE board_categs (
	id SERIAL PRIMARY KEY,
	name TEXT
);
CREATE TABLE boards (
	id SERIAL PRIMARY KEY,
	name TEXT,
	descr TEXT,
	parent INTEGER,
	flag INTEGER,
	perm INTEGER,
	categ INTEGER REFERENCES board_categs,
	sector INTEGER REFERENCES board_sectors,
	bms TEXT
);
CREATE UNIQUE INDEX ON boards (lower(bms));
CREATE TABLE bms (
	user_id INTEGER REFERENCES users,
	board_id INTEGER REFERENCES boards,
	UNIQUE (user_id, board_id)
);

CREATE TABLE fav_board_folders (
	id SERIAL PRIMARY KEY,
	user_id INTEGER,
	name TEXT,
	descr TEXT
);
INSERT INTO fav_board_folders (name) VALUES ('ROOT');
CREATE TABLE fav_boards (
	user_id INTEGER REFERENCES users,
	board INTEGER REFERENCES boards,
	folder INTEGER REFERENCES fav_board_folders
);
CREATE UNIQUE INDEX ON fav_boards (user_id, board);

CREATE TABLE prop_categs (
	id SERIAL PRIMARY KEY,
	name TEXT
);
CREATE TABLE prop_items (
	id SERIAL PRIMARY KEY,
	categ INTEGER REFERENCES prop_categs,
	name TEXT,
	price INTEGER,
	expire INTERVAL,
	valid BOOLEAN DEFAULT TRUE
);
CREATE TABLE prop_records (
	id SERIAL PRIMARY KEY,
	user_id INTEGER REFERENCES users,
	item INTEGER REFERENCES prop_items,
	price INTEGER,
	order_time TIMESTAMPTZ,
	expire TIMESTAMPTZ
);

CREATE TABLE titles (
	id SERIAL PRIMARY KEY,
	user_id INTEGER REFERENCES users,
	granter INTEGER REFERENCES users,
	title TEXT NOT NULL,
	approved BOOLEAN DEFAULT FALSE,
	record_id INTEGER REFERENCES prop_records ON DELETE CASCADE
);

CREATE SCHEMA audit;

CREATE TABLE audit.money (
	user_id INTEGER NOT NULL,
	delta INTEGER NOT NULL,
	stamp TIMESTAMPTZ NOT NULL,
	reason TEXT
);

COMMIT;
