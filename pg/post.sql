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

	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0,
	score INTEGER DEFAULT 0,

	title TEXT,
	content TEXT
);

CREATE TABLE posts (
) INHERITS (posts_base);

CREATE TABLE posts_deleted (
	eraser INTEGER,
	deleted TIMESTAMPTZ,
	junk BOOLEAN DEFAULT FALSE,
	bm_visible BOOLEAN
) INHERITS (posts_base);

CREATE TABLE posts_sticked (
	pid BIGINT REFERECES posts,
	stamp TIMESTAMP
);

CREATE TABLE threads (
	id BIGINT,
	replies INTEGER DEFAULT 0,
	comments INTEGER DEFAULT 0
);
