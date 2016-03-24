-- Function: pgm_hello_world(cstring)

DROP FUNCTION pgm_hello_world(CSTRING);

CREATE OR REPLACE FUNCTION pgm_hello_world(entrada CSTRING)
  RETURNS CSTRING AS
'$libdir/pgminer.so', 'pgm_hello_world'
  LANGUAGE c STRICT;

-- Deprecated
-- -- Function: pgm_create_matrix(integer, integer)
-- 
-- DROP FUNCTION pgm_create_matrix(integer, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_create_matrix(n_lines integer, n_cols integer)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_create_matrix'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_create_vetor_double(integer)
-- 
--  DROP FUNCTION pgm_create_vetor_double(integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_create_vetor_double(n_elem integer)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_create_vetor_double'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_create_vetor_int(integer)
-- 
-- DROP FUNCTION pgm_create_vetor_int(integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_create_vetor_int(n_elems integer)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_create_vetor_int'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_free_matrix(bigint)
-- 
-- DROP FUNCTION pgm_free_matrix(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_free_matrix(matrix bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_free_matrix'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_free_vetor_double(bigint)
-- 
-- DROP FUNCTION pgm_free_vetor_double(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_free_vetor_double(vetor bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_free_vetor_double'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_free_vetor_int(bigint)
-- 
-- DROP FUNCTION pgm_free_vetor_int(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_free_vetor_int(vetor bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_free_vetor_int'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_get_matrix_elem(bigint, integer, integer)
-- 
-- DROP FUNCTION pgm_get_matrix_elem(bigint, integer, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_matrix_elem(matrix bigint, line integer, col integer)
--   RETURNS double precision AS
-- '$libdir/pgminer.so', 'pgm_get_matrix_elem'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_get_matrix_line(bigint, integer)
-- 
-- DROP FUNCTION pgm_get_matrix_line(bigint, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_matrix_line(matrix bigint, line integer)
--   RETURNS double precision[] AS
-- '$libdir/pgminer.so', 'pgm_get_matrix_line'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_get_vetor_double_elem(bigint, integer)
-- 
-- DROP FUNCTION pgm_get_vetor_double_elem(bigint, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_vetor_double_elem(vetor bigint, posicao integer)
--   RETURNS double precision AS
-- '$libdir/pgminer.so', 'pgm_get_vetor_double_elem'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_get_vetor_int_elem(bigint, integer)
-- 
-- DROP FUNCTION pgm_get_vetor_int_elem(bigint, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_vetor_int_elem(vetor bigint, posicao integer)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_get_vetor_int_elem'
--   LANGUAGE c VOLATILE STRICT;

-- Function: pgm_kmeans(bigint, integer, integer, integer)

-- Deprecated
-- -- Function: pgm_ncols_matrix(bigint)
-- 
-- DROP FUNCTION pgm_ncols_matrix(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_ncols_matrix(matrix bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_ncols_matrix'
--   LANGUAGE c VOLATILE STRICT;

-- Deprecated
-- -- Function: pgm_nelem_vetor_double(bigint)
-- 
-- DROP FUNCTION pgm_nelem_vetor_double(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_nelem_vetor_double(vetor bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_nelem_vetor_double'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_nelem_vetor_int(bigint)
-- 
-- DROP FUNCTION pgm_nelem_vetor_int(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_nelem_vetor_int(vetor bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_nelem_vetor_int'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_nlines_matrix(bigint)
-- 
-- DROP FUNCTION pgm_nlines_matrix(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_nlines_matrix(matrix bigint)
--   RETURNS integer AS
-- '$libdir/pgminer.so', 'pgm_nlines_matrix'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_set_matrix_elem(bigint, integer, integer, double precision)
-- 
-- DROP FUNCTION pgm_set_matrix_elem(bigint, integer, integer, double precision);
-- 
-- CREATE OR REPLACE FUNCTION pgm_set_matrix_elem(matrix bigint, line integer, col integer, "value" double precision)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_set_matrix_elem'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_set_matrix_line(bigint, integer, double precision[])
-- 
-- DROP FUNCTION pgm_set_matrix_line(bigint, integer, double precision[]);
-- 
-- CREATE OR REPLACE FUNCTION pgm_set_matrix_line(matrix bigint, line integer, valor double precision[])
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_set_matrix_line'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_set_vetor_double_elem(bigint, integer, double precision)
-- 
-- DROP FUNCTION pgm_set_vetor_double_elem(bigint, integer, double precision);
-- 
-- CREATE OR REPLACE FUNCTION pgm_set_vetor_double_elem(vetor bigint, posicao integer, valor double precision)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_set_vetor_double_elem'
--   LANGUAGE c VOLATILE STRICT;
-- 
-- Deprecated
-- -- Function: pgm_set_vetor_int_elem(bigint, integer, integer)
-- 
-- DROP FUNCTION pgm_set_vetor_int_elem(bigint, integer, integer);
-- 
-- CREATE OR REPLACE FUNCTION pgm_set_vetor_int_elem(vetor bigint, posicao integer, valor integer)
--   RETURNS bigint AS
-- '$libdir/pgminer.so', 'pgm_set_vetor_int_elem'
--   LANGUAGE c VOLATILE STRICT;


-- Deprecated
-- -- Function: pgm_get_matrix(bigint)
-- 
-- DROP FUNCTION pgm_get_matrix(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_matrix(vetor bigint)
--   RETURNS float8[][] AS
-- '$libdir/pgminer.so', 'pgm_get_matrix'
--   LANGUAGE c VOLATILE STRICT;
-- 

-- Deprecated
-- -- Function: pgm_get_vector_int(bigint)
-- 
-- DROP FUNCTION pgm_get_vector_int(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_vector_int(vetor bigint)
--   RETURNS integer[] AS
-- '$libdir/pgminer.so', 'pgm_get_vector_int'
--   LANGUAGE c VOLATILE STRICT;
-- 

-- Deprecated
-- -- Function: pgm_get_vector_double(bigint)
-- 
-- DROP FUNCTION pgm_get_vector_double(bigint);
-- 
-- CREATE OR REPLACE FUNCTION pgm_get_vector_double(vetor bigint)
--   RETURNS float8[] AS
-- '$libdir/pgminer.so', 'pgm_get_vector_double'
--   LANGUAGE c VOLATILE STRICT;


DROP FUNCTION pgm_kmeans(bigint, integer, integer, integer);

CREATE OR REPLACE FUNCTION pgm_kmeans(IN pontos bigint, IN k integer, IN seed integer, IN fim integer, OUT centroide bigint, OUT grupo bigint, OUT sumd double precision)
  RETURNS record AS
'$libdir/pgminer.so', 'pgm_kmeans'
  LANGUAGE c IMMUTABLE STRICT;
