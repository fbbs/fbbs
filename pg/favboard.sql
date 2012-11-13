BEGIN;

DROP TABLE IF EXISTS fav_boards;
DROP TABLE IF EXISTS fav_board_folders;

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
	folder INTEGER REFERENCES fav_board_folders,
	name TEXT -- cached
);
CREATE UNIQUE INDEX ON fav_boards (user_id, board);

COMMIT;
