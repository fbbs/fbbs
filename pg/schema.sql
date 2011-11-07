BEGIN;

CREATE TABLE all_users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	alive BOOLEAN DEFAULT TRUE,
	money BIGINT DEFAULT 0
);

CREATE VIEW users AS
	SELECT * FROM all_users WHERE alive = TRUE;

CREATE UNIQUE INDEX user_name_idx ON all_users (lower(name)) WHERE alive = TRUE;

COMMIT;
