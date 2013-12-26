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

CREATE TABLE boards (
	id SERIAL PRIMARY KEY,
	name TEXT,
	descr TEXT,
	parent INTEGER,
	flag INTEGER,
	perm INTEGER,
	categ TEXT,
	sector INTEGER REFERENCES board_sectors,
	bms TEXT
);
CREATE UNIQUE INDEX ON boards (lower(name));
CREATE TABLE bms (
	user_id INTEGER REFERENCES users,
	board_id INTEGER REFERENCES boards,
	stamp TIMESTAMPTZ,
	UNIQUE (user_id, board_id)
);

CREATE OR REPLACE FUNCTION bms_trigger() RETURNS TRIGGER AS $$
DECLARE
	_row RECORD;
BEGIN
	IF (TG_OP = 'DELETE') THEN
		_row = OLD;
	ELSE
		_row = NEW;
	END IF;
	UPDATE boards SET bms =
		(SELECT string_agg(u.name, ' ' ORDER BY b.stamp)
			FROM bms b JOIN users u ON b.user_id = u.id WHERE b.board_id = _row.board_id)
		WHERE id = _row.board_id;
	RETURN NULL;
END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS bms_trigger ON bms;
CREATE TRIGGER bms_trigger AFTER INSERT OR UPDATE OR DELETE ON bms
	FOR EACH ROW EXECUTE PROCEDURE bms_trigger();

COMMIT;
