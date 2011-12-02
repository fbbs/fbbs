BEGIN;

CREATE TABLE all_users (
	id SERIAL PRIMARY KEY,
	name TEXT,
	passwd TEXT,
	alive BOOLEAN DEFAULT TRUE,
	money BIGINT DEFAULT 0,
	paid_posts INTEGER DEFAULT 0,
	followings INTEGER DEFAULT 0,
	followers INTEGER DEFAULT 0,
	friends	INTEGER DEFAULT 0
);

CREATE VIEW users AS
	SELECT * FROM all_users WHERE alive = TRUE;

CREATE UNIQUE INDEX user_name_idx ON all_users (lower(name)) WHERE alive = TRUE;

CREATE TABLE follows (
	user_id INTEGER NOT NULL REFERENCES all_users,
	follower INTEGER NOT NULL REFERENCES all_users,
	follow_time TIMESTAMPTZ,
	notes TEXT,
	is_friend BOOLEAN DEFAULT FALSE,
	PRIMARY KEY (user_id, follower)
);

COMMIT;
