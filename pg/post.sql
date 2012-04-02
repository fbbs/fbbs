CREATE TABLE posts_base (
	id BIGSERIAL,
	reid BIGINT,
	tid BIGINT,
	owner INTEGER,
	stamp TIMESTAMPTZ,
	board INTEGER,

	digest BOOLEAN,
	marked BOOLEAN,
	locked BOOLEAN,
	imported BOOLEAN,
	sticky BOOLEAN,

	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,

	title TEXT,
	content TEXT
);

CREATE TABLE posts (
) INHERITS (posts_base);

CREATE TABLE posts_deleted (
	eraser INTEGER,
	deleted TIMESTAMPTZ,
	junk BOOLEAN DEFAULT FALSE,
	scope SMALLINT
) INHERITS (posts_base);

CREATE TABLE threads (
	id BIGINT,
	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0
);
