BEGIN;

CREATE TABLE all_users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT
);

CREATE TABLE users () INHERITS (all_users);
CREATE UNIQUE INDEX user_name_idx ON users (lower(name));

COMMIT;
