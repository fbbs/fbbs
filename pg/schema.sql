BEGIN;

CREATE TABLE emails (
	id SERIAL PRIMARY KEY,
	addr TEXT
);
CREATE UNIQUE INDEX ON emails(addr);
INSERT INTO emails (addr) VALUES (NULL);

CREATE TABLE users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	email INTEGER REFERENCES emails,
	alive BOOLEAN DEFAULT TRUE,
	money BIGINT DEFAULT 0,
	rank REAL DEFAULT 0,
	paid_posts INTEGER DEFAULT 0,
	title TEXT
);

CREATE OR REPLACE VIEW alive_users AS
	SELECT * FROM users WHERE alive = TRUE;

CREATE UNIQUE INDEX ON users (lower(name)) WHERE alive = TRUE;

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
