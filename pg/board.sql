BEGIN;

DROP TABLE IF EXISTS bms;
DROP TABLE IF EXISTS boards;
DROP TABLE IF EXISTS board_categs;
DROP TABLE IF EXISTS board_sectors;

CREATE TABLE board_sectors (
	id SERIAL PRIMARY KEY,
	name CHAR,
	descr TEXT,
	short_descr TEXT
);
INSERT INTO board_sectors (id, name) VALUES (0, ' ');

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
CREATE UNIQUE INDEX ON boards (lower(name));
CREATE TABLE bms (
	user_id INTEGER REFERENCES users,
	board_id INTEGER REFERENCES boards,
	UNIQUE (user_id, board_id)
);

COMMIT;
