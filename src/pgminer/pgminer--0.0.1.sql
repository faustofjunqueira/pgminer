CREATE TYPE fann AS (
  ann bigint
);


--
-- TOC entry 856 (class 1247 OID 28040)
-- Dependencies: 8 190
-- Name: matrix; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE matrix AS (
  ptr bigint
);


--
-- TOC entry 930 (class 1247 OID 95771)
-- Dependencies: 8 212
-- Name: memelmcos; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE memelmcos AS (
  pesos_a bigint,
  pesos_b bigint
);


--
-- TOC entry 859 (class 1247 OID 28043)
-- Dependencies: 8 191
-- Name: memnn; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE memnn AS (
  pesos_a bigint,
  pesos_b bigint,
  aplica_sigmoid boolean
);


--
-- TOC entry 863 (class 0 OID 0)
-- Name: neuralnet; Type: SHELL TYPE; Schema: public; Owner: -
--

CREATE TYPE neuralnet;


--
-- TOC entry 288 (class 1255 OID 28045)
-- Dependencies: 8 863
-- Name: pgm_neuralnetin(cstring); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_neuralnetin(st cstring) RETURNS neuralnet
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_NeuralNetIn';


--
-- TOC entry 289 (class 1255 OID 28046)
-- Dependencies: 8 863
-- Name: pgm_neuralnetout(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_neuralnetout(neuralnet) RETURNS cstring
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_NeuralNetOut';


--
-- TOC entry 862 (class 1247 OID 28044)
-- Dependencies: 289 8 288
-- Name: neuralnet; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE neuralnet (
    INTERNALLENGTH = variable,
    INPUT = pgm_neuralnetin,
    OUTPUT = pgm_neuralnetout,
    ALIGNMENT = int4,
    STORAGE = plain
);


--
-- TOC entry 866 (class 1247 OID 28050)
-- Dependencies: 8 192
-- Name: svd_record; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE svd_record AS (
  ut bigint,
  s bigint,
  vt bigint
);


--
-- TOC entry 1013 (class 1247 OID 282934)
-- Dependencies: 8 234
-- Name: traindata; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE traindata AS (
  entrada matrix,
  saida matrix,
  ptr bigint
);


--
-- TOC entry 869 (class 1247 OID 28053)
-- Dependencies: 8 193
-- Name: vector; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE vector AS (
  ptr bigint
);



--
-- TOC entry 302 (class 1255 OID 28064)
-- Dependencies: 27 859 1097
-- Name: create_elm2(text, integer, text, double precision); Type: FUNCTION; Schema: draft; Owner: -
--

CREATE FUNCTION create_elm2(table_data text, neurons integer, test_fold text DEFAULT '1'::text, lambda double precision DEFAULT (10000)::numeric, OUT nn public.memnn, OUT mse_train double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  entrada_treinamento matrix;
  saida_treinamento matrix;
  entrada_teste matrix;
  saida_teste matrix;
  pesos_a matrix;
  pesos_b matrix;
  meio matrix;
  pinv_meio matrix;
  saida matrix;
  previsao_treinamento matrix;
  previsao_teste matrix;
  erro_treinamento matrix;
  erro_teste matrix;
BEGIN
  entrada_treinamento := matrix( 'select id, entrada valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  saida_treinamento := matrix( 'select id, saida valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  entrada_teste := matrix( 'select id, entrada valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  saida_teste := matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  pesos_a := matrix( make_random_matrix(  ncols( entrada_treinamento ), neurons, -1, 1 ) );
  meio := entrada_treinamento * pesos_a;
  PERFORM apply_sigmoid( meio );

  pinv_meio := pinv_reg( meio, lambda ); 

  PERFORM apply_inverse_sigmoid( saida_treinamento ); 
  pesos_b := pinv_meio * saida_treinamento;
  PERFORM apply_sigmoid( saida_treinamento );

  nn := memnn( pesos_a, pesos_b, true ); 
  
  previsao_treinamento := entrada_treinamento * nn;
  previsao_teste := entrada_teste * nn;

  erro_treinamento := previsao_treinamento - saida_treinamento;
  erro_teste := previsao_teste - saida_teste;

  mse_train := norm( erro_treinamento )/pgm_matrix_nlines( saida_treinamento.ptr );
  mse_test := norm( erro_teste )/pgm_matrix_nlines( saida_teste.ptr );

  misclassified_train := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_treinamento, saida_treinamento) ) ) v where v < (sqrt(2.0)/2.0)::float8);
  misclassified_test := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_teste, saida_teste) ) ) v where v < (sqrt(2.0)/2.0)::float8);


  PERFORM free( meio );
  PERFORM free( saida_teste );
  PERFORM free( entrada_teste );
  PERFORM free( saida_treinamento );
  PERFORM free( entrada_treinamento );
  PERFORM free( erro_teste );
  PERFORM free( erro_treinamento );
  PERFORM free( previsao_teste );
  PERFORM free( previsao_treinamento );
  PERFORM free( saida );
  PERFORM free( pinv_meio );
END;
$$;


--
-- TOC entry 301 (class 1255 OID 28065)
-- Dependencies: 27 1097 859
-- Name: create_elm3(text, integer, integer, text, double precision); Type: FUNCTION; Schema: draft; Owner: -
--

CREATE FUNCTION create_elm3(table_data text, neurons integer, iterations integer DEFAULT 1, test_fold text DEFAULT '1'::text, lambda double precision DEFAULT (10000)::numeric, OUT nn public.memnn, OUT mse_train double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS SETOF record
    LANGUAGE plpgsql COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  pinv_entrada matrix;
  entrada_treinamento matrix;
  saida_treinamento matrix;
  saida_treinamento_siginv matrix;
  saida_treinamento_corrigida matrix;

  entrada_teste matrix;
  saida_teste matrix;
  pesos_a matrix;
  pesos_b matrix;
  meio matrix;
  meio_corrigido matrix;

  previsao_treinamento matrix;
  previsao_teste matrix;
  erro_treinamento matrix;
  erro_teste matrix;

  i integer;
  peso float8;
BEGIN
  entrada_treinamento := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  pinv_entrada := pinv( entrada_treinamento );
  entrada_teste := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold in (' || test_fold || ')' );

  saida_treinamento := matrix( 'select id, saida valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  saida_teste := matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ')' );

  saida_treinamento_siginv := apply_inverse_sigmoid( dup( saida_treinamento ) );
  pesos_a := matrix( make_random_matrix( ncols( entrada_treinamento ), neurons, -1, 1 ) );

  peso := max( 0.01, min( 50.0/lambda, 0.3 ) );
  raise info 'Peso %', peso;

  for i in 1..iterations loop
    meio := apply_sigmoid( entrada_treinamento * pesos_a );
    pesos_b := pinv_reg( meio, lambda ) * saida_treinamento_siginv;

    nn := memnn( pesos_a, pesos_b, true ); 
  
    previsao_treinamento := entrada_treinamento * nn;
    previsao_teste := entrada_teste * nn;

    erro_treinamento := previsao_treinamento - saida_treinamento;
    erro_teste := previsao_teste - saida_teste;

    mse_train := norm( erro_treinamento )/nlines( saida_treinamento );
    mse_test := norm( erro_teste )/nlines( saida_teste );

    misclassified_train := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_treinamento, saida_treinamento) ) ) v where v < (sqrt(2.0)/2.0)::float8);
    misclassified_test := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_teste, saida_teste) ) ) v where v < (sqrt(2.0)/2.0)::float8);

    PERFORM free( erro_teste );
    PERFORM free( erro_treinamento );
    
    return next;
    
    saida_treinamento_corrigida := saida_treinamento_siginv*peso + apply_inverse_sigmoid( previsao_treinamento )*(1.0::float8-peso);

    meio_corrigido := (saida_treinamento_corrigida * pinv( pesos_b ))*peso + meio*(1.0::float8-peso);
    pesos_a = pinv_entrada * apply_inverse_sigmoid( meio_corrigido );

    PERFORM free( meio );
    PERFORM free( previsao_teste );
    PERFORM free( previsao_treinamento );
  END loop;

  PERFORM free( saida_teste );
  PERFORM free( entrada_teste );
  PERFORM free( saida_treinamento );
  PERFORM free( entrada_treinamento );
  
  return;
END;
$$;


--
-- TOC entry 494 (class 1255 OID 114338)
-- Dependencies: 27 1097 859
-- Name: create_elm4(text, integer, text, double precision, boolean); Type: FUNCTION; Schema: draft; Owner: -
--

CREATE FUNCTION create_elm4(table_data text, neurons integer, test_fold text, lambda double precision, keep_middle boolean, OUT nn public.memnn, OUT mse_train double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  entrada_treinamento matrix;
  saida_treinamento matrix;
  entrada_teste matrix;
  saida_teste matrix;
  pesos_a matrix;
  pesos_b matrix;
  meio matrix;
  pinv_meio matrix;
  saida matrix;
  previsao_treinamento matrix;
  previsao_teste matrix;
  erro_treinamento matrix;
  erro_teste matrix;
BEGIN
  entrada_treinamento := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  saida_treinamento := matrix( 'select id, saida valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  entrada_teste := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  saida_teste := matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  pesos_a := matrix( make_random_matrix(  ncols( entrada_treinamento ), neurons, -1, 1 ) );
  meio := entrada_treinamento * pesos_a;
  perform apply_sigmoid( meio );
--  pinv_meio := pinv( meio ); 
  pinv_meio := pinv_reg( meio, lambda ); 
  pesos_b := pinv_meio * saida_treinamento;

  nn := memnn( pesos_a, pesos_b );
  
  previsao_treinamento := entrada_treinamento * nn;
  previsao_teste := entrada_teste * nn;

  perform entrada_treinamento ==> 'etreino';
  perform entrada_teste ==> 'eteste';
  perform previsao_treinamento ==> 'ptreino';
  perform previsao_teste ==> 'pteste';

  erro_treinamento := previsao_treinamento - saida_treinamento;
  erro_teste := previsao_teste - saida_teste;

  mse_train := norm( erro_treinamento )/pgm_matrix_nlines( saida_treinamento.ptr );
  mse_test := norm( erro_teste )/pgm_matrix_nlines( saida_teste.ptr );

  misclassified_train := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_treinamento, saida_treinamento) ) ) v where v < (sqrt(2.0)/2.0)::float8);
  misclassified_test := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_teste, saida_teste) ) ) v where v < (sqrt(2.0)/2.0)::float8);

  if keep_middle then
    perform meio ==> (table_data || '_meio');
  end if;
  
  PERFORM free( meio );
  PERFORM free( saida_teste );
  PERFORM free( entrada_teste );
  PERFORM free( saida_treinamento );
  PERFORM free( entrada_treinamento );
  PERFORM free( erro_teste );
  PERFORM free( erro_treinamento );
  PERFORM free( previsao_teste );
  PERFORM free( previsao_treinamento );
  PERFORM free( saida );
  PERFORM free( pinv_meio );
END;
$$;


--
-- TOC entry 513 (class 1255 OID 103776)
-- Dependencies: 930 1097 27
-- Name: create_elm_cos(text, integer, text, double precision, boolean); Type: FUNCTION; Schema: draft; Owner: -
--

CREATE FUNCTION create_elm_cos(table_data text, neurons integer, test_fold text DEFAULT '1'::text, lambda double precision DEFAULT 10000, keep_middle boolean DEFAULT false, OUT elmcos public.memelmcos, OUT mse_train double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  entrada_treinamento matrix;
  saida_treinamento matrix;
  entrada_teste matrix;
  saida_teste matrix;
  pesos_a matrix;
  pesos_b matrix;
  meio matrix;
  pinv_meio matrix;
  saida matrix;
  previsao_treinamento matrix;
  previsao_teste matrix;
  erro_treinamento matrix;
  erro_teste matrix;
BEGIN
  entrada_treinamento := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  saida_treinamento := matrix( 'select id, saida valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  entrada_teste := matrix( 'select id, entrada || 1::float8 valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  saida_teste := matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  pesos_a := matrix( make_random_matrix(  ncols( entrada_treinamento ), neurons, -1, 1 ) );
  meio := lsh_cos( entrada_treinamento, pesos_a ) + entrada_treinamento*pesos_a;

  pinv_meio := pinv_reg( meio, lambda ); 
  pesos_b := pinv_meio * saida_treinamento;

  elmcos := memelmcos( pesos_a, pesos_b );
  
  previsao_treinamento := entrada_treinamento * elmcos;
  previsao_teste := entrada_teste * elmcos;

  erro_treinamento := previsao_treinamento - saida_treinamento;
  erro_teste := previsao_teste - saida_teste;

  mse_train := norm( erro_treinamento )/pgm_matrix_nlines( saida_treinamento.ptr );
  mse_test := norm( erro_teste )/pgm_matrix_nlines( saida_teste.ptr );

  misclassified_train := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_treinamento, saida_treinamento) ) ) v where v < (sqrt(2.0)/2.0)::float8);
  misclassified_test := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_teste, saida_teste) ) ) v where v < (sqrt(2.0)/2.0)::float8);

  if keep_middle then
    perform meio ==> (table_data || '_meio');
  end if;
  
  PERFORM free( meio );
  PERFORM free( saida_teste );
  PERFORM free( entrada_teste );
  PERFORM free( saida_treinamento );
  PERFORM free( entrada_treinamento );
  PERFORM free( erro_teste );
  PERFORM free( erro_treinamento );
  PERFORM free( previsao_teste );
  PERFORM free( previsao_treinamento );
  PERFORM free( saida );
  PERFORM free( pinv_meio );
END;
$$;


SET search_path = public, pg_catalog;

--
-- TOC entry 290 (class 1255 OID 28054)
-- Dependencies: 8 1097
-- Name: analyze_dense_matrix(text, text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION analyze_dense_matrix(sql text, base_name text, options text DEFAULT 'C'::text, OUT min double precision, OUT max double precision, OUT avg double precision, OUT stddev_samp double precision, OUT stddev_pop double precision, OUT count bigint, OUT distinct_values bigint) RETURNS record
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE
  LINHA FLOAT8[];
  i INTEGER;
BEGIN
  -- Dada um SQL do tipo 'select id, array[ x, y ] valor from tabela', calcula as estatisticas  
  -- supondo uma matriz densa e coloca em nome_base_line_st, nome_base_col_st e nome_base_matrix_st.
  -- Retorna o conteudo de nome_base_st.
  -- Options: 'LCM' ==> Line, Col, Matrix

  IF STRPOS( options, 'M' ) > 0 THEN
    PERFORM cleanup_table( base_name || '_matrix_st', '( ID SERIAL NOT NULL PRIMARY KEY, MIN FLOAT8, MAX FLOAT8, AVG FLOAT8, STDDEV_SAMP FLOAT8, STDDEV_POP FLOAT8, COUNT BIGINT, DISTINCT_VALUES BIGINT )' );

    EXECUTE 'INSERT INTO ' || base_name || '_matrix_st( MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT, DISTINCT_VALUES ) 
             SELECT MIN( X ), MAX( X ), AVG( X ), STDDEV_SAMP( X ), STDDEV_POP( X ), COUNT( X ), COUNT( DISTINCT( X ) ) DISTINCT_VALUES 
             FROM (SELECT UNNEST(VALOR) X FROM (' || sql || ') VV) V';

    EXECUTE 'ANALYZE ' || base_name || '_matrix_st';

    EXECUTE 'SELECT MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT, DISTINCT_VALUES FROM ' || base_name || '_matrix_st' INTO MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT, DISTINCT_VALUES;
  END IF;
     
  IF STRPOS( options, 'L' ) > 0 THEN
    PERFORM cleanup_table( base_name || '_line_st', '( ID INTEGER NOT NULL PRIMARY KEY, MIN FLOAT8, MAX FLOAT8, AVG FLOAT8, STDDEV_SAMP FLOAT8, STDDEV_POP FLOAT8, COUNT BIGINT, DISTINCT_VALUES BIGINT )' );

    EXECUTE 'INSERT INTO ' || base_name || '_line_st( ID, MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT, DISTINCT_VALUES ) 
             SELECT ID, MIN( X ), MAX( X ), AVG( X ), STDDEV_SAMP( X ), STDDEV_POP( X ), COUNT( X ), COUNT( DISTINCT( X ) ) DISTINCT_VALUES 
             FROM (SELECT ID, UNNEST(VALOR) X FROM (' || sql || ') VV) V GROUP BY ID ORDER BY ID';

    EXECUTE 'ANALYZE ' || base_name || '_line_st';
  END IF;
    
  IF STRPOS( options, 'C' ) > 0 THEN
    PERFORM cleanup_table( base_name || '_col_st', '( ID INTEGER NOT NULL PRIMARY KEY, MIN FLOAT8, MAX FLOAT8, AVG FLOAT8, STDDEV_SAMP FLOAT8, STDDEV_POP FLOAT8, COUNT BIGINT, DISTINCT_VALUES BIGINT )' );
  
    EXECUTE 'SELECT valor from (' || sql || ') V LIMIT 1' into LINHA;

    FOR i IN SELECT generate_series( 1, array_upper( LINHA, 1 ) ) LOOP
      EXECUTE 'INSERT INTO ' || base_name || '_col_st( ID, MIN, MAX, AVG, STDDEV_SAMP, STDDEV_POP, COUNT, DISTINCT_VALUES ) 
               SELECT ' || I || ', MIN( X ), MAX( X ), AVG( X ), STDDEV_SAMP( X ), STDDEV_POP( X ), COUNT( X ), COUNT( DISTINCT( X ) ) 
               FROM (SELECT VALOR[' || I || '] X FROM (' || SQL || ') VV) V'; 
    END LOOP; 

    EXECUTE 'ANALYZE ' || base_name || '_col_st';
  END IF;
END
$$;


--
-- TOC entry 527 (class 1255 OID 282822)
-- Dependencies: 1097 8
-- Name: analyze_elm(text, double precision, double precision, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION analyze_elm(table_data text, percent_min_neurons double precision DEFAULT 0.1, percent_max_neurons double precision DEFAULT 1.0, steps integer DEFAULT 10, repeticoes integer DEFAULT 10, OUT neurons integer, OUT lambda double precision, OUT fold integer, OUT rmse_train double precision, OUT rmse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS SETOF record
    LANGUAGE plpgsql COST 1000
    AS $$
DECLARE 
  rec record;
  dd record; 
BEGIN
  FOR fold in execute 'select distinct fold from ' || table_data || ' order by fold' Loop
    dd := split_folds( table_data, fold::text );
  
    FOR rec in select * from analyze_elm( dd.entrada_treinamento, dd.saida_treinamento, 
                            dd.entrada_teste, dd.saida_teste, 
                            percent_min_neurons, percent_max_neurons, steps, repeticoes ) loop
      neurons := rec.neurons;
      lambda  := rec.lambda;
      rmse_train := rec.rmse_train;
      rmse_test := rec.rmse_test;
      misclassified_train := rec.misclassified_train;
      misclassified_test := rec.misclassified_test;
      
      RETURN NEXT; 
    end loop;  
  end loop; 
END;
$$;


--
-- TOC entry 525 (class 1255 OID 282819)
-- Dependencies: 1097 856 856 856 856 8
-- Name: analyze_elm(matrix, matrix, matrix, matrix, double precision, double precision, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION analyze_elm(entrada_treinamento matrix, saida_treinamento matrix, entrada_teste matrix, saida_teste matrix, percent_min_neurons double precision DEFAULT 0.1, percent_max_neurons double precision DEFAULT 1.0, steps integer DEFAULT 10, repeticoes integer DEFAULT 10, OUT neurons integer, OUT lambda double precision, OUT rmse_train double precision, OUT rmse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS SETOF record
    LANGUAGE plpgsql COST 1000
    AS $$
DECLARE
  rec record;
  i integer;
  k float8;
  j integer;
  min_neurons integer;
  max_neurons integer;
  step integer;
BEGIN
  IF percent_min_neurons < 1 THEN
    min_neurons := (percent_min_neurons*nlines(entrada_treinamento))::integer;
    max_neurons := (percent_max_neurons*nlines(entrada_treinamento))::integer;
    step := (max_neurons-min_neurons)/steps+1;
  ELSE  
    min_neurons := percent_min_neurons;
    max_neurons := percent_max_neurons;
    step := (max_neurons-min_neurons+1)/steps;
  END IF;

  neurons := min_neurons - step;
  FOR j in select generate_series( 0, steps ) LOOP
    neurons := neurons + step;
    FOR lambda in select generate_series( 1, 12 ) LOOP 
      k := power( 10, lambda );  
      FOR i in select generate_series( 1, repeticoes ) LOOP   
        rec := create_elm( entrada_treinamento, saida_treinamento, neurons, k, entrada_teste, saida_teste ); 
     
        PERFORM free(rec.nn);
        rmse_train := rec.rmse_train;
        rmse_test := rec.rmse_test;
        misclassified_train := rec.misclassified_train;
        misclassified_test := rec.misclassified_test;

        RETURN NEXT; 
      END LOOP;
    END LOOP;
  END LOOP;
END;
$$;


--
-- TOC entry 291 (class 1255 OID 28055)
-- Dependencies: 1097 856 856 8 856
-- Name: apply_inverse_sigmoid(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION apply_inverse_sigmoid(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
-- A propria matriz é alterada, não cria cópia.
BEGIN
  c.ptr := pgm_matrix_apply_inverse_sigmoid( m.ptr );
END;
$$;


--
-- TOC entry 292 (class 1255 OID 28056)
-- Dependencies: 856 856 8 1097 856
-- Name: apply_sigmoid(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION apply_sigmoid(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
-- A propria matriz é alterada, não cria cópia.
BEGIN
  c.ptr := pgm_matrix_apply_sigmoid( m.ptr );
END;
$$;


--
-- TOC entry 293 (class 1255 OID 28057)
-- Dependencies: 8
-- Name: blas_multiply(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION blas_multiply(a bigint, b bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'blas_multiply';


--
-- TOC entry 294 (class 1255 OID 28058)
-- Dependencies: 8 1097
-- Name: classe2vector(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION classe2vector(sql_distinct_classes text, OUT classe text, OUT vector double precision[]) RETURNS SETOF record
    LANGUAGE plpgsql STABLE COST 1
    AS $$
-- A query sql_distinct_classes deve ter ordenação.
-- EX: select * from classe2vector( 'select distinct classe from iris order by classe' )
DECLARE
  n_dimensions INTEGER;
BEGIN
  EXECUTE 'SELECT COUNT(*) FROM (' || sql_distinct_classes || ') A' INTO n_dimensions;    
  
  RETURN QUERY EXECUTE 'select classe::text, unitary_vector( cast( row_number() over() as integer ), ' || n_dimensions ||') vector from (' || sql_distinct_classes ||') v';
END;
$$;


--
-- TOC entry 296 (class 1255 OID 28059)
-- Dependencies: 1097 8
-- Name: classe2vector(text, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION classe2vector(sql_distinct_classes text, precisao double precision, OUT classe text, OUT vector double precision[]) RETURNS SETOF record
    LANGUAGE plpgsql STABLE COST 1
    AS $$
-- A query sql_distinct_classes deve ter ordenação.
-- EX: select * from classe2vector( 'select distinct classe from iris order by classe' )
DECLARE
  n_dimensions INTEGER;
BEGIN
  EXECUTE 'SELECT COUNT(*) FROM (' || sql_distinct_classes || ') A' INTO n_dimensions;    
  
  RETURN QUERY EXECUTE 'select classe::text, unitary_vector( cast( row_number() over() as integer ), ' || n_dimensions || ', ' || precisao || ') vector from (' || sql_distinct_classes ||') v';
END;
$$;


--
-- TOC entry 297 (class 1255 OID 28060)
-- Dependencies: 8 1097
-- Name: cleanup_table(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION cleanup_table(table_name text, table_columns text, make_temp boolean DEFAULT true) RETURNS integer
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  -- Zera ou cria uma tabela
  -- Ex: PERFORM cleanup_table( 'centroides_iris', '( GRUPO INTEGER PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );
  BEGIN
    EXECUTE 'DELETE FROM ' || table_name;

    EXCEPTION WHEN UNDEFINED_TABLE THEN
      IF make_temp THEN
        EXECUTE 'CREATE TEMPORARY TABLE ' || table_name || table_columns;
      ELSE
        EXECUTE 'CREATE TABLE ' || table_name || table_columns;
      END IF;
  END;

  RETURN 0;
END
$$;


--
-- TOC entry 299 (class 1255 OID 28062)
-- Dependencies: 8 1097 856 856 869 869
-- Name: cos(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION cos(a matrix, b matrix, OUT c vector) RETURNS vector
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_cos( a.ptr, b.ptr );
END;
$$;


--
-- TOC entry 505 (class 1255 OID 123239)
-- Dependencies: 8
-- Name: cos(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION cos(a double precision[], b double precision[]) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', '_cos';


--
-- TOC entry 516 (class 1255 OID 282793)
-- Dependencies: 1097 859 8
-- Name: create_elm(text, integer, text, double precision, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION create_elm(table_data text, neurons integer, test_fold text DEFAULT '1'::text, lambda double precision DEFAULT (10000)::numeric, repeticoes integer DEFAULT 10, OUT nn memnn, OUT rmse_train double precision, OUT rmse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS SETOF record
    LANGUAGE plpgsql COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  rec record;
  entrada_trn matrix;
  saida_trn matrix;
  entrada_tst matrix;
  saida_tst matrix;
  i integer;
BEGIN
  rec := split_folds( table_data, test_fold );
  entrada_trn  := rec.entrada_treinamento;
  saida_trn    := rec.saida_treinamento;
  entrada_tst  := rec.entrada_teste;
  saida_tst    := rec.saida_teste;

  FOR i in select generate_series( 1, repeticoes ) LOOP   
    rec := create_elm( entrada_trn, saida_trn, neurons, lambda, entrada_tst, saida_tst ); 
     
    nn := rec.nn;
    rmse_train := rec.rmse_train;
    rmse_test := rec.rmse_test;
    misclassified_train := rec.misclassified_train;
    misclassified_test := rec.misclassified_test;

    RETURN NEXT; 
  END LOOP;
END;
$$;


--
-- TOC entry 517 (class 1255 OID 282705)
-- Dependencies: 856 856 859 856 856 1097 8 856
-- Name: create_elm(matrix, matrix, integer, double precision, matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION create_elm(entrada_treinamento matrix, saida_treinamento matrix, neurons integer, lambda double precision, entrada_teste matrix DEFAULT NULL::matrix, saida_teste matrix DEFAULT NULL::matrix, OUT nn memnn, OUT rmse_train double precision, OUT rmse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$
DECLARE
  pesos_a matrix;
  pesos_b matrix;
  meio matrix;
  pinv_meio matrix;
  previsao_treinamento matrix;
  previsao_teste matrix;
BEGIN
  pesos_a := matrix( make_random_matrix(  ncols( entrada_treinamento ), neurons, -1, 1 ) );
  meio := entrada_treinamento * pesos_a;
  perform apply_sigmoid( meio );
  pinv_meio := pinv_reg( meio, lambda ); 
  pesos_b := pinv_meio * saida_treinamento;

  nn := memnn( pesos_a, pesos_b );
  
  previsao_treinamento := entrada_treinamento * nn;
  rmse_train := distancia( previsao_treinamento, saida_treinamento );
  
  IF NOT (entrada_teste is null)  and NOT (saida_teste is null) THEN
    previsao_teste := entrada_teste * nn;    
    rmse_test := distancia( previsao_teste, saida_teste );
  END IF;
  
  IF ncols( saida_treinamento ) > 1 THEN
    misclassified_train := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_treinamento, saida_treinamento) ) ) v where v < (sqrt(2.0)/2.0)::float8);

    IF not ( previsao_teste IS null ) THEN
      misclassified_test := (select count(*) from unnest(pgm_vector_double2array( cos( previsao_teste, saida_teste) ) ) v where v < (sqrt(2.0)/2.0)::float8);
    END IF;
  END IF;
  
  PERFORM free( meio );
  PERFORM free( previsao_teste );
  PERFORM free( previsao_treinamento );
  PERFORM free( pinv_meio );
END;
$$;


--
-- TOC entry 541 (class 1255 OID 282938)
-- Dependencies: 1010 8 1097
-- Name: create_fann(text, integer[], text, text, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION create_fann(table_data text, layers integer[], validation_fold text DEFAULT '1'::text, test_fold text DEFAULT '1'::text, max_epochs integer DEFAULT 500, epochs_between_report integer DEFAULT 10, OUT net fann, OUT mse_train double precision, OUT mse_validation double precision, OUT mse_test double precision, OUT misclassified_train integer, OUT misclassified_validation integer, OUT misclassified_test integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  train traindata;
  validation traindata;
  test traindata;
  i integer;
BEGIN
  train := fann_create_train_data( matrix( 'select id, entrada valor from ' || table_data || ' where not (fold in (' || validation_fold || ', ' || test_fold || ' ))' ),
                                   matrix( 'select id, saida valor from ' || table_data || ' where not (fold in (' || validation_fold || ', ' || test_fold || ' ))' ) );
  
  validation := fann_create_train_data( matrix( 'select id, entrada valor from ' || table_data || ' where fold in ( ' || validation_fold || ' ) ' ),
                                        matrix( 'select id, saida valor from ' || table_data || ' where fold  in( ' || validation_fold || ' ) ' ) );

  test := fann_create_train_data( matrix( 'select id, entrada valor from ' || table_data || ' where fold in (' || test_fold || ') ' ),
                                  matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ') ' ) );

  IF layers is null or layers = array[]::integer[] THEN
    net.ANN := pgm_nn_fann_train( train.ptr, 0, 0, 0.00001 );
  else
    net.ANN := pgm_nn_fann_train( train.ptr, layers, 0, 0, 0.00001 );
  end if;
  
  i := 0;
  WHILE i <= max_epochs LOOP
    PERFORM pgm_nn_fann_train( train.ptr, net.ANN, epochs_between_report, 0, 0.00001 );
    i := i + epochs_between_report;
    
    mse_train := pgm_nn_fann_test( train.ptr, net.ANN );
    mse_validation := pgm_nn_fann_test( validation.ptr, net.ANN );
    mse_test := pgm_nn_fann_test( test.ptr, net.ANN );

    IF ncols( train.saida ) > 1 THEN 
      SELECT count(*) INTO misclassified_train 
        FROM generate_series( 0, nlines(train.entrada)-1 ) k
        WHERE cos( get_line( train.entrada, k ) * net, get_line( train.saida, k ) ) < sqrt(2.0)/2.0;

      SELECT count(*) INTO misclassified_test 
        FROM generate_series( 0, nlines(test.entrada)-1 ) k
        WHERE cos( get_line( test.entrada, k ) * net, get_line( test.saida, k ) ) < sqrt(2.0)/2.0;

      SELECT count(*) INTO misclassified_validation 
        FROM generate_series( 0, nlines(validation.entrada)-1 ) k
        WHERE cos( get_line( validation.entrada, k ) * net, get_line( validation.saida, k ) ) < sqrt(2.0)/2.0;
    END IF;
    

    RETURN NEXT; 
  END LOOP;

  PERFORM pgm_nn_fann_free_train_data( test.ptr );
  PERFORM pgm_nn_fann_free_train_data( validation.ptr );
  PERFORM pgm_nn_fann_free_train_data( train.ptr );

   RETURN;
END;
$$;

--
-- TOC entry 304 (class 1255 OID 28068)
-- Dependencies: 862 1097 8
-- Name: create_nn(text, integer[], integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION create_nn(table_data text, hidden integer[], functionActivation integer DEFAULT 3, steepness float default 0.01, validation_fold integer default 1, test_fold integer default 1, max_epochs integer default 500, epochs_between_report integer default 10, OUT nn neuralnet, OUT mse_report double precision[]) RETURNS record
    LANGUAGE plpgsql STABLE COST 1000
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  FANN bigint;
  trainData bigint;
  validationData bigint;
  testData bigint;
BEGIN
  trainData := pgm_nn_fann_create_train_data( 
                (make_matrix( 'select id, entrada valor from ' || table_data || ' where not (fold in (' || validation_fold || ', ' || test_fold || ' ))' )).matrix,
                (make_matrix( 'select id, saida valor from ' || table_data || ' where not (fold in (' || validation_fold || ', ' || test_fold || ' ))' )).matrix );
  
  validationData := pgm_nn_fann_create_train_data(
                      (make_matrix( 'select id, entrada valor from ' || table_data || ' where fold = ' || validation_fold || ' ' )).matrix,
                      (make_matrix( 'select id, saida valor from ' || table_data || ' where fold  = ' || validation_fold || ' ' )).matrix );

  testData := pgm_nn_fann_create_train_data(
                (make_matrix( 'select id, entrada valor from ' || table_data || ' where fold = ' || test_fold || ' ' )).matrix,
                (make_matrix( 'select id, saida valor from ' || table_data || ' where fold = ' || test_fold || ' ' )).matrix );

  FANN := pgm_nn_fann_train( trainData, hidden, functionActivation, steepness, max_epochs, epochs_between_report, 0.00001 );

  PERFORM pgm_nn_fann_train( trainData, FANN, max_epochs, epochs_between_report, 0.00001 );

  mse_report := ARRAY[ 0.0 ];

  NN := pgm_nn_fann2neuralnet( FANN );

  RAISE INFO 'MSE: % / % / %', pgm_nn_fann_test( trainData, FANN ), pgm_nn_fann_test( validationData, FANN ), pgm_nn_fann_test( testData, FANN );

  PERFORM pgm_nn_fann_free_train_data( testData );
  PERFORM pgm_nn_fann_free_train_data( validationData );
  PERFORM pgm_nn_fann_free_train_data( trainData );
END;
$$;
  

--
-- TOC entry 305 (class 1255 OID 28069)
-- Dependencies: 856 8 1097 856 869
-- Name: diag(vector); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION diag(v vector, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql STABLE COST 10
    AS $$
DECLARE
  -- Cria e retorna uma matrix diagonal [a] do tamanho de v.
  i integer;
  size integer;
BEGIN
  size := pgm_vector_double_nlines( v.ptr );
  c.ptr := pgm_matrix_create( size, size );

  FOR i IN 0..size-1 LOOP
    PERFORM pgm_matrix_set_elem( c.ptr, i, i, pgm_vector_double_get_elem( v.ptr, i ) );
  END LOOP;
END;
$$;


--
-- TOC entry 306 (class 1255 OID 28070)
-- Dependencies: 856 8 1097 856
-- Name: diag(integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION diag(size integer, a double precision DEFAULT 1.0, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql STABLE COST 10
    AS $$
DECLARE
  -- Cria e retorna uma matrix diagonal [a] de tamanho size.
  i integer;
BEGIN
  c.ptr := pgm_matrix_create( size, size );

  FOR i IN 0..size-1 LOOP
    PERFORM pgm_matrix_set_elem( c.ptr, i, i, a );
  END LOOP;
END;
$$;


--
-- TOC entry 307 (class 1255 OID 28071)
-- Dependencies: 1097 8 856
-- Name: dims(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dims(m matrix, OUT lines integer, OUT cols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
BEGIN
  lines := pgm_matrix_nlines( m.ptr );
  cols := pgm_matrix_ncols( m.ptr );
END;
$$;


--
-- TOC entry 447 (class 1255 OID 282703)
-- Dependencies: 856 1097 8 856
-- Name: distancia(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION distancia(a matrix, b matrix, OUT valor double precision) RETURNS double precision
    LANGUAGE plpgsql COST 1
    AS $$
DECLARE
  c matrix;
BEGIN
  c := a - b;
  valor = norm( c )/nlines( c );

  perform free( c );
END;
$$;


--
-- TOC entry 308 (class 1255 OID 28072)
-- Dependencies: 8 856 856 856 1097
-- Name: dup(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION dup(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_copy( m.ptr );
END;
$$;


--
-- TOC entry 530 (class 1255 OID 282794)
-- Dependencies: 859 8 1097
-- Name: elm_find_best(text, integer, text, double precision, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION elm_find_best(table_data text, neurons integer, test_fold text DEFAULT '1'::text, lambda double precision DEFAULT (10000)::numeric, repeticoes integer DEFAULT 10, OUT nn memnn, OUT rmse_train double precision, OUT rmse_test double precision, OUT misclassified_train integer, OUT misclassified_test integer) RETURNS record
    LANGUAGE plpgsql COST 10
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
  rec record;
  entrada_trn matrix;
  saida_trn matrix;
  entrada_tst matrix;
  saida_tst matrix;
  i integer;
BEGIN
  rec := split_folds( table_data, test_fold );
  entrada_trn  := rec.entrada_treinamento;
  saida_trn    := rec.saida_treinamento;
  entrada_tst  := rec.entrada_teste;
  saida_tst    := rec.saida_teste;

  FOR i in select generate_series( 1, repeticoes ) LOOP   
    rec := create_elm( entrada_trn, saida_trn, neurons, lambda, entrada_tst, saida_tst ); 

    IF nn IS NULL OR 
       (rec.misclassified_train IS NULL AND rec.rmse_train < rmse_train) OR 
       rec.misclassified_train < misclassified_train THEN   
      nn := rec.nn;
      rmse_train := rec.rmse_train;
      rmse_test := rec.rmse_test;
      misclassified_train := rec.misclassified_train;
      misclassified_test := rec.misclassified_test;
    ELSE
      PERFORM FREE( rec.nn );
    END IF;
  END LOOP;

  RETURN;
END;
$$;


--
-- TOC entry 496 (class 1255 OID 282927)
-- Dependencies: 8 1010 1010 1097
-- Name: fann(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION fann(ann bigint, OUT c fann) RETURNS fann
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  c.ann := ann;
END;
$$;


--
-- TOC entry 514 (class 1255 OID 282935)
-- Dependencies: 856 8 1097 1013 856 1013
-- Name: fann_create_train_data(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION fann_create_train_data(entrada matrix, saida matrix, OUT r traindata) RETURNS traindata
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  r.entrada := entrada;
  r.saida := saida;
  r.ptr = pgm_nn_fann_create_train_data( entrada.ptr, saida.ptr );
END;
$$;


--
-- TOC entry 471 (class 1255 OID 28073)
-- Dependencies: 1097 856 8
-- Name: free(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION free(m matrix) RETURNS void
    LANGUAGE plpgsql STRICT
    AS $$
BEGIN
  IF not (m is null) AND not (m.ptr is null) THEN
    PERFORM pgm_matrix_free( m.ptr );
  END IF;  
END;
$$;


--
-- TOC entry 495 (class 1255 OID 282706)
-- Dependencies: 859 8 1097
-- Name: free(memnn); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION free(m memnn) RETURNS void
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  IF not (m is null) THEN
    PERFORM free( matrix( m.pesos_a ) );
    PERFORM free( matrix( m.pesos_b ) );
  END IF;  
END;
$$;


--
-- TOC entry 506 (class 1255 OID 213339)
-- Dependencies: 8 856 1097
-- Name: get_col(matrix, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION get_col(m matrix, col integer) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  return pgm_matrix_get_col( m.ptr, col );
END
$$;


--
-- TOC entry 497 (class 1255 OID 213338)
-- Dependencies: 856 8 1097
-- Name: get_line(matrix, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION get_line(m matrix, line integer) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  return pgm_matrix_get_line( m.ptr, line );
END
$$;


--
-- TOC entry 309 (class 1255 OID 28074)
-- Dependencies: 1097 8
-- Name: histogram(text, text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION histogram(coluna text, sql_in text, nbins integer, OUT bin integer, OUT count bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
declare
   a float8;
   b float8;
   sql text;
BEGIN
  execute 'select min('||coluna||'), max('||coluna||') from ' || table_or_query( sql_in ) into a, b;
  sql := 'select width_bucket( ' || coluna || ', ' || a || ', ' || b || ', ' || nbins || ' ) bin from ' || table_or_query( sql_in );
  return query execute 'select bin, count(*) count from ( ' || sql || ' ) vv group by bin order by bin'; 
END;
$$;


--
-- TOC entry 310 (class 1255 OID 28075)
-- Dependencies: 1097 856 856 8 856
-- Name: inv(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION inv(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_inverse( m.ptr );
END;
$$;


--
-- TOC entry 311 (class 1255 OID 28076)
-- Dependencies: 8 1097
-- Name: inverse_sigmoid(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION inverse_sigmoid(a double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( ln( x/(1.0::float8 - x) )/2.0::float8 ) from (select min( 0.999999999999, max( x, 0.000000000001 ) ) x from unnest( a ) x ) c;
END
$$;


--
-- TOC entry 300 (class 1255 OID 28077)
-- Dependencies: 1097 8
-- Name: inverse_sigmoid(double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION inverse_sigmoid(a double precision) RETURNS double precision
    LANGUAGE plpgsql STRICT COST 1
    AS $$
declare
  x float8;
BEGIN
  x := min( 0.999999999999, max( a, 0.000000000001 ) );
  RETURN ln( x/(1.0::float8 - x) )/2.0::float8;
END
$$;


--
-- TOC entry 542 (class 1255 OID 322677)
-- Dependencies: 8 1097
-- Name: lista_mapa(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION lista_mapa(mapa text, OUT v double precision[]) RETURNS SETOF double precision[]
    LANGUAGE plpgsql STABLE COST 10
    AS $$
-- mapa deve ser uma tabela com as colunas x, y e classe. Suporta 4 classes.
DECLARE
  curs0 refcursor;  
  curs1 refcursor;  
  curs2 refcursor;  
  curs3 refcursor;  
  vazio float8[];
  xy float8[];
BEGIN
  open curs0 for execute 'select array[x, y] from ' || mapa || ' where classe = 0';   
  open curs1 for execute 'select array[x, y] from ' || mapa || ' where classe = 1';   
  open curs2 for execute 'select array[x, y] from ' || mapa || ' where classe = 2';   
  open curs3 for execute 'select array[x, y] from ' || mapa || ' where classe = 3';
  vazio := array[null,null,null,null,null,null,null,null]::float8[];   

  LOOP
    V := vazio;
    FETCH FROM curs0 INTO xy;
    IF xy is not NULL then
      v[1] := xy[1];
      v[2] := xy[2];
    END IF;
    
    FETCH FROM curs1 INTO xy;
    IF xy is not NULL then
      v[3] := xy[1];
      v[4] := xy[2];
    END IF;

    FETCH FROM curs2 INTO xy;
    IF xy is not NULL then
      v[5] := xy[1];
      v[6] := xy[2];
    END IF;

    FETCH FROM curs3 INTO xy;
    IF xy is not NULL then
      v[7] := xy[1];
      v[8] := xy[2];
    END IF;
    
    RETURN NExT;
    IF v = vazio THEN
      RETURN;
    END IF;
  END LOOP;
  
END;
$$;


--
-- TOC entry 364 (class 1255 OID 189008)
-- Dependencies: 8 1097
-- Name: logistic_cdf(double precision, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION logistic_cdf(x double precision, mean double precision, stddev double precision DEFAULT (1.0)::double precision) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN 1::float8/(1::float8 + exp( -((x-mean)/stddev) ) );
END
$$;


--
-- TOC entry 446 (class 1255 OID 189011)
-- Dependencies: 8 1097
-- Name: logistic_cdf_kde(double precision, double precision[], double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION logistic_cdf_kde(x double precision, p double precision[], h double precision) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN avg(logistic_cdf( x, p_i, h )) from unnest( p ) p_i;
END
$$;


--
-- TOC entry 371 (class 1255 OID 189009)
-- Dependencies: 1097 8
-- Name: logistic_pdf(double precision, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION logistic_pdf(x double precision, mean double precision, stddev double precision DEFAULT (1.0)::double precision) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
DECLARE
  t float8;
BEGIN
  t := exp( -((x-mean)/stddev) );
  RETURN t/(stddev * (1::float8 + t)*(1::float8 + t));
END
$$;


--
-- TOC entry 445 (class 1255 OID 189012)
-- Dependencies: 8 1097
-- Name: logistic_pdf_kde(double precision, double precision[], double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION logistic_pdf_kde(x double precision, p double precision[], h double precision) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN avg(logistic_pdf( x, p_i, h ))/h from unnest( p ) p_i;
END
$$;


--
-- TOC entry 313 (class 1255 OID 28078)
-- Dependencies: 856 856 1097 8 856
-- Name: lpinv(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION lpinv(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_lpinv( m.ptr );
END;
$$;


--
-- TOC entry 489 (class 1255 OID 95727)
-- Dependencies: 856 856 856 856 8 1097
-- Name: lsh_cos(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION lsh_cos(a matrix, b matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
BEGIN
  c.ptr := pgm_lsh_cos( a.ptr, b.ptr );
END
$$;


--
-- TOC entry 510 (class 1255 OID 104808)
-- Dependencies: 1097 8 856
-- Name: lsh_cos(double precision[], matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION lsh_cos(a double precision[], b matrix, OUT c double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
DECLARE
  aa matrix;
  cc matrix;
BEGIN
   aa := matrix( array[ a ] );
   cc := lsh_cos( aa, b );

   c := pgm_matrix_get_line( cc.ptr, 0  );

   perform free( aa );
   perform free( cc );
END
$$;


--
-- TOC entry 314 (class 1255 OID 28079)
-- Dependencies: 1097 8
-- Name: make_dense_matrix(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION make_dense_matrix(sql text, zero text DEFAULT ''::text, centrar boolean DEFAULT true) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- Cria uma matiz densa a partir de um SQL de uma matriz esparsa do tipo 'select linha, coluna, valor from ...'
  -- zero: '' significa que zero é zero; não haverá substituição;
  --       'mavg' significa substituir zero pela média dos valores da matriz inteira;
  --       'lavg' significa substituir os zeros de cada linha pela média de cada linha;
  --       'cavg' significa substituir os zeros de cada coluna pela média de cada coluna;
  --       centrar: centrar a média em zero, o que equivale a subtrair de cada valor 
  --                diferente de zero a média escolhida.
  matriz BIGINT;
  n_linhas INTEGER;
  n_colunas INTEGER;
  rec RECORD;
  source text;
  media FLOAT8[];
  media_geral FLOAT8;
  i integer;
BEGIN
  source := table_or_query( sql );
  EXECUTE 'SELECT MAX(linha), max(coluna) FROM ' || source INTO n_linhas, n_colunas;

  matriz := pgm_matrix_create( n_linhas, n_colunas );

  if zero = '' then 
    FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
       perform pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor );
    END LOOP;
  elsif centrar then 
    if upper(zero) = 'MAVG' then
      EXECUTE 'SELECT AVG(valor) FROM ' || source INTO media_geral;

      FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
         perform pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor - media_geral );
      END LOOP;
    elsif upper(zero) = 'LAVG' then
      EXECUTE 'SELECT ARRAY( SELECT AVG(valor) FROM ' || source || ' GROUP BY linha ORDER BY linha )' INTO media;

      FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
         perform pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor - media[rec.linha] );
      END LOOP;
    elsif upper(zero) = 'CAVG' then
      EXECUTE 'SELECT ARRAY( SELECT AVG(valor) FROM ' || source || ' GROUP BY coluna ORDER BY coluna )' INTO media;

      FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
         perform pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor - media[rec.coluna] );
      END LOOP;
    end if;
  else -- Nao centrar
    if upper(zero) = 'MAVG' then
      EXECUTE 'SELECT AVG(valor) FROM ' || source INTO media_geral;

      perform pgm_matrix_set_value( matriz, media_geral );

      FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
        PERFORM pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor );
      END LOOP;
    elsif upper(zero) = 'LAVG' then
      EXECUTE 'SELECT ARRAY( SELECT AVG(valor) FROM ' || source || ' GROUP BY linha ORDER BY linha )' INTO media;

      for i in 1 .. n_linhas loop
        PERFORM pgm_matrix_set_line_value( matriz, i-1, media[i] );
      end loop;

      FOR rec IN EXECUTE 'SELECT linha, coluna, valor FROM ' || source LOOP 
         perform pgm_matrix_set_elem( matriz, rec.linha-1, rec.coluna-1, rec.valor );
      END LOOP;

    else
      raise exception 'Nao implementado ainda';  
    end if;  
  end if;
  
  RETURN matriz;
END
$$;


--
-- TOC entry 543 (class 1255 OID 372255)
-- Dependencies: 1097 8
-- Name: make_matrix(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION make_matrix(sql text, OUT matrix bigint, OUT keys bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto,
  -- ou uma tabela/view com as colunas id::integer, valor::float8[]
  -- EX1: 'select id, array[ x, y ] valor from tabela'
  -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
  -- EX3: 'svd_iris_vt'
  -- retorna:
  -- matrix: PGM_Matriz_Double*, contendo a matriz;
  -- keys: PGM_Vetor_Int*, contendo a chave (id) de cada linha da matriz.
  -- OBS: Ordena a query por ID.

  rec RECORD;
  i INTEGER;
  n_linhas INTEGER;
  n_colunas INTEGER;
  tabela TEXT;
BEGIN
  if position( 'FROM ' in upper( sql ) ) > 0 THEN
    tabela := ' ( ' || sql || ' ) AAA ';   
  else
    tabela := sql;
  end if;
  
  EXECUTE 'SELECT id, valor::float8[] FROM ' || tabela || ' LIMIT 1' INTO rec;
  n_colunas := array_upper( rec.valor, 1 );
  EXECUTE 'SELECT COUNT(*) FROM ' || tabela INTO n_linhas; 

  matrix := pgm_matrix_create( n_linhas, n_colunas );
  keys := pgm_vector_int_create( n_linhas );
  i := 0;

  FOR rec IN EXECUTE 'SELECT id, valor::float8[] FROM ' || tabela || ' ORDER BY ID' LOOP
    PERFORM pgm_vector_int_set_elem( keys, i, rec.id );
    PERFORM pgm_matrix_set_line( matrix, i, rec.valor );
    i := i + 1;
  END LOOP;
END;
$$;


--
-- TOC entry 507 (class 1255 OID 123238)
-- Dependencies: 8
-- Name: make_random_matrix(integer, integer, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION make_random_matrix(n_lines integer, n_cols integer, a double precision DEFAULT 0.0, b double precision DEFAULT 1.0) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'make_random_matrix';


--
-- TOC entry 519 (class 1255 OID 301707)
-- Dependencies: 1097 8
-- Name: make_token_table(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION make_token_table(sql text, base_name text) RETURNS text
    LANGUAGE plpgsql STRICT
    AS $$
-- Dada um SQL do tipo 'select msg from tabela', cria a tabela de tokens
BEGIN
        BEGIN EXECUTE 'DROP TABLE '||base_name||'_dict'; EXCEPTION WHEN UNDEFINED_TABLE THEN END;

  PERFORM cleanup_table( base_name||'_dict', '( id serial PRIMARY KEY, palavra text not null, frequencia integer not null)',false ); 
  
  EXECUTE 'insert into '||base_name||'_dict (palavra, frequencia) select palavra, count(*) frequencia from (
    select unnest(pgm_tsvector2words(to_tsvector(msg)))::text palavra FROM (' || SQL || ') V) vv
    group by palavra order by frequencia desc';
  RETURN 'OK';
END
$$;


--
-- TOC entry 315 (class 1255 OID 28082)
-- Dependencies: 8 856 1097 856
-- Name: matrix(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix(ptr_matrix bigint, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  c.ptr := ptr_matrix;
END;
$$;


--
-- TOC entry 316 (class 1255 OID 28083)
-- Dependencies: 856 856 8 1097
-- Name: matrix(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix(bidimensional_array double precision[], OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  c.ptr := pgm_array2matrix( bidimensional_array );
END;
$$;


--
-- TOC entry 317 (class 1255 OID 28084)
-- Dependencies: 8 1097 856 856
-- Name: matrix(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix(sql text, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  c.ptr := (make_matrix( sql )).matrix;
END;
$$;


--
-- TOC entry 318 (class 1255 OID 28085)
-- Dependencies: 8 1097
-- Name: matrix2table(bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix2table(matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );

  nlines := pgm_matrix_nlines( matrix );
  ncols := pgm_matrix_ncols( matrix );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT i+1, pgm_matrix_get_line( '|| matrix || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 319 (class 1255 OID 28086)
-- Dependencies: 8 856 1097
-- Name: matrix2table(matrix, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix2table(m matrix, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
BEGIN
  SELECT V.nlines, V.ncols from matrix2table( m.ptr, table_name ) V into nlines, ncols;
END;
$$;


--
-- TOC entry 320 (class 1255 OID 28087)
-- Dependencies: 8 1097
-- Name: matrix2table(bigint, bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix2table(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
-- keys: o array de keys criado por make_matrix.
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );

  nlines := pgm_matrix_nlines( matrix );
  ncols := pgm_matrix_ncols( matrix );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT pgm_vector_int_get_elem( ' || keys || ', i ), pgm_matrix_get_line( '|| matrix || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 323 (class 1255 OID 28088)
-- Dependencies: 8 1097
-- Name: matrix2table_transpose(bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix2table_transpose(matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );

  nlines := pgm_matrix_ncols( matrix );
  ncols := pgm_matrix_nlines( matrix );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT i+1, pgm_matrix_get_col( '|| matrix || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 324 (class 1255 OID 28089)
-- Dependencies: 8 1097
-- Name: matrix2table_transpose(bigint, bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION matrix2table_transpose(keys bigint, matrix bigint, table_name text, OUT nlines integer, OUT ncols integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
-- keys: o array de keys criado por make_matrix.
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );

  nlines := pgm_matrix_ncols( matrix );
  ncols := pgm_matrix_nlines( matrix );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT pgm_vector_int_get_elem( ' || keys || ', i ), pgm_matrix_get_col( '|| matrix || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 325 (class 1255 OID 28090)
-- Dependencies: 8
-- Name: max(double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION max(a double precision, b double precision) RETURNS double precision
    LANGUAGE sql IMMUTABLE STRICT COST 1
    AS $_$
 SELECT CASE $1 > $2 WHEN TRUE THEN $1 ELSE $2 END;
$_$;


--
-- TOC entry 326 (class 1255 OID 28091)
-- Dependencies: 1097 8
-- Name: max_elem(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION max_elem(a double precision[]) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN max( x ) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 500 (class 1255 OID 103770)
-- Dependencies: 1097 930 930 8
-- Name: memelmcos(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memelmcos(sql_pesos_a text, sql_pesos_b text, OUT elmcos memelmcos) RETURNS memelmcos
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  elmcos := memelmcos( matrix( sql_pesos_a ), matrix( sql_pesos_b ) );
END;
$$;


--
-- TOC entry 502 (class 1255 OID 103773)
-- Dependencies: 930 930 8 856 856 1097
-- Name: memelmcos(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memelmcos(pesos_a matrix, pesos_b matrix, OUT elmcos memelmcos) RETURNS memelmcos
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  elmcos.pesos_a := pesos_a.ptr;
  elmcos.pesos_b := pesos_b.ptr;
END;
$$;


--
-- TOC entry 503 (class 1255 OID 103774)
-- Dependencies: 930 1097 8 930
-- Name: memelmcos(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memelmcos(ptr_pesos_a bigint, ptr_pesos_b bigint, OUT elmcos memelmcos) RETURNS memelmcos
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  elmcos.pesos_a := ptr_pesos_a;
  elmcos.pesos_b := ptr_pesos_b;
END;
$$;


--
-- TOC entry 327 (class 1255 OID 28092)
-- Dependencies: 8 859 1097 859
-- Name: memnn(bigint, bigint, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memnn(ptr_pesos_a bigint, ptr_pesos_b bigint, aplica_sigmoid boolean DEFAULT false, OUT nn memnn) RETURNS memnn
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  nn.pesos_a := ptr_pesos_a;
  nn.pesos_b := ptr_pesos_b;
  nn.aplica_sigmoid := aplica_sigmoid;
END;
$$;


--
-- TOC entry 328 (class 1255 OID 28093)
-- Dependencies: 859 8 1097 859 856 856
-- Name: memnn(matrix, matrix, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memnn(pesos_a matrix, pesos_b matrix, aplica_sigmoid boolean DEFAULT false, OUT nn memnn) RETURNS memnn
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  nn.pesos_a := pesos_a.ptr;
  nn.pesos_b := pesos_b.ptr;
  nn.aplica_sigmoid := aplica_sigmoid;
END;
$$;


--
-- TOC entry 329 (class 1255 OID 28094)
-- Dependencies: 859 859 8 1097
-- Name: memnn(text, text, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION memnn(sql_pesos_a text, sql_pesos_b text, aplica_sigmoid boolean DEFAULT false, OUT nn memnn) RETURNS memnn
    LANGUAGE plpgsql COST 1
    AS $$
BEGIN
  nn := memnn( matrix( sql_pesos_a ), matrix( sql_pesos_b ), aplica_sigmoid );
END;
$$;


--
-- TOC entry 330 (class 1255 OID 28095)
-- Dependencies: 8
-- Name: min(double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION min(a double precision, b double precision) RETURNS double precision
    LANGUAGE sql IMMUTABLE STRICT COST 1
    AS $_$
 SELECT CASE $1 < $2 WHEN TRUE THEN $1 ELSE $2 END;
$_$;


--
-- TOC entry 331 (class 1255 OID 28096)
-- Dependencies: 8 856 1097
-- Name: ncols(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION ncols(m matrix, OUT cols integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
BEGIN
  cols := pgm_matrix_ncols( m.ptr );
END;
$$;


--
-- TOC entry 332 (class 1255 OID 28097)
-- Dependencies: 856 1097 8
-- Name: nlines(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION nlines(m matrix, OUT nlines integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
BEGIN
  nlines := pgm_matrix_nlines( m.ptr );
END;
$$;


--
-- TOC entry 333 (class 1255 OID 28098)
-- Dependencies: 1097 8
-- Name: norm(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION norm(a double precision[]) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN sqrt(sum( x*x )) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 334 (class 1255 OID 28099)
-- Dependencies: 856 1097 8
-- Name: norm(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION norm(m matrix) RETURNS double precision
    LANGUAGE plpgsql
    AS $$
BEGIN
  return pgm_matrix_norm( m.ptr );
END;
$$;


--
-- TOC entry 335 (class 1255 OID 28100)
-- Dependencies: 1097 869 8 856 856 856
-- Name: ols(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION ols(a matrix, b matrix, OUT x matrix, OUT s vector, OUT rank integer) RETURNS record
    LANGUAGE plpgsql STRICT COST 1000
    AS $$
declare
  rec record;
BEGIN
  rec := pgm_ols( a.ptr, b.ptr );
  x.ptr := rec.b;
  s.ptr := rec.s;
  rank := rec.rank; 
END;
$$;


--
-- TOC entry 336 (class 1255 OID 28101)
-- Dependencies: 1097 8 856
-- Name: opr_array_matrix_multiply(double precision[], matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_array_matrix_multiply(a double precision[], b matrix, OUT c double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  c = pgm_vector_double2array( pgm_vector_double_matrix_multiply( pgm_array2vector_double( a ), b.ptr ) );
END
$$;


--
-- TOC entry 523 (class 1255 OID 28102)
-- Dependencies: 8 1097 859
-- Name: opr_array_memnn(double precision[], memnn); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_array_memnn(a double precision[], b memnn, OUT c double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  c = sigmoid( a * matrix( b.pesos_a ) ) * matrix( b.pesos_b );
END
$$;


--
-- TOC entry 337 (class 1255 OID 28103)
-- Dependencies: 1097 8
-- Name: opr_div_float_vector(double precision, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_div_float_vector(a double precision, b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( a/b ) from ( select unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 338 (class 1255 OID 28104)
-- Dependencies: 1097 8
-- Name: opr_div_vector_vector(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_div_vector_vector(a double precision[], b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa/bb ) from ( select unnest( a ) aa, unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 518 (class 1255 OID 282936)
-- Dependencies: 1097 1010 8
-- Name: opr_fann_apply(double precision[], fann); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_fann_apply(entrada double precision[], net fann, OUT saida double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  saida := pgm_nn_fann_run( net.ann, entrada );
END
$$;


--
-- TOC entry 339 (class 1255 OID 28105)
-- Dependencies: 856 856 8 856 1097 856
-- Name: opr_matrix_add(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_matrix_add(a matrix, b matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
BEGIN
  c.ptr = pgm_matrix_add( a.ptr, b.ptr );
END
$$;


--
-- TOC entry 340 (class 1255 OID 28106)
-- Dependencies: 856 856 856 1097 8
-- Name: opr_matrix_double_multiply(matrix, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_matrix_double_multiply(a matrix, b double precision, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  --raise info 'fator %', b;
  c.ptr = pgm_matrix_double_multiply( a.ptr, b );
END
$$;


--
-- TOC entry 341 (class 1255 OID 28107)
-- Dependencies: 856 8 1097 856 856 856
-- Name: opr_matrix_multiply(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_matrix_multiply(a matrix, b matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
BEGIN
  c.ptr = pgm_matrix_multiply( a.ptr, b.ptr );
END
$$;


--
-- TOC entry 321 (class 1255 OID 28108)
-- Dependencies: 856 856 856 856 1097 8
-- Name: opr_matrix_subtract(matrix, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_matrix_subtract(a matrix, b matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
BEGIN
  c.ptr = pgm_matrix_subtract( a.ptr, b.ptr );
END
$$;


--
-- TOC entry 498 (class 1255 OID 103766)
-- Dependencies: 8 1097 856 856 930 856
-- Name: opr_memelmcos_evaluate_matrix(matrix, memelmcos); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_memelmcos_evaluate_matrix(a matrix, elmcos memelmcos, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  c.ptr := pgm_memelmcos_evaluate_matrix( a.ptr, elmcos );
END
$$;


--
-- TOC entry 342 (class 1255 OID 28109)
-- Dependencies: 859 856 8 856 1097 856
-- Name: opr_memnn_evaluate_matrix(matrix, memnn); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_memnn_evaluate_matrix(a matrix, nn memnn, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  c.ptr := pgm_memnn_evaluate_matrix( a.ptr, nn );
END
$$;


--
-- TOC entry 343 (class 1255 OID 28110)
-- Dependencies: 862 8 1097 862
-- Name: opr_nn_split_part(neuralnet, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_nn_split_part(nn neuralnet, layer integer) RETURNS neuralnet
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
DECLARE
  -- Dada uma rede, particiona na camada layer (necessariamente hidden) e retorna a primeira (layer positivo) ou a segunda (layer negativo) parte da rede.
  rec RECORD;
BEGIN
  -- raise info 'Saida';

  IF layer > 0 THEN
    rec := pgm_nn_split( nn, layer );

    RETURN rec.nn1;
  ELSE
    rec := pgm_nn_split( nn, -layer );

    RETURN rec.nn2;
  END IF;
END
$$;


--
-- TOC entry 344 (class 1255 OID 28111)
-- Dependencies: 8 1097
-- Name: opr_sub_float_vector(double precision, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sub_float_vector(a double precision, b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( a-b ) from ( select unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 345 (class 1255 OID 28112)
-- Dependencies: 8 1097
-- Name: opr_sub_vector_float(double precision[], double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sub_vector_float(a double precision[], b double precision) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa-b ) from ( select unnest( a ) aa ) c;
END
$$;


--
-- TOC entry 346 (class 1255 OID 28113)
-- Dependencies: 8 1097
-- Name: opr_sub_vector_vector(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sub_vector_vector(a double precision[], b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa-bb ) from ( select unnest( a ) aa, unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 347 (class 1255 OID 28114)
-- Dependencies: 8 1097
-- Name: opr_sum_float_vector(double precision, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sum_float_vector(a double precision, b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( a+b ) from ( select unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 348 (class 1255 OID 28115)
-- Dependencies: 8 1097
-- Name: opr_sum_vector_float(double precision[], double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sum_vector_float(a double precision[], b double precision) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa+b ) from ( select unnest( a ) aa ) c;
END
$$;


--
-- TOC entry 349 (class 1255 OID 28116)
-- Dependencies: 8 1097
-- Name: opr_sum_vector_vector(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_sum_vector_vector(a double precision[], b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa+bb ) from ( select unnest( a ) aa, unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 350 (class 1255 OID 28117)
-- Dependencies: 869 8 1097 869 869 856
-- Name: opr_vector_matrix_multiply(vector, matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION opr_vector_matrix_multiply(a vector, b matrix, OUT c vector) RETURNS vector
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  c.ptr = pgm_vector_double_matrix_multiply( a.ptr, b.ptr );
END
$$;


--
-- TOC entry 476 (class 1255 OID 28118)
-- Dependencies: 1097 8
-- Name: pgm_array2matrix(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_array2matrix(bidimensional_array double precision[]) RETURNS bigint
    LANGUAGE plpgsql STABLE STRICT
    AS $$
DECLARE
-- Cria uma matriz a partir de um array bidimensional.
  matriz BIGINT;
  n_linhas INTEGER;
  n_colunas INTEGER;
  i INTEGER;
  j INTEGER;
  low_i INTEGER;
  low_j INTEGER;
BEGIN
  IF array_ndims( bidimensional_array ) <> 2 THEN
    RAISE EXCEPTION 'O argumento deve ser um array bidimensional (uma matriz).';
  END IF;
    
  n_linhas := array_length( bidimensional_array, 1 );
  n_colunas := array_length( bidimensional_array, 2 );
  matriz := pgm_matrix_create( n_linhas, n_colunas );
  low_i := array_lower( bidimensional_array, 1 );
  low_j := array_lower( bidimensional_array, 2 );

  FOR i IN 0 .. n_linhas-1 LOOP 
    FOR j IN 0 .. n_colunas-1 LOOP 
      PERFORM pgm_matrix_set_elem( matriz, i, j, bidimensional_array[low_i+i][low_j+j] );
    END LOOP;
  END LOOP;

  RETURN matriz;
END
$$;

--DROP FUNCTION pgm_hello_world(CSTRING);

CREATE OR REPLACE FUNCTION pgm_hello_world(entrada CSTRING)
  RETURNS CSTRING AS
'$libdir/pgminer.so', 'pgm_hello_world'
  LANGUAGE c STRICT;

--
-- TOC entry 477 (class 1255 OID 30043)
-- Dependencies: 8
-- Name: pgm_array2matrix_float(real[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_array2matrix_float(value real[]) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_arraytype2Matrix_Float';


--
-- TOC entry 351 (class 1255 OID 28119)
-- Dependencies: 8
-- Name: pgm_array2vector_double(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_array2vector_double(unidimensional_array double precision[]) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_array2vector_double';


--
-- TOC entry 521 (class 1255 OID 353645)
-- Dependencies: 8
-- Name: pgm_array2vector_int(integer[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_array2vector_int(unidimensional_array integer[]) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_array2vector_int';


--
-- TOC entry 352 (class 1255 OID 28120)
-- Dependencies: 8
-- Name: pgm_cos(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_cos(matrix_a bigint, matrix_b bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_cos';


--
-- TOC entry 524 (class 1255 OID 372338)
-- Dependencies: 8
-- Name: pgm_create_vector_presence_frequency(tsvector, cstring[], integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_create_vector_presence_frequency(vector tsvector, dict cstring[], flag integer, OUT presence bigint, OUT frequency bigint) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_create_vector_presence_frequency';


--
-- TOC entry 485 (class 1255 OID 70997)
-- Dependencies: 8
-- Name: pgm_cublas_dgemm(bigint, bigint, boolean); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 484 (class 1255 OID 70996)
-- Dependencies: 8
-- Name: pgm_cublas_sgemm(bigint, bigint, boolean); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 353 (class 1255 OID 28121)
-- Dependencies: 8
-- Name: pgm_kmeans(bigint, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_kmeans(pontos bigint, k integer, seed integer, fim integer, OUT centroide bigint, OUT grupo bigint, OUT sumd double precision) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_kmeans';


--
-- TOC entry 512 (class 1255 OID 109305)
-- Dependencies: 8 1097
-- Name: pgm_lsh_cos(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_lsh_cos(matrix_a bigint, matrix_b bigint, OUT matrix_c bigint) RETURNS bigint
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1000
    AS $$
BEGIN
  matrix_c := pgm_lsh_cos( matrix_a, matrix_b, blas_multiply( matrix_a, matrix_b ) );
END
$$;


--
-- TOC entry 511 (class 1255 OID 109304)
-- Dependencies: 8
-- Name: pgm_lsh_cos(bigint, bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_lsh_cos(matrix_a bigint, matrix_b bigint, matrix_c bigint) RETURNS bigint
    LANGUAGE c IMMUTABLE STRICT COST 1000
    AS '$libdir/pgminer.so', 'pgm_lsh_cos';


--
-- TOC entry 354 (class 1255 OID 28122)
-- Dependencies: 8
-- Name: pgm_lsvd(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_lsvd(matriz bigint, numfeatures integer, OUT u bigint, OUT s bigint, OUT vt bigint) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_lapack_svd';


--
-- TOC entry 355 (class 1255 OID 28123)
-- Dependencies: 8
-- Name: pgm_matrix2array(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix2array(matrix bigint) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix2arraytype';


--
-- TOC entry 356 (class 1255 OID 28124)
-- Dependencies: 8 1097 856
-- Name: pgm_matrix2array(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix2array(a matrix) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  return pgm_matrix2array( a.ptr );
END
$$;


--
-- TOC entry 357 (class 1255 OID 28125)
-- Dependencies: 8
-- Name: pgm_matrix_add(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_add(matrix_a bigint, matrix_b bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_add';


--
-- TOC entry 358 (class 1255 OID 28126)
-- Dependencies: 8
-- Name: pgm_matrix_apply_inverse_sigmoid(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_apply_inverse_sigmoid(matrix bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_apply_inverse_sigmoid';


--
-- TOC entry 359 (class 1255 OID 28127)
-- Dependencies: 8
-- Name: pgm_matrix_apply_scale(bigint, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_apply_scale(m bigint, scale double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_apply_scale';


--
-- TOC entry 360 (class 1255 OID 28128)
-- Dependencies: 8
-- Name: pgm_matrix_apply_sigmoid(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_apply_sigmoid(matrix bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_apply_sigmoid';


--
-- TOC entry 488 (class 1255 OID 79191)
-- Dependencies: 8
-- Name: pgm_matrix_copy(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_copy(matrix bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_double_copy';


--
-- TOC entry 362 (class 1255 OID 28130)
-- Dependencies: 8
-- Name: pgm_matrix_create(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_create(n_lines integer, n_cols integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_create';


--
-- TOC entry 363 (class 1255 OID 28131)
-- Dependencies: 1097 8
-- Name: pgm_matrix_double_multiply(bigint, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_double_multiply(a bigint, b double precision, OUT c bigint) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
  n_linhas integer;
  n_colunas integer;
  i integer;
  j integer;
BEGIN
  
  n_linhas := pgm_matrix_nlines( a );
  n_colunas := pgm_matrix_ncols( a );

  c := pgm_matrix_create( n_linhas, n_colunas );

  FOR i IN 0 .. n_linhas-1 LOOP 
    FOR j IN 0 .. n_colunas-1 LOOP 
      PERFORM pgm_matrix_set_elem( c, i, j, pgm_matrix_get_elem( a, i, j ) * b );
    END LOOP;
  END LOOP;


END;
$$;


--
-- TOC entry 482 (class 1255 OID 30045)
-- Dependencies: 8
-- Name: pgm_matrix_float2array(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_float2array(matrix bigint) RETURNS real[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_float2arraytype';


--
-- TOC entry 475 (class 1255 OID 30034)
-- Dependencies: 8
-- Name: pgm_matrix_float_create(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_float_create(n_lines integer, n_cols integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_float_create';


--
-- TOC entry 478 (class 1255 OID 30036)
-- Dependencies: 8
-- Name: pgm_matrix_float_set_elem(bigint, integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_float_set_elem(matrix bigint, line integer, col integer, value double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_float_set_elem';


--
-- TOC entry 295 (class 1255 OID 28132)
-- Dependencies: 8
-- Name: pgm_matrix_free(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_free(matrix bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_free';


--
-- TOC entry 312 (class 1255 OID 28133)
-- Dependencies: 8
-- Name: pgm_matrix_get(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_get(matrix bigint) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_get';


--
-- TOC entry 365 (class 1255 OID 28134)
-- Dependencies: 8
-- Name: pgm_matrix_get_col(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_get_col(matrix bigint, line integer) RETURNS double precision[]
    LANGUAGE c STABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_get_col';


--
-- TOC entry 366 (class 1255 OID 28135)
-- Dependencies: 8
-- Name: pgm_matrix_get_elem(bigint, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_get_elem(matrix bigint, line integer, col integer) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_get_elem';


--
-- TOC entry 367 (class 1255 OID 28136)
-- Dependencies: 8
-- Name: pgm_matrix_get_line(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_get_line(matrix bigint, line integer) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_get_line';


--
-- TOC entry 368 (class 1255 OID 28137)
-- Dependencies: 8
-- Name: pgm_matrix_get_min_max_mean(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_get_min_max_mean(matrix bigint, OUT min double precision, OUT max double precision, OUT mean double precision) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_get_min_max_mean';


--
-- TOC entry 369 (class 1255 OID 28138)
-- Dependencies: 8
-- Name: pgm_matrix_inverse(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_inverse(matrix bigint) RETURNS bigint
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_inverse';


--
-- TOC entry 370 (class 1255 OID 28139)
-- Dependencies: 8 1097
-- Name: pgm_matrix_lpinv(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_lpinv(matrix bigint) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE 
  Ut bigint;
  U bigint;
  S bigint;
  Vt bigint;
  pinv bigint;
BEGIN
  SELECT * from pgm_lsvd( matrix, min( pgm_matrix_nlines( matrix ), pgm_matrix_ncols( matrix ) )::integer ) INTO U, S, Vt;
  Ut := pgm_matrix_transpose( U );
  pinv := pgm_matrix_pinv( svd_record( Ut, S, Vt ) );

  PERFORM pgm_matrix_free( u );
  PERFORM pgm_matrix_free( ut );
  PERFORM pgm_matrix_free( vt );
  PERFORM pgm_vector_double_free( s );

  RETURN pinv;
END
$$;


--
-- TOC entry 486 (class 1255 OID 71003)
-- Dependencies: 8
-- Name: pgm_matrix_multiply(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_multiply(matrix_a bigint, matrix_b bigint) RETURNS bigint
    LANGUAGE c IMMUTABLE STRICT COST 1000
    AS '$libdir/pgminer.so', 'pgm_matrix_multiply';


--
-- TOC entry 372 (class 1255 OID 28141)
-- Dependencies: 8
-- Name: pgm_matrix_ncols(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_ncols(matrix bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_ncols';


--
-- TOC entry 373 (class 1255 OID 28142)
-- Dependencies: 8
-- Name: pgm_matrix_nlines(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_nlines(matrix bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_nlines';


--
-- TOC entry 483 (class 1255 OID 65403)
-- Dependencies: 8
-- Name: pgm_matrix_norm(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_norm(matrix bigint) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_norma';


--
-- TOC entry 374 (class 1255 OID 28144)
-- Dependencies: 866 8 1097
-- Name: pgm_matrix_pinv(svd_record); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_pinv(svd svd_record) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE
  SP bigint;
  SPArray float8[];
  nonZeroFeatures integer;
  PINV bigint;
BEGIN
  nonZeroFeatures := (SELECT count(*) from (SELECT unnest( pgm_vector_double_get( svd.S ) ) x ) sii WHERE Sii.x > 1E-6);
  SPArray := (SELECT array_agg( 1/sii.x ) from (SELECT unnest( pgm_vector_double_get( svd.S ) ) x order by x desc) sii WHERE Sii.x > 1E-6); 
  
  --RAISE INFO '1/Sigma: %', SPArray;

  SP := pgm_vector_double_create( nonZeroFeatures );
  PERFORM pgm_vector_double_set_elem( SP, i-1, SPArray[i] ) FROM generate_series( 1, nonZeroFeatures ) i;

  PINV := pgm_svd_matrix_reconstruct( svd.vt, SP, svd.ut, nonZeroFeatures );

  PERFORM pgm_vector_double_free( SP );

  RETURN PINV;
END
$$;


--
-- TOC entry 375 (class 1255 OID 28145)
-- Dependencies: 1097 8
-- Name: pgm_matrix_pinv(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_pinv(matrix bigint) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE 
  rec RECORD;
  pinv bigint;
BEGIN
  rec := pgm_svd_rec( matrix, max( pgm_matrix_nlines( matrix ), pgm_matrix_ncols( matrix ) )::integer );
  pinv := pgm_matrix_pinv( rec );

  PERFORM pgm_matrix_free( rec.ut );
  PERFORM pgm_matrix_free( rec.vt );
  PERFORM pgm_vector_double_free( rec.s );

  RETURN pinv;
END
$$;


--
-- TOC entry 322 (class 1255 OID 28146)
-- Dependencies: 1097 8
-- Name: pgm_matrix_pinv_reg(bigint, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_pinv_reg(matrix bigint, lambda double precision) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE
  H bigint;
  HT bigint;
  aux bigint;
  inv_aux bigint;
  resultado bigint;
BEGIN
  H := matrix;
  HT := pgm_matrix_transpose( h );

  IF pgm_matrix_nlines( H ) > pgm_matrix_ncols( H ) THEN
    aux := pgm_matrix_multiply( HT, H );
  ELSE
    aux := pgm_matrix_multiply( H, HT );
  END IF;
  
  -- RAISE INFO 'aux %, %', pgm_matrix_nlines( aux ), pgm_matrix_ncols( aux );

  IF lambda > 0 THEN
    PERFORM pgm_matrix_set_elem( aux, i, i, pgm_matrix_get_elem( aux, i, i ) + 1.0/lambda ) 
      FROM generate_series( 0, pgm_matrix_nlines( aux ) - 1 ) i;
  END IF;
   
  inv_aux := pgm_matrix_inverse( aux );

  IF pgm_matrix_nlines( H ) > pgm_matrix_ncols( H ) THEN
    resultado := pgm_matrix_multiply( inv_aux, HT );
  ELSE
    resultado := pgm_matrix_multiply( HT, inv_aux );
  END IF;

  -- RAISE INFO 'pinv %, %', pgm_matrix_nlines( resultado ), pgm_matrix_ncols( resultado );

  PERFORM pgm_matrix_free( HT );
  PERFORM pgm_matrix_free( aux );
  PERFORM pgm_matrix_free( inv_aux );

  RETURN resultado;
END
$$;


--
-- TOC entry 459 (class 1255 OID 198866)
-- Dependencies: 8
-- Name: pgm_matrix_remove_line_col(bigint, integer[], integer[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_remove_line_col(matrix bigint, linhas integer[], colunas integer[]) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_remove_line_col';


--
-- TOC entry 376 (class 1255 OID 28147)
-- Dependencies: 8
-- Name: pgm_matrix_set_col_value(bigint, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_set_col_value(m bigint, col integer, value double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_set_col_value';


--
-- TOC entry 479 (class 1255 OID 28148)
-- Dependencies: 8
-- Name: pgm_matrix_set_elem(bigint, integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_set_elem(matrix bigint, line integer, col integer, value double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_set_elem';


--
-- TOC entry 377 (class 1255 OID 28149)
-- Dependencies: 8
-- Name: pgm_matrix_set_line(bigint, integer, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_set_line(matrix bigint, line integer, valor double precision[]) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_set_line';


--
-- TOC entry 378 (class 1255 OID 28150)
-- Dependencies: 8
-- Name: pgm_matrix_set_line_value(bigint, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_set_line_value(m bigint, line integer, value double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_set_line_value';


--
-- TOC entry 379 (class 1255 OID 28151)
-- Dependencies: 8 1097
-- Name: pgm_matrix_set_value(bigint, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_set_value(m bigint, value double precision) RETURNS bigint
    LANGUAGE plpgsql IMMUTABLE STRICT COST 10
    AS $$
DECLARE
  i integer;
BEGIN
  for i in 0 .. pgm_matrix_nlines( m ) - 1 loop
    perform pgm_matrix_set_line_value( m, i, value );
  end loop;
  return m;
END
$$;


--
-- TOC entry 380 (class 1255 OID 28152)
-- Dependencies: 8
-- Name: pgm_matrix_subtract(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_subtract(matrix_a bigint, matrix_b bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_subtract';


--
-- TOC entry 381 (class 1255 OID 28153)
-- Dependencies: 8
-- Name: pgm_matrix_transpose(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_matrix_transpose(matrix bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_matrix_transpose';


--
-- TOC entry 504 (class 1255 OID 103775)
-- Dependencies: 930 8 1097
-- Name: pgm_memelmcos_evaluate_matrix(bigint, memelmcos); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_memelmcos_evaluate_matrix(matrix_ptr bigint, elmcos memelmcos) RETURNS bigint
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
DECLARE
  meio bigint;
  meio1 bigint;
  meio2 bigint;
  saida bigint;
BEGIN
  -- Criar uma função para considerar o bias
  meio1 := pgm_lsh_cos( matrix_ptr, elmcos.pesos_a );
  meio2 := pgm_matrix_multiply( matrix_ptr, elmcos.pesos_a );
  meio := pgm_matrix_add( meio1, meio2 );
  saida := pgm_matrix_multiply( meio, elmcos.pesos_b );
  
  PERFORM pgm_matrix_free( meio );
  PERFORM pgm_matrix_free( meio1 );
  PERFORM pgm_matrix_free( meio2 );

  RETURN saida;
END;
$$;


--
-- TOC entry 383 (class 1255 OID 28154)
-- Dependencies: 859 1097 8
-- Name: pgm_memnn_evaluate_matrix(bigint, memnn); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_memnn_evaluate_matrix(matrix_ptr bigint, nn memnn) RETURNS bigint
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
DECLARE
  meio bigint;
  saida bigint;
BEGIN
  -- Criar uma função para considerar o bias
  meio := pgm_matrix_multiply( matrix_ptr, nn.pesos_a );
  PERFORM pgm_matrix_apply_sigmoid( meio );

  saida := pgm_matrix_multiply( meio, nn.pesos_b );
  if( nn.aplica_sigmoid ) then 
    PERFORM pgm_matrix_apply_sigmoid( saida );
  end if;

  PERFORM pgm_matrix_free( meio );

  RETURN saida;
END;
$$;


--
-- TOC entry 384 (class 1255 OID 28155)
-- Dependencies: 8 1097 859
-- Name: pgm_memnn_evaluate_vector(bigint, memnn); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_memnn_evaluate_vector(vector_ptr bigint, nn memnn) RETURNS bigint
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
DECLARE
  meio bigint;
  saida bigint;
BEGIN
  meio := pgm_vector_double_matrix_multiply( vector_ptr, nn );
  -- precisa criar apply sigmoid para vector
END;
$$;


--
-- TOC entry 385 (class 1255 OID 28156)
-- Dependencies: 8 862 862
-- Name: pgm_nn_equal(neuralnet, neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_equal(nn1 neuralnet, nn2 neuralnet) RETURNS boolean
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_equal';

CREATE FUNCTION pgm_nn_fann_get_mse_report(fann bigint) RETURNS double precision
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_fann_get_mse_report';

--
-- TOC entry 386 (class 1255 OID 28157)
-- Dependencies: 8 862
-- Name: pgm_nn_evaluate(neuralnet, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_evaluate(nn neuralnet, vector double precision[]) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_evaluate';


--
-- TOC entry 387 (class 1255 OID 28158)
-- Dependencies: 862 8
-- Name: pgm_nn_evaluate(double precision[], neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_evaluate(vector double precision[], nn neuralnet) RETURNS double precision[]
    LANGUAGE sql STRICT COST 1
    AS $_$
select pgm_nn_evaluate( $2, $1 )
$_$;


--
-- TOC entry 388 (class 1255 OID 28159)
-- Dependencies: 8 862
-- Name: pgm_nn_fann2neuralnet(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann2neuralnet(fann bigint) RETURNS neuralnet
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_fann2neuralnet';


--
-- TOC entry 389 (class 1255 OID 28160)
-- Dependencies: 8
-- Name: pgm_nn_fann_create_train_data(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_create_train_data(data_matrix_input bigint, data_matrix_ouput bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_create_train_data';


--
-- TOC entry 390 (class 1255 OID 28161)
-- Dependencies: 8
-- Name: pgm_nn_fann_free_train_data(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_free_train_data(train_data bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_free_train_data';


--
-- TOC entry 391 (class 1255 OID 28162)
-- Dependencies: 8
-- Name: pgm_nn_fann_run(bigint, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_run(ann bigint, _input double precision[]) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_fann_run';


--
-- TOC entry 392 (class 1255 OID 28163)
-- Dependencies: 8
-- Name: pgm_nn_fann_test(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_test(train_data bigint, ann bigint, OUT mse double precision) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_fann_test';


--
-- TOC entry 393 (class 1255 OID 28164)
-- Dependencies: 8
-- Name: pgm_nn_fann_train(bigint, integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_train(train_data bigint, max_epochs integer, epochs_between_reports integer, desired_error double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_train_neuralnet2';


--
-- TOC entry 394 (class 1255 OID 28165)
-- Dependencies: 8
-- Name: pgm_nn_fann_train(bigint, bigint, integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_train(train_data bigint, ann bigint, max_epochs integer, epochs_between_reports integer, desired_error double precision) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_train_neuralnet3';


--
-- TOC entry 395 (class 1255 OID 28166)
-- Dependencies: 8
-- Name: pgm_nn_fann_train(bigint, integer[], integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_fann_train(train_data bigint, hidden_layer integer[], functionActivation integer, steepness double precision, max_epochs integer, epochs_between_reports integer, desired_error double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_train_neuralnet1';

--
-- TOC entry 396 (class 1255 OID 28167)
-- Dependencies: 8 862 862
-- Name: pgm_nn_get_distance(neuralnet, neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_get_distance(nna neuralnet, nnb neuralnet) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_get_distance';


--
-- TOC entry 397 (class 1255 OID 28168)
-- Dependencies: 8 862
-- Name: pgm_nn_get_mse(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_get_mse(nn neuralnet) RETURNS double precision
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_get_MSE';


--
-- TOC entry 398 (class 1255 OID 28169)
-- Dependencies: 862 8
-- Name: pgm_nn_get_scaling(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_get_scaling(nn neuralnet, OUT input_min double precision, OUT input_max double precision, OUT output_min double precision, OUT output_max double precision) RETURNS record
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_get_scaling';


--
-- TOC entry 399 (class 1255 OID 28170)
-- Dependencies: 8 862
-- Name: pgm_nn_get_weight_array(neuralnet, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_get_weight_array(nn neuralnet, layer integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_get_weight_array';


--
-- TOC entry 400 (class 1255 OID 28171)
-- Dependencies: 862 8
-- Name: pgm_nn_inputsize(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_inputsize(nn neuralnet) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_inputsize';


--
-- TOC entry 401 (class 1255 OID 28172)
-- Dependencies: 862 862 862 8
-- Name: pgm_nn_merge(neuralnet, neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_merge(nna neuralnet, nnb neuralnet) RETURNS neuralnet
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_merge';


--
-- TOC entry 402 (class 1255 OID 28173)
-- Dependencies: 862 8
-- Name: pgm_nn_neuralnet2fann(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_neuralnet2fann(nn neuralnet) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_neuralnet2fann';


--
-- TOC entry 403 (class 1255 OID 28174)
-- Dependencies: 8 862
-- Name: pgm_nn_nlayers(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_nlayers(neuralnet) RETURNS integer[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_nlayers';


--
-- TOC entry 404 (class 1255 OID 28175)
-- Dependencies: 862 8
-- Name: pgm_nn_outputsize(neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_outputsize(nn neuralnet) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_outputsize';


--
-- TOC entry 405 (class 1255 OID 28176)
-- Dependencies: 8 862
-- Name: pgm_nn_set_mse(neuralnet, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_set_mse(nn neuralnet, new_mse double precision) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_set_MSE';


--
-- TOC entry 406 (class 1255 OID 28177)
-- Dependencies: 862 862 8
-- Name: pgm_nn_set_scaling(neuralnet, double precision, double precision, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_set_scaling(nn neuralnet, input_min double precision, input_max double precision, output_min double precision, output_max double precision) RETURNS neuralnet
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_set_scaling';


--
-- TOC entry 407 (class 1255 OID 28178)
-- Dependencies: 8 862 862
-- Name: pgm_nn_set_weight_array(neuralnet, bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_set_weight_array(nn neuralnet, matrix bigint, layer integer) RETURNS neuralnet
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_set_weight_array';


--
-- TOC entry 408 (class 1255 OID 28179)
-- Dependencies: 862 8 862 862
-- Name: pgm_nn_split(neuralnet, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_split(nn neuralnet, split integer, OUT nn1 neuralnet, OUT nn2 neuralnet) RETURNS record
    LANGUAGE c IMMUTABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_nn_split';


--
-- TOC entry 409 (class 1255 OID 28180)
-- Dependencies: 8 1097 862 862
-- Name: pgm_nn_split_part(neuralnet, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_split_part(nn neuralnet, layer integer, part integer) RETURNS neuralnet
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
DECLARE
  -- Dada uma rede, particiona na camada layer (necessariamente hidden) e retorna a primeira ou a segunda parte da rede.
  rec RECORD;
BEGIN
  rec := pgm_nn_split( nn, layer );

  -- RAISE INFO '%, %', rec.nn1, rec.nn2;

  IF part = 1 THEN
    RETURN rec.nn1;
  ELSE
    RETURN rec.nn2;
  END IF;
END
$$;


--
-- TOC entry 382 (class 1255 OID 28181)
-- Dependencies: 8 1097 862
-- Name: pgm_nn_test(bigint, bigint, neuralnet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_nn_test(matrix_in bigint, matrix_out bigint, nn neuralnet) RETURNS double precision
    LANGUAGE plpgsql
    AS $$
DECLARE

  ann bigint;
  new_mse double precision;
  
BEGIN
  ann := pgm_nn_neuralnet2fann( nn );
  new_mse := pgm_nn_fann_test(matrix_in, matrix_out, ann);
  PERFORM pgm_nn_set_mse( nn, new_mse );
  RETURN pgm_nn_get_mse( nn );
  
END;
$$;


--
-- TOC entry 410 (class 1255 OID 28182)
-- Dependencies: 8
-- Name: pgm_ols(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_ols(matrix bigint, matrix_b bigint, OUT b bigint, OUT s bigint, OUT rank integer) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_ordinary_least_squares';


--
-- TOC entry 487 (class 1255 OID 79190)
-- Dependencies: 8
-- Name: pgm_random_matrix(integer, integer, boolean); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 411 (class 1255 OID 28183)
-- Dependencies: 8 1097
-- Name: pgm_scale_vector(double precision, double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_scale_vector(scale double precision, vector double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( scale*elem ) FROM (SELECT unnest( vector ) elem ) v;
END
$$;


--
-- TOC entry 412 (class 1255 OID 28184)
-- Dependencies: 8 1097
-- Name: pgm_scale_vector(double precision[], double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_scale_vector(vector double precision[], scale double precision) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( scale*elem ) FROM (SELECT unnest( vector ) elem ) v;
END
$$;


--
-- TOC entry 413 (class 1255 OID 28185)
-- Dependencies: 8
-- Name: pgm_svd(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_svd(matriz bigint, numfeatures integer, OUT ut bigint, OUT s bigint, OUT vt bigint) RETURNS record
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_svd';


--
-- TOC entry 414 (class 1255 OID 28186)
-- Dependencies: 8 1097
-- Name: pgm_svd_error_features(bigint, bigint, bigint, bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_svd_error_features(matriz_original bigint, ut bigint, s bigint, vt bigint, numfeatures integer) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
DECLARE
-- Cria um vetor double contendo o erro da matriz reconstruida usando de 1 a numfeatures,
-- calculado usando a norma quadratica.
  vetor_erro BIGINT;
  erro float8;
  i integer;
  mr bigint;
  ms bigint;
BEGIN
  vetor_erro := pgm_vector_double_create( numfeatures );

  for i in 1..numfeatures loop
    mr := pgm_svd_matrix_reconstruct( UT, S, VT, i );
    ms := pgm_matrix_subtract( matriz_original, mr );
    erro := pgm_matrix_norm( ms );  

    PERFORM pgm_vector_double_set_elem( vetor_erro, i-1, erro );

    PERFORM pgm_matrix_free( ms ); 
    PERFORM pgm_matrix_free( mr ); 
  end loop;
 
  RETURn vetor_erro;
END
$$;


--
-- TOC entry 415 (class 1255 OID 28187)
-- Dependencies: 866 8 1097
-- Name: pgm_svd_matrix_reconstruct(svd_record, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_svd_matrix_reconstruct(svd svd_record, numfeatures integer) RETURNS bigint
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  RETURN pgm_svd_matrix_reconstruct( svd.ut, svd.s, svd.vt, numfeatures );
END
$$;


--
-- TOC entry 416 (class 1255 OID 28188)
-- Dependencies: 8
-- Name: pgm_svd_matrix_reconstruct(bigint, bigint, bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_svd_matrix_reconstruct(ut bigint, s bigint, vt bigint, numfeatures integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_svd_matrix_reconstruct';


--
-- TOC entry 417 (class 1255 OID 28189)
-- Dependencies: 8 1097 866
-- Name: pgm_svd_rec(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_svd_rec(matriz bigint, numfeatures integer) RETURNS svd_record
    LANGUAGE plpgsql STRICT
    AS $$
DECLARE 
  REC record;
  RES SVD_RECORD;
BEGIN
  REC := pgm_svd( matriz, numfeatures );
  RES.ut := rec.ut;
  RES.s := rec.s;
  RES.vt := rec.vt;
  RETURN RES;
END
$$;


--
-- TOC entry 298 (class 1255 OID 164172)
-- Dependencies: 8
-- Name: pgm_svm_model2string(bigint); Type: FUNCTION; Schema: public; Owner: -
--

--
-- TOC entry 499 (class 1255 OID 241512)
-- Dependencies: 8
-- Name: pgm_svm_predict(bigint, bigint, bigint, boolean); Type: FUNCTION; Schema: public; Owner: -
--


--
-- TOC entry 509 (class 1255 OID 233113)
-- Dependencies: 8
-- Name: pgm_svm_string2model(cstring); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 361 (class 1255 OID 164231)
-- Dependencies: 8
-- Name: pgm_svm_train(bigint, bigint, integer, integer, integer, double precision, double precision, double precision, double precision, double precision, double precision, double precision, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 472 (class 1255 OID 372292)
-- Dependencies: 8
-- Name: pgm_tsvector2words(tsvector); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_tsvector2words(t tsvector) RETURNS cstring[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_tsvector2words';


--
-- TOC entry 418 (class 1255 OID 28190)
-- Dependencies: 8 1097
-- Name: pgm_vector_distance(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_distance(a double precision[], b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa-bb ) from ( select unnest( a ) aa, unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 419 (class 1255 OID 28191)
-- Dependencies: 8
-- Name: pgm_vector_double2array(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double2array(vector bigint) RETURNS double precision[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vectorDouble2arraytype';


--
-- TOC entry 420 (class 1255 OID 28192)
-- Dependencies: 8 1097 869
-- Name: pgm_vector_double2array(vector); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double2array(a vector) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT
    AS $$
BEGIN
  return pgm_vector_double2array( a.ptr );
END
$$;


--
-- TOC entry 421 (class 1255 OID 28193)
-- Dependencies: 8
-- Name: pgm_vector_double_copy(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_copy(vector bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vector_double_copy';


--
-- TOC entry 422 (class 1255 OID 28194)
-- Dependencies: 8
-- Name: pgm_vector_double_create(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_create(n_elem integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_double_create';


--
-- TOC entry 423 (class 1255 OID 28195)
-- Dependencies: 8
-- Name: pgm_vector_double_free(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_free(vector bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_double_free';


--
-- TOC entry 424 (class 1255 OID 28196)
-- Dependencies: 8
-- Name: pgm_vector_double_get(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_get(vector bigint) RETURNS double precision[]
    LANGUAGE c STABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_vector_double_get';


--
-- TOC entry 425 (class 1255 OID 28197)
-- Dependencies: 8
-- Name: pgm_vector_double_get_elem(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_get_elem(vector bigint, pos integer) RETURNS double precision
    LANGUAGE c STABLE STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_double_get_elem';


--
-- TOC entry 426 (class 1255 OID 28198)
-- Dependencies: 8
-- Name: pgm_vector_double_matrix_multiply(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_matrix_multiply(vector bigint, matrix bigint) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vector_matrix_multiply';


--
-- TOC entry 427 (class 1255 OID 28199)
-- Dependencies: 8
-- Name: pgm_vector_double_nlines(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_nlines(vector bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_double_elem';


--
-- TOC entry 428 (class 1255 OID 28200)
-- Dependencies: 8
-- Name: pgm_vector_double_set_elem(bigint, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_double_set_elem(vector bigint, pos integer, val double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_double_set_elem';


--
-- TOC entry 501 (class 1255 OID 249705)
-- Dependencies: 8
-- Name: pgm_vector_int2array(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int2array(vector bigint) RETURNS integer[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vectorInt2arraytype';


--
-- TOC entry 429 (class 1255 OID 28201)
-- Dependencies: 8
-- Name: pgm_vector_int_create(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_create(n_elems integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_int_create';


--
-- TOC entry 430 (class 1255 OID 28202)
-- Dependencies: 8
-- Name: pgm_vector_int_free(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_free(vector bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_int_free';


--
-- TOC entry 431 (class 1255 OID 28203)
-- Dependencies: 8
-- Name: pgm_vector_int_get(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_get(vector bigint) RETURNS integer[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vector_int_get';


--
-- TOC entry 432 (class 1255 OID 28204)
-- Dependencies: 8
-- Name: pgm_vector_int_get_elem(bigint, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_get_elem(vetor bigint, posicao integer) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_int_get_elem';


--
-- TOC entry 433 (class 1255 OID 28205)
-- Dependencies: 8
-- Name: pgm_vector_int_nlines(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_nlines(vetor bigint) RETURNS integer
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_int_elem';


--
-- TOC entry 434 (class 1255 OID 28206)
-- Dependencies: 8
-- Name: pgm_vector_int_set_elem(bigint, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_int_set_elem(vector bigint, pos integer, val integer) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vetor_int_set_elem';


--
-- TOC entry 435 (class 1255 OID 28207)
-- Dependencies: 8
-- Name: pgm_vector_norm(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_norm(vector bigint) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vector_norma';


--
-- TOC entry 436 (class 1255 OID 28208)
-- Dependencies: 1097 8
-- Name: pgm_vector_vector(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vector_vector(a double precision[], b double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( aa*bb ) from ( select unnest( a ) aa, unnest( b ) bb ) c;
END
$$;


--
-- TOC entry 437 (class 1255 OID 28209)
-- Dependencies: 8
-- Name: pgm_vectorint2array(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_vectorint2array(vector bigint) RETURNS integer[]
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_vectorInt2arraytype';


--
-- TOC entry 438 (class 1255 OID 28210)
-- Dependencies: 8
-- Name: pgm_weighted_mean(bigint, double precision, bigint, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pgm_weighted_mean(matrix_a bigint, p double precision, matrix_b bigint, q double precision) RETURNS bigint
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', 'pgm_weighted_mean';


--
-- TOC entry 439 (class 1255 OID 28211)
-- Dependencies: 1097 856 856 856 8
-- Name: pinv(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pinv(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_pinv( m.ptr );
END;
$$;


--
-- TOC entry 440 (class 1255 OID 28212)
-- Dependencies: 856 856 856 1097 8
-- Name: pinv_reg(matrix, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION pinv_reg(m matrix, lambda double precision DEFAULT (100000)::numeric, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_pinv_reg( m.ptr, lambda );
END;
$$;


--
-- TOC entry 441 (class 1255 OID 28213)
-- Dependencies: 1097 8
-- Name: possui_coluna(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION possui_coluna(sql text, coluna text) RETURNS boolean
    LANGUAGE plpgsql STABLE COST 1
    AS $$
-- Retorna verdadeiro se a query possui a coluna passada.
BEGIN
  BEGIN
    IF STRPOS( SQL, ' ' ) > 0 THEN
      EXECUTE 'SELECT ' || coluna || ' FROM (' || SQL || ') VV LIMIT 0';
    ELSE
      EXECUTE 'SELECT ' || coluna || ' FROM ' || SQL || ' VV LIMIT 0';
    END IF;

    RETURN TRUE;

    EXCEPTION
      WHEN UNDEFINED_COLUMN THEN 
        RETURN FALSE;
  END;
END;
$$;


--
-- TOC entry 442 (class 1255 OID 28214)
-- Dependencies: 1097 8
-- Name: prepare_data_to_learn(text, text, integer, boolean, double precision, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION prepare_data_to_learn(sql text, nome_tabela text, k_folds integer DEFAULT 5, keep_col_ratio boolean DEFAULT true, min_e double precision DEFAULT (-1), max_e double precision DEFAULT 1, tol double precision DEFAULT 0.001) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
-- CREATE OR REPLACE FUNCTION prepare_data_to_learn(sql text, nome_tabela text, k_folds integer=5, min_e double precision=-1, max_e double precision=1, tol double precision=1e-3)
-- SQL eh uma consulta de um dos dois tipos a seguir, representando uma classificação ou uma regressão:
--
-- 1) Classificação: 'select id, entrada, classe from tabela'
--    Ex: 'select id, array[ petal_w, sepal_w, petal_l, sepal_l ] entrada, classe from iris'
--        classe pode ser integer ou text, e será convertido para uma representação vetorial do tipo (0,1,0,0,0)
--
-- 2) Regressão:     'select id, entrada, saida from tabela'
--    Ex: 'select x*100+y id, array[ x, y ] entrada, array[ x*y ] saida from generate_series( 1, 10 ) x, generate_series( 1, 10 ) y'
-- 
-- nome_tabela: nome da tabela a ser criada, contendo as seguintes colunas: id, fold, entrada, saida
-- k_folds: número de folds na tabela a ser criada
-- keep_col_ratio: define se a normalização manterá a razão entre as colunas,
-- min/max: normalização da entrada no intervalo (min+tol, max-tol).
-- OBS: a saída é sempre normalizada no intervalo (tol, 1-tol), exceto se tol >= 0.5 (nesse caso não há normalização da saída).
--      Se min >= max, não há normalização da entrada.
-- Retorno: o número de linhas da tabela criada.
DECLARE
  total bigint;
BEGIN
  IF NOT possui_coluna( SQL, 'ID' ) OR NOT possui_coluna( SQL, 'entrada' ) THEN
    RAISE EXCEPTION 'A consulta deve ter os campos id, entrada, classe ou saida. [%]', sql;
  END IF;

  IF possui_coluna( SQL, 'classe' ) THEN

    IF tol >= 0.5 THEN
      RAISE EXCEPTION 'Para classificação a tolerância deve ser menor que 0.5 (tol = %)', tol;
    END IF;

    PERFORM cleanup_table( nome_tabela, '( id INTEGER PRIMARY KEY, fold INTEGER, entrada FLOAT8[], saida FLOAT8[] )' );
    PERFORM cleanup_table( nome_tabela || '_classes', '( classe text PRIMARY KEY, vector FLOAT8[] )' );

    EXECUTE 'INSERT INTO ' || nome_tabela || '_classes( classe, vector ) ' || 
            'SELECT * from classe2vector( ''select distinct dd.classe from (' || SQL || ') dd'', ' || tol || ' ) c';
    
    IF min_e < max_e THEN
      PERFORM analyze_dense_matrix( 'SELECT id, entrada valor from ( ' || SQL || ' ) VV', nome_tabela || '_entrada' );
     
      EXECUTE 'UPDATE ' || nome_tabela || '_entrada_col_st SET ' ||
              '  min = min - ' || tol || ', ' ||
              '  max = max + ' || tol;

      IF keep_col_ratio THEN
        EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
                '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, '|| max_e - min_e ||
                '*(d.entrada-a.valor)*(1.0/(b.valor-a.valor))+ '|| min_e ||', c.vector FROM (' || SQL || ') d, ' || 
                   nome_tabela || '_classes c, ' ||
                '  (select min( min ) valor FROM ' || nome_tabela || '_entrada_col_st ) A,' || 
                '  (select max( max ) valor FROM ' || nome_tabela || '_entrada_col_st ) B' || 
                '  WHERE c.classe = d.classe';
      ELSE
        EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
                '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, '|| max_e - min_e ||'*(d.entrada-a.valor)/(b.valor-a.valor)+ '|| min_e ||', c.vector FROM (' || SQL || ') d, ' || 
                   nome_tabela || '_classes c, ' ||
                '  (select array( select min FROM ' || nome_tabela || '_entrada_col_st ) valor) A,' || 
                '  (select array( select max FROM ' || nome_tabela || '_entrada_col_st ) valor) B' || 
                '  WHERE c.classe = d.classe';
      END IF;          
    ELSE
      EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
              '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, d.entrada, c.vector FROM (' || SQL || ') d, ' || 
                 nome_tabela || '_classes c ' || 
              '  WHERE c.classe = d.classe';
    END IF;
  ELSIF possui_coluna( SQL, 'saida' ) THEN
    PERFORM cleanup_table( nome_tabela, '( id INTEGER PRIMARY KEY, fold INTEGER, entrada FLOAT8[], saida FLOAT8[] )' );
    
    IF min_e < max_e THEN
      PERFORM analyze_dense_matrix( 'SELECT id, entrada valor from ( ' || SQL || ' ) VV', nome_tabela || '_entrada' );
      PERFORM analyze_dense_matrix( 'SELECT id, saida valor from ( ' || SQL || ' ) VV', nome_tabela || '_saida' );

      EXECUTE 'UPDATE ' || nome_tabela || '_entrada_col_st SET ' ||
              '  min = min - ' || tol || ', ' ||
              '  max = max + ' || tol;

      EXECUTE 'UPDATE ' || nome_tabela || '_saida_col_st SET ' ||
              '  min = min - ' || tol || ', ' ||
              '  max = max + ' || tol;
    
      IF keep_col_ratio THEN
        EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
                '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, '|| max_e - min_e ||
                '*(d.entrada-a_e.valor)*(1.0/(b_e.valor-a_e.valor))+ '|| min_e ||', ' ||
                '  (d.saida-a_s.valor)*(1.0/(b_s.valor-a_s.valor)) FROM (' || SQL || ') d, ' || 
                '  (select min(min) valor FROM ' || nome_tabela || '_entrada_col_st ) a_e,' || 
                '  (select max(max) valor FROM ' || nome_tabela || '_entrada_col_st ) b_e,' || 
                '  (select min(min) valor FROM ' || nome_tabela || '_saida_col_st ) a_s,' || 
                '  (select max(max) valor FROM ' || nome_tabela || '_saida_col_st ) b_s';
      ELSE
        EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
                '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, '|| max_e - min_e ||'*(d.entrada-a_e.valor)/(b_e.valor-a_e.valor)+ '|| min_e ||', ' ||
                '  (d.saida-a_s.valor)/(b_s.valor-a_s.valor) FROM (' || SQL || ') d, ' || 
                '  (select array( select min FROM ' || nome_tabela || '_entrada_col_st ) valor) a_e,' || 
                '  (select array( select max FROM ' || nome_tabela || '_entrada_col_st ) valor) b_e,' || 
                '  (select array( select min FROM ' || nome_tabela || '_saida_col_st ) valor) a_s,' || 
                '  (select array( select max FROM ' || nome_tabela || '_saida_col_st ) valor) b_s';
      END IF;
    ELSE
      PERFORM analyze_dense_matrix( 'SELECT id, saida valor from ( ' || SQL || ' ) VV', nome_tabela || '_saida' );

      EXECUTE 'UPDATE ' || nome_tabela || '_saida_col_st SET ' ||
              '  min = min - ' || tol || ', ' ||
              '  max = max + ' || tol;

      EXECUTE 'INSERT INTO ' || nome_tabela || '( id, fold, entrada, saida ) ' ||
              '  SELECT d.id, (random()*' || k_folds || '+0.5)::integer, d.entrada, ' ||
              '  (d.saida-a_s.valor)*(1.0/(b_s.valor-a_s.valor)) FROM (' || SQL || ') d, ' || 
              '  (select min(min) valor FROM ' || nome_tabela || '_saida_col_st) A_S,' || 
              '  (select max(max) valor FROM ' || nome_tabela || '_saida_col_st) B_S';
    END IF;
  ELSE
    RAISE EXCEPTION 'A consulta deve ter os campos id, entrada, classe ou saida. [%]', sql;
  END IF;
  
  EXECUTE 'ANALYZE ' || nome_tabela;
  EXECUTE 'SELECT count( distinct fold ) FROM ' || nome_tabela INTO total;
  RETURN total;
END;
$$;


--
-- TOC entry 443 (class 1255 OID 28216)
-- Dependencies: 8 856 856 1097
-- Name: random_matrix(integer, integer, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION random_matrix(n_lines integer, n_cols integer, a double precision DEFAULT 0.0, b double precision DEFAULT 1.0, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql COST 10
    AS $$
BEGIN
  c.ptr := make_random_matrix( n_lines, n_cols, a, b );
END;
$$;


--
-- TOC entry 470 (class 1255 OID 198867)
-- Dependencies: 856 8 1097 856 856
-- Name: remove(matrix, integer[], integer[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove(a matrix, linhas integer[], colunas integer[], OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr, linhas, colunas );
END;
$$;


--
-- TOC entry 481 (class 1255 OID 205132)
-- Dependencies: 8 856 856 856 1097
-- Name: remove(matrix, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove(a matrix, linha integer, coluna integer, OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr, array[linha], array[coluna] );
END;
$$;


--
-- TOC entry 492 (class 1255 OID 213327)
-- Dependencies: 856 8 856 856 1097
-- Name: remove_coluna(matrix, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove_coluna(a matrix, coluna integer, OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr,  array[]::integer[], array[coluna]);
END;
$$;


--
-- TOC entry 490 (class 1255 OID 213329)
-- Dependencies: 856 8 1097 856 856
-- Name: remove_colunas(matrix, integer[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove_colunas(a matrix, colunas integer[], OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr, array[]::integer[], colunas );
END;
$$;


--
-- TOC entry 480 (class 1255 OID 213324)
-- Dependencies: 856 8 1097 856 856
-- Name: remove_linha(matrix, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove_linha(a matrix, linha integer, OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr, array[linha], array[]::integer[] );
END;
$$;


--
-- TOC entry 493 (class 1255 OID 213328)
-- Dependencies: 856 8 1097 856 856
-- Name: remove_linhas(matrix, integer[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION remove_linhas(a matrix, linhas integer[], OUT r matrix) RETURNS matrix
    LANGUAGE plpgsql STRICT COST 1
    AS $$
BEGIN
  r.ptr := pgm_matrix_remove_line_col( a.ptr, linhas, array[]::integer[] );
END;
$$;


--
-- TOC entry 491 (class 1255 OID 28217)
-- Dependencies: 1097 8
-- Name: run_kmeans(text, text, text, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION run_kmeans(sql text, nome_tabela_grupos text, nome_tabela_centroides text, k integer, seed integer DEFAULT 1, fim integer DEFAULT 1, repeticoes integer DEFAULT 10) RETURNS double precision
    LANGUAGE plpgsql
    AS $$
DECLARE
  matriz bigint;
  chaves bigint;
  centroides bigint;
  grupos bigint;
  sumdistance float8;
  centroides_loop bigint;
  grupos_loop bigint;
  sumdistance_loop float8;
  i integer;
  n_linhas INTEGER;
BEGIN
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto.
     -- EX1: 'select id, array[ x, y ] valor from tabela'
     -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
  -- 'nome_tabela_grupos' deve ser uma tabela nao existente ou existente com as colunas ( id integer primary key, grupo integer not null). Valores pre-existentes serao apagados.
  -- 'nome_tabela_centroides' deve ser uma tabela nao existente ou existente com as colunas ( grupo integer primary key, valor float8[] not null). Valores pre-existentes serao apagados.
  -- 'k': numero de grupos desejado
  -- 'seed': 1 para Random Centroides, 2 para Random Pontos
  -- 'fim': 1 para eliminar grupo sem elementos, 2 divide o maior grupo em 2 Grupos, 3 sinaliza Erro
  -- 'repeticoes': numero de vezes que o algoritmo sera executado para escolher o melhor resultado (menor soma das distancias dos pontos aos respectivos centroides)
  -- retorna o número de elementos em cada grupo.
  
  PERFORM cleanup_table( nome_tabela_grupos, '( ID INTEGER NOT NULL PRIMARY KEY, GRUPO INTEGER NOT NULL )' );
  PERFORM cleanup_table( nome_tabela_centroides, '( GRUPO INTEGER PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );

  SELECT * FROM make_matrix( sql ) INTO matriz, chaves;

  SELECT * FROM pgm_kmeans( matriz, k, seed, fim ) INTO centroides, grupos, sumdistance;
  
  FOR i IN 2 .. repeticoes LOOP
     SELECT * FROM pgm_kmeans( matriz, k, seed, fim ) INTO centroides_loop, grupos_loop, sumdistance_loop;
     IF sumdistance_loop < sumdistance THEN
        PERFORM pgm_vector_int_free( grupos );
        PERFORM pgm_matrix_free( centroides );
        centroides := centroides_loop;
        grupos := grupos_loop;
        sumdistance := sumdistance_loop;
     ELSE
        PERFORM pgm_vector_int_free( grupos_loop );
        PERFORM pgm_matrix_free( centroides_loop );        
     END IF;
   END LOOP;
     
  n_linhas := pgm_matrix_nlines( matriz );

  EXECUTE 'INSERT INTO ' || nome_tabela_grupos || '( ID, GRUPO ) ' ||
          'SELECT pgm_vector_int_get_elem( '|| chaves || ', i ), pgm_vector_int_get_elem( ' || grupos ||', i ) ' ||
          'from generate_series( 0, ' || n_linhas || ' - 1) i';    

  EXECUTE 'INSERT INTO ' || nome_tabela_centroides || '( GRUPO, VALOR ) ' ||
          'SELECT i, pgm_matrix_get_line( ' || centroides || ', i ) from generate_series( 0, ' || k || '-1 ) i';
    
  PERFORM pgm_vector_int_free( grupos );
  PERFORM pgm_matrix_free( centroides );
  
  PERFORM pgm_vector_int_free( chaves );
  PERFORM pgm_matrix_free( matriz );

  -- RETURN QUERY EXECUTE 'select grupo, count(*)::integer from ' || nome_tabela_grupos || ' group by grupo order by grupo';
  RETURN sumdistance;
END;
$$;


--
-- TOC entry 444 (class 1255 OID 28218)
-- Dependencies: 1097 8
-- Name: run_lsvd(text, text, integer, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION run_lsvd(sql text, nome_base_tabela text, numfeatures integer, analisa_erro boolean DEFAULT false) RETURNS SETOF double precision
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- SVD: faz a decomposicao M = U S Vt
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto.
     -- EX1: 'select id, array[ x, y ] valor from tabela'
     -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
     -- Essa string usará make_matrix, e terá como id as ids originais da query. Já a tabela s e vt terão ids sequenciais de 1 a N
  -- 'nome_base_tabela' sera usado como prefixo das tabelas tabela_u, tabela_s, tabela_vt. Valores pre-existentes serao apagados.
  -- 'numfeatures': numero de fatores do SVD.
  -- retorna os valores singulares (tabela_s).

  rec RECORD;
  matrix bigint;
  keys bigint;
  UT bigint;
  U bigint;
  S bigint;
  VT bigint;
  E bigint;
BEGIN  
  rec := make_matrix( sql );
  matrix := rec.matrix;
  keys := rec.keys;

  rec := pgm_lsvd( matrix, numfeatures );
  U := rec.u;
  S := rec.s;
  VT := rec.vt;
  
  PERFORM matrix2table_transpose( keys, U, nome_base_tabela || '_u' );
  PERFORM matrix2table_transpose( VT, nome_base_tabela || '_vt' );
  PERFORM vector_double2table( S, nome_base_tabela || '_s' );

  if analisa_erro then
    UT := pgm_matrix_transpose( U );
    E := pgm_svd_error_features( matrix, ut, s, vt, numfeatures );

    PERFORM vector_double2table( E, nome_base_tabela || '_err' );
    EXECUTE 'ANALYZE ' || nome_base_tabela || '_err';
    PERFORM pgm_matrix_free( UT );
    PERFORM pgm_vector_double_free( E );
  end if;

  EXECUTE 'ANALYZE ' || nome_base_tabela || '_u';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_vt';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_s';

  PERFORM pgm_matrix_free( matrix );
  PERFORM pgm_matrix_free( keys );
  PERFORM pgm_matrix_free( U );
  PERFORM pgm_vector_double_free( S );
  PERFORM pgm_matrix_free( VT );

  RETURN QUERY EXECUTE 'SELECT VALOR FROM ' || nome_base_tabela || '_s ORDER BY ID';
END;
$$;


--
-- TOC entry 448 (class 1255 OID 28219)
-- Dependencies: 1097 8
-- Name: run_pca(text, text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION run_pca(sql text, nome_base_tabela text, numfeatures integer) RETURNS SETOF double precision
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- PCA: cria a tabela com a transformacao PCA de numFeatures
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto.
     -- EX1: 'select id, array[ x, y ] valor from tabela'
     -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
     -- Essa string usará make_matrix, e terá como id as ids originais da query. Já as tabela s, v e vt terão ids sequenciais de 1 a N
  -- 'nome_base_tabela' sera usado como prefixo das tabelas tabela_pca, tabela_u, tabela_s, tabela_vt, tabela_v. Valores pre-existentes serao apagados.
  -- 'numfeatures': numero de fatores do PCA.
  -- retorna os valores singulares (tabela_s).
  -- Cria as tabelas <nome_base_tabela> _pca, _features, _u, _v, _vt, _s

  rec RECORD;
  matrix bigint;
  keys bigint;
  UT bigint;
  S bigint;
  VT bigint;
  zeroMeanSql text;
BEGIN  
  PERFORM analyze_dense_matrix( sql, nome_base_tabela );

  zeroMeanSql := 'SELECT X.ID, (X.VALOR - MEDIA.VALOR) valor FROM (' || sql || ') X, (SELECT ARRAY_AGG( VV.AVG ) VALOR FROM (SELECT AVG FROM ' || nome_base_tabela || '_COL_ST ORDER BY ID) vv) MEDIA';
  rec := make_matrix( zeroMeanSql );
  matrix := rec.matrix;
  keys := rec.keys;

  rec := pgm_svd( matrix, numfeatures );
  UT := rec.ut;
  S := rec.s;
  VT := rec.vt;
  
  PERFORM matrix2table_transpose( keys, UT, nome_base_tabela || '_u' );
  PERFORM matrix2table_transpose( VT, nome_base_tabela || '_vt' );
  PERFORM matrix2table( VT, nome_base_tabela || '_v' );
  PERFORM vector_double2table( S, nome_base_tabela || '_s' );

  PERFORM cleanup_table( nome_base_tabela || '_features', '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );
  EXECUTE 'INSERT INTO ' || nome_base_tabela || '_features( id, valor ) SELECT ID, VALOR * pgm_vector_double_get( ' || S || ') FROM ' || nome_base_tabela || '_u ORDER BY ID'; 

  PERFORM cleanup_table( nome_base_tabela || '_pca', '( ID INTEGER NOT NULL PRIMARY KEY, VALOR FLOAT8[] NOT NULL )' );


  EXECUTE 'INSERT INTO ' || nome_base_tabela || '_pca( id, valor ) ' ||
          'SELECT A.ID, ARRAY_AGG( SUM( a.valor * b.valor ) ) valor FROM (' || sql || ') A, ' || nome_base_tabela || '_v B GROUP BY A.id ORDER BY A.id';

  EXECUTE 'ANALYZE ' || nome_base_tabela || '_u';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_vt';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_v';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_s';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_features';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_pca';

  PERFORM pgm_matrix_free( matrix );
  PERFORM pgm_matrix_free( keys );
  PERFORM pgm_matrix_free( UT );
  PERFORM pgm_vector_double_free( S );
  PERFORM pgm_matrix_free( VT );

  RETURN QUERY EXECUTE 'SELECT VALOR FROM ' || nome_base_tabela || '_s ORDER BY ID';
END;
$$;


--
-- TOC entry 449 (class 1255 OID 28220)
-- Dependencies: 1097 8
-- Name: run_svd(text, text, integer, boolean, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION run_svd(sql text, nome_base_tabela_in text DEFAULT ''::text, numfeatures integer DEFAULT 2, analisa_erro boolean DEFAULT false, reconstroi_matriz boolean DEFAULT true) RETURNS SETOF double precision
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- SVD: faz a decomposicao M = U S Vt
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto.
     -- EX1: 'select id, array[ x, y ] valor from tabela'
     -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
     -- Essa string usará make_matrix, e terá como id as ids originais da query. Já a tabela s e vt terão ids sequenciais de 1 a N
  -- 'nome_base_tabela' sera usado como prefixo das tabelas tabela_u, tabela_s, tabela_vt. Valores pre-existentes serao apagados.
  -- 'numfeatures': numero de fatores do SVD.
  -- retorna os valores singulares (tabela_s).

  rec RECORD;
  nome_base_tabela text;
  matrix bigint;
  keys bigint;
  UT bigint;
  S bigint;
  VT bigint;
  E bigint;
  mrt bigint;
BEGIN  
  nome_base_tabela := case when nome_base_tabela_in <> '' then nome_base_tabela_in else sql end;
  
  rec := make_matrix( sql );
  matrix := rec.matrix;
  keys := rec.keys;

  rec := pgm_svd( matrix, numfeatures );
  UT := rec.ut;
  S := rec.s;
  VT := rec.vt;
  
  PERFORM matrix2table_transpose( keys, UT, nome_base_tabela || '_u' );
  PERFORM matrix2table_transpose( VT, nome_base_tabela || '_vt' );
  PERFORM vector_double2table( S, nome_base_tabela || '_s' );

  if reconstroi_matriz then
    mrt := pgm_svd_matrix_reconstruct( ut, s, vt, numfeatures );
    PERFORM matrix2table( mrt, nome_base_tabela || '_rt' );
    EXECUTE 'ANALYZE ' || nome_base_tabela || '_rt';
    --PERFORM pgm_matrix_free( mrt );
  end if;

  if analisa_erro then
    E := pgm_svd_error_features( matrix, ut, s, vt, pgm_vector_double_nlines( S ) );
    PERFORM vector_double2table( E, nome_base_tabela || '_err' );
    EXECUTE 'ANALYZE ' || nome_base_tabela || '_err';
    --PERFORM pgm_vector_double_free( E );
  end if;

  EXECUTE 'ANALYZE ' || nome_base_tabela || '_u';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_vt';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_s';

--  PERFORM pgm_matrix_free( matrix );
--  PERFORM pgm_matrix_free( keys );
--  PERFORM pgm_matrix_free( UT );
--  PERFORM pgm_vector_double_free( S );
--  PERFORM pgm_matrix_free( VT );

  RETURN QUERY EXECUTE 'SELECT VALOR FROM ' || nome_base_tabela || '_s ORDER BY ID';
END;
$$;


--
-- TOC entry 450 (class 1255 OID 28221)
-- Dependencies: 1097 8
-- Name: run_svd(bigint, text, integer, boolean, boolean); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION run_svd(matrix bigint, nome_base_tabela_in text, numfeatures integer DEFAULT 2, analisa_erro boolean DEFAULT false, reconstroi_matriz boolean DEFAULT true) RETURNS SETOF double precision
    LANGUAGE plpgsql
    AS $$
DECLARE
  -- SVD: faz a decomposicao M = U S Vt
  -- 'sql' deve ser uma string retornando (id,valor), onde "id integer primary key" e valor é um array float8[] com os valores do ponto.
     -- EX1: 'select id, array[ x, y ] valor from tabela'
     -- EX2: 'select cast( row_number() over() as integer ) id, array[ x, y ] valor from tabela'
     -- Essa string usará make_matrix, e terá como id as ids originais da query. Já a tabela s e vt terão ids sequenciais de 1 a N
  -- 'nome_base_tabela' sera usado como prefixo das tabelas tabela_u, tabela_s, tabela_vt. Valores pre-existentes serao apagados.
  -- 'numfeatures': numero de fatores do SVD.
  -- retorna os valores singulares (tabela_s).
  nome_base_tabela text;
  matrix bigint;
  keys bigint;
  UT bigint;
  S bigint;
  VT bigint;
  E bigint;
  mrt bigint;
  rec record;
BEGIN  
  nome_base_tabela := case when nome_base_tabela_in <> '' then nome_base_tabela_in else sql end;

  rec := pgm_svd( matrix, numfeatures );
  UT := rec.ut;
  S := rec.s;
  VT := rec.vt;
  
  PERFORM matrix2table_transpose( keys, UT, nome_base_tabela || '_u' );
  PERFORM matrix2table_transpose( VT, nome_base_tabela || '_vt' );
  PERFORM vector_double2table( S, nome_base_tabela || '_s' );

  if analisa_erro then
    E := pgm_svd_error_features( matrix, ut, s, vt, pgm_vector_double_nlines( S ) );

    PERFORM vector_double2table( E, nome_base_tabela || '_err' );
    EXECUTE 'ANALYZE ' || nome_base_tabela || '_err';
    PERFORM pgm_vector_double_free( E );
  end if;

  if reconstroi_matriz then
    PERFORM matrix( pgm_svd_matrix_reconstruct( ut, s, vt, numfeatures ) ) ==> (nome_base_tabela || '_rt');
    EXECUTE 'ANALYZE ' || nome_base_tabela || '_rt';
    PERFORM pgm_vector_double_free( mrt );
  end if;

  EXECUTE 'ANALYZE ' || nome_base_tabela || '_u';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_vt';
  EXECUTE 'ANALYZE ' || nome_base_tabela || '_s';

  PERFORM pgm_matrix_free( UT );
  PERFORM pgm_vector_double_free( S );
  PERFORM pgm_matrix_free( VT );

  RETURN QUERY EXECUTE 'SELECT VALOR FROM ' || nome_base_tabela || '_s ORDER BY ID';
END;
$$;


--
-- TOC entry 508 (class 1255 OID 123240)
-- Dependencies: 8
-- Name: sen(double precision[], double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sen(a double precision[], b double precision[]) RETURNS double precision
    LANGUAGE c STRICT
    AS '$libdir/pgminer.so', '_sen';


--
-- TOC entry 520 (class 1255 OID 28222)
-- Dependencies: 8 1097
-- Name: sigmoid(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sigmoid(a double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  --raise info '%', a;
  RETURN array_agg( CASE WHEN x >= 50 THEN 1.0::float8
                         WHEN x <= -50 THEN 0.0::float8
                         ELSE 1.0/(1.0 + exp( -2.0*x ) ) 
                    END
                  ) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 451 (class 1255 OID 28223)
-- Dependencies: 8 1097
-- Name: sigmoid(double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sigmoid(a double precision) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN 1.0/(1.0 + exp( -2.0*a ) );
END
$$;


--
-- TOC entry 515 (class 1255 OID 282787)
-- Dependencies: 8 856 856 856 856 1097
-- Name: split_folds(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION split_folds(table_data text, test_fold text, OUT entrada_treinamento matrix, OUT saida_treinamento matrix, OUT entrada_teste matrix, OUT saida_teste matrix) RETURNS record
    LANGUAGE plpgsql STRICT COST 10
    AS $$
-- table_data deve ser uma tabela preparada por prepare_data_to_learn, tendo as colunas:
--   id, fold, entrada, saida, com entrada e saida normalizadas.
DECLARE
BEGIN
  entrada_treinamento := matrix( 'select id, entrada valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  saida_treinamento := matrix( 'select id, saida valor from ' || table_data || ' where fold not in (' || test_fold || ')' );
  entrada_teste := matrix( 'select id, entrada valor from ' || table_data || ' where fold in (' || test_fold || ')' );
  saida_teste := matrix( 'select id, saida valor from ' || table_data || ' where fold in (' || test_fold || ')' );
END;
$$;


--
-- TOC entry 452 (class 1255 OID 28224)
-- Dependencies: 8
-- Name: sqr(double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sqr(a double precision) RETURNS double precision
    LANGUAGE sql IMMUTABLE STRICT COST 1
    AS $_$
 SELECT $1*$1;
$_$;


--
-- TOC entry 453 (class 1255 OID 28225)
-- Dependencies: 1097 8
-- Name: sqrt(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sqrt(a double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN array_agg( sqrt( x ) ) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 454 (class 1255 OID 28226)
-- Dependencies: 1097 8
-- Name: sum(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sum(a double precision[]) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN sum( x ) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 455 (class 1255 OID 28227)
-- Dependencies: 8 1097
-- Name: sumsq(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sumsq(a double precision[]) RETURNS double precision
    LANGUAGE plpgsql IMMUTABLE STRICT COST 1
    AS $$
BEGIN
  RETURN sum( x*x ) from ( select unnest( a ) x ) c;
END
$$;


--
-- TOC entry 456 (class 1255 OID 28228)
-- Dependencies: 8 1097 866 866
-- Name: svd_record(bigint, bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION svd_record(utptr bigint, sptr bigint, vtptr bigint, OUT c svd_record) RETURNS svd_record
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
BEGIN
  c.Ut := UtPtr;
  c.S := SPtr;
  c.Vt := VtPtr;
END;
$$;


--
-- TOC entry 457 (class 1255 OID 28229)
-- Dependencies: 856 8 1097 856 856
-- Name: t(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION t(m matrix, OUT c matrix) RETURNS matrix
    LANGUAGE plpgsql
    AS $$
BEGIN
  c.ptr := pgm_matrix_transpose( m.ptr );
END;
$$;


--
-- TOC entry 458 (class 1255 OID 28230)
-- Dependencies: 8 1097
-- Name: table_or_query(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION table_or_query(sql text) RETURNS text
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
DECLARE
  -- torna 'sql' apropriado para ser usado como subquery ou table.
BEGIN
  if position( 'FROM ' in upper( sql ) ) > 0 THEN
    return ' ( ' || sql || ' ) AAA ';   
  else
    return sql;
  end if;
END;
$$;


--
-- TOC entry 460 (class 1255 OID 28231)
-- Dependencies: 8 1097
-- Name: test_elm(text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION test_elm(table_data text, neurons integer, OUT missclassified_train double precision, OUT missclassified_test double precision) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$ 
DECLARE
  elm MemNN;
BEGIN
  elm := (create_elm( table_data, neurons )).nn;

  execute 'select count(*) from ' || table_data || ' where fold not in ( ' || 1 || ') and cos( (entrada || 1::float8) * memnn(' ||
          elm.pesos_a || ',' || elm.pesos_b  || '), saida )  > 0.07071' into missclassified_train;
--  execute 'select count(*) from ' || table_data || ' where fold in ( ' || 1 || ') and saida * elm > 0.07071' into missclassified_test;
END;
$$;


--
-- TOC entry 461 (class 1255 OID 28232)
-- Dependencies: 8 1097 856
-- Name: toarray(matrix); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION toarray(m matrix) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE
    AS $$
BEGIN
  return pgm_matrix2Array( m.ptr );
END;
$$;


--
-- TOC entry 462 (class 1255 OID 28233)
-- Dependencies: 8 1097 869
-- Name: toarray(vector); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION toarray(v vector) RETURNS double precision[]
    LANGUAGE plpgsql IMMUTABLE
    AS $$
BEGIN
  return pgm_vector_double2array( v.ptr );
END;
$$;


--
-- TOC entry 474 (class 1255 OID 372293)
-- Dependencies: 8
-- Name: tokens(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION tokens(text) RETURNS cstring[]
    LANGUAGE sql
    AS $_$
    SELECT pgm_tsvector2words(to_tsvector($1));
$_$;


--
-- TOC entry 526 (class 1255 OID 372340)
-- Dependencies: 8
-- Name: tokens_frequency(text, cstring[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION tokens_frequency(text, cstring[]) RETURNS bigint
    LANGUAGE sql
    AS $_$
    SELECT frequency from pgm_create_vector_presence_frequency(to_tsvector($1),$2,2);
$_$;


--
-- TOC entry 528 (class 1255 OID 372339)
-- Dependencies: 8
-- Name: tokens_presence(text, cstring[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION tokens_presence(text, cstring[]) RETURNS bigint
    LANGUAGE sql
    AS $_$
    SELECT presence from pgm_create_vector_presence_frequency(to_tsvector($1),$2,1);
$_$;


--
-- TOC entry 529 (class 1255 OID 372343)
-- Dependencies: 8
-- Name: tokens_presence_frequency(text, cstring[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION tokens_presence_frequency(text, cstring[], OUT presence bigint, OUT frenquecy bigint) RETURNS record
    LANGUAGE sql
    AS $_$
    SELECT * from pgm_create_vector_presence_frequency(to_tsvector($1),$2,3);
$_$;


--
-- TOC entry 463 (class 1255 OID 28234)
-- Dependencies: 8 1097
-- Name: unitary_vector(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION unitary_vector(dimension integer, n_dimensions integer, OUT vector double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql STABLE COST 1
    AS $$
BEGIN
  vector := array_fill( 0.001, array[ n_dimensions ] );
  vector[dimension] := 0.999;
  RETURN;
END;
$$;


--
-- TOC entry 464 (class 1255 OID 28235)
-- Dependencies: 8 1097
-- Name: unitary_vector(integer, integer, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION unitary_vector(dimension integer, n_dimensions integer, precisao double precision, OUT vector double precision[]) RETURNS double precision[]
    LANGUAGE plpgsql STABLE COST 1
    AS $$
-- Precisao: valor entre 0.0 e 0.1 indicando qual a precisao do vetor gerado.
-- Exemplo: se precisão for 0.0, o vetor será gerado como ( 1, 0, 0, ... )
-- Se precisão for 0.05 ==> ( 0.95, 0.05, 0.05, ... )
DECLARE
  prec float8;
BEGIN
  IF precisao > 0.1 OR precisao < 0.0 THEN
    prec := 0.001;
  ELSE
    prec := precisao;
  END IF;
  vector := array_fill( prec, array[ n_dimensions ] );
  vector[dimension] := 1.0 - prec;

  RETURN;
END;
$$;


--
-- TOC entry 465 (class 1255 OID 28236)
-- Dependencies: 1097 8 869 869
-- Name: vector(double precision[]); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector(unidimensional_array double precision[], OUT c vector) RETURNS vector
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
BEGIN
  c.ptr := pgm_array2vector_double( unidimensional_array );
END;
$$;


--
-- TOC entry 473 (class 1255 OID 28237)
-- Dependencies: 869 8 869 1097
-- Name: vector(bigint); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector(pgm_vector_ptr bigint, OUT c vector) RETURNS vector
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
BEGIN
  c.ptr := pgm_vector_ptr;
END;
$$;


--
-- TOC entry 466 (class 1255 OID 28238)
-- Dependencies: 869 1097 8 869
-- Name: vector(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector(sql_in text, OUT c vector) RETURNS vector
    LANGUAGE plpgsql IMMUTABLE COST 1
    AS $$
declare
-- 'sql' deve ser uma string retornando valor::float8 ou uma tabela/view com essa coluna
-- EX1: 'select x valor from tabela order by id'
  sql text;
  nlinhas integer;
  rec record;
  i integer;
BEGIN
  sql := table_or_query( sql_in );
  execute 'SELECT count(*) FROM ' || sql into nlinhas;

  c.ptr := pgm_vector_double_create( nlinhas ); 
  i := 0;
  for rec in execute 'SELECT valor FROM ' || sql loop
    PERFORM pgm_vector_double_set_elem( c.ptr, i, rec.valor );
    i := i + 1;
  end loop;

END;
$$;


--
-- TOC entry 467 (class 1255 OID 28239)
-- Dependencies: 8 1097 869
-- Name: vector2table(vector, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector2table(v vector, table_name text, OUT nlines integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
BEGIN
  nlines := vector_double2table( v.ptr, table_name );
END;
$$;


--
-- TOC entry 468 (class 1255 OID 28240)
-- Dependencies: 8 1097
-- Name: vector_double2table(bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector_double2table(vector_double bigint, table_name text, OUT nlines integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR DOUBLE PRECISION NOT NULL )' );

  nlines := pgm_vector_double_nlines( vector_double );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT i+1, pgm_vector_double_get_elem( '|| vector_double || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 469 (class 1255 OID 28241)
-- Dependencies: 8 1097
-- Name: vector_int2table(bigint, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION vector_int2table(vector_int bigint, table_name text, OUT nlines integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
BEGIN
  PERFORM cleanup_table( table_name, '( ID INTEGER NOT NULL PRIMARY KEY, VALOR INTEGER NOT NULL )' );

  nlines := pgm_vector_int_nlines( vector_int );

  EXECUTE 'INSERT INTO ' || table_name || '( ID, VALOR ) ' ||
          'SELECT i+1, pgm_vector_int_get_elem( '|| vector_int || ', i ) '
          'from generate_series( 0, ' || nlines || ' - 1 ) i';  
END;
$$;


--
-- TOC entry 522 (class 1255 OID 394340)
-- Dependencies: 8
-- Name: writematrix(bigint, bigint); Type: FUNCTION; Schema: public; Owner: -
--



--
-- TOC entry 1805 (class 2617 OID 28242)
-- Dependencies: 8 411
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = pgm_scale_vector,
    LEFTARG = double precision,
    RIGHTARG = double precision[],
    COMMUTATOR = *
);


--
-- TOC entry 1804 (class 2617 OID 28243)
-- Dependencies: 8 412
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = pgm_scale_vector,
    LEFTARG = double precision[],
    RIGHTARG = double precision,
    COMMUTATOR = *
);


--
-- TOC entry 1806 (class 2617 OID 28244)
-- Dependencies: 436 8
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = pgm_vector_vector,
    LEFTARG = double precision[],
    RIGHTARG = double precision[],
    COMMUTATOR = *
);


--
-- TOC entry 1807 (class 2617 OID 28245)
-- Dependencies: 8 862 386
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = pgm_nn_evaluate,
    LEFTARG = neuralnet,
    RIGHTARG = double precision[]
);


--
-- TOC entry 1808 (class 2617 OID 28246)
-- Dependencies: 8 387 862
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = pgm_nn_evaluate,
    LEFTARG = double precision[],
    RIGHTARG = neuralnet
);


--
-- TOC entry 1809 (class 2617 OID 28247)
-- Dependencies: 8 341 856 856 856
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_matrix_multiply,
    LEFTARG = matrix,
    RIGHTARG = matrix
);


--
-- TOC entry 1810 (class 2617 OID 28248)
-- Dependencies: 869 8 869 856 350
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_vector_matrix_multiply,
    LEFTARG = vector,
    RIGHTARG = matrix
);


--
-- TOC entry 1811 (class 2617 OID 28249)
-- Dependencies: 336 8 856
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_array_matrix_multiply,
    LEFTARG = double precision[],
    RIGHTARG = matrix
);


--
-- TOC entry 1812 (class 2617 OID 28250)
-- Dependencies: 859 856 8 342 856
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_memnn_evaluate_matrix,
    LEFTARG = matrix,
    RIGHTARG = memnn
);


--
-- TOC entry 1813 (class 2617 OID 28251)
-- Dependencies: 8 859 523
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_array_memnn,
    LEFTARG = double precision[],
    RIGHTARG = memnn
);


--
-- TOC entry 1814 (class 2617 OID 28252)
-- Dependencies: 8 856 856 340
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_matrix_double_multiply,
    LEFTARG = matrix,
    RIGHTARG = double precision
);


--
-- TOC entry 1828 (class 2617 OID 103767)
-- Dependencies: 856 856 8 498 930
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_memelmcos_evaluate_matrix,
    LEFTARG = matrix,
    RIGHTARG = memelmcos
);


--
-- TOC entry 1829 (class 2617 OID 282937)
-- Dependencies: 518 8 1010
-- Name: *; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR * (
    PROCEDURE = opr_fann_apply,
    LEFTARG = double precision[],
    RIGHTARG = fann
);


--
-- TOC entry 1815 (class 2617 OID 28253)
-- Dependencies: 348 8
-- Name: +; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR + (
    PROCEDURE = opr_sum_vector_float,
    LEFTARG = double precision[],
    RIGHTARG = double precision
);


--
-- TOC entry 1816 (class 2617 OID 28254)
-- Dependencies: 8 347
-- Name: +; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR + (
    PROCEDURE = opr_sum_float_vector,
    LEFTARG = double precision,
    RIGHTARG = double precision[]
);


--
-- TOC entry 1817 (class 2617 OID 28255)
-- Dependencies: 8 349
-- Name: +; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR + (
    PROCEDURE = opr_sum_vector_vector,
    LEFTARG = double precision[],
    RIGHTARG = double precision[]
);


--
-- TOC entry 1818 (class 2617 OID 28256)
-- Dependencies: 856 339 856 856 8
-- Name: +; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR + (
    PROCEDURE = opr_matrix_add,
    LEFTARG = matrix,
    RIGHTARG = matrix
);


--
-- TOC entry 1819 (class 2617 OID 28257)
-- Dependencies: 418 8
-- Name: -; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR - (
    PROCEDURE = pgm_vector_distance,
    LEFTARG = double precision[],
    RIGHTARG = double precision[],
    COMMUTATOR = -
);


--
-- TOC entry 1820 (class 2617 OID 28258)
-- Dependencies: 8 345
-- Name: -; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR - (
    PROCEDURE = opr_sub_vector_float,
    LEFTARG = double precision[],
    RIGHTARG = double precision
);


--
-- TOC entry 1821 (class 2617 OID 28259)
-- Dependencies: 344 8
-- Name: -; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR - (
    PROCEDURE = opr_sub_float_vector,
    LEFTARG = double precision,
    RIGHTARG = double precision[]
);


--
-- TOC entry 1822 (class 2617 OID 28260)
-- Dependencies: 856 321 8 856 856
-- Name: -; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR - (
    PROCEDURE = opr_matrix_subtract,
    LEFTARG = matrix,
    RIGHTARG = matrix
);


--
-- TOC entry 1823 (class 2617 OID 28261)
-- Dependencies: 338 8
-- Name: /; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR / (
    PROCEDURE = opr_div_vector_vector,
    LEFTARG = double precision[],
    RIGHTARG = double precision[]
);


--
-- TOC entry 1824 (class 2617 OID 28262)
-- Dependencies: 8 337
-- Name: /; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR / (
    PROCEDURE = opr_div_float_vector,
    LEFTARG = double precision,
    RIGHTARG = double precision[]
);


--
-- TOC entry 1825 (class 2617 OID 28263)
-- Dependencies: 862 8 862 343
-- Name: /; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR / (
    PROCEDURE = opr_nn_split_part,
    LEFTARG = neuralnet,
    RIGHTARG = integer
);


--
-- TOC entry 1826 (class 2617 OID 28264)
-- Dependencies: 319 8 856
-- Name: ==>; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR ==> (
    PROCEDURE = matrix2table,
    LEFTARG = matrix,
    RIGHTARG = text
);


--
-- TOC entry 1827 (class 2617 OID 28265)
-- Dependencies: 8 869 467
-- Name: ==>; Type: OPERATOR; Schema: public; Owner: -
--

CREATE OPERATOR ==> (
    PROCEDURE = vector2table,
    LEFTARG = vector,
    RIGHTARG = text
);

CREATE OR REPLACE FUNCTION pgm_array2matrix_int(value integer[])
  RETURNS bigint AS
'$libdir/pgminer.so', 'pgm_arraytype2Matrix_Int'
  LANGUAGE c VOLATILE STRICT
  COST 1;

CREATE OR REPLACE FUNCTION pgm_matrix_int2array(matrix bigint)
  RETURNS integer[] AS
'$libdir/pgminer.so', 'pgm_matrix_int2arraytype'
  LANGUAGE c VOLATILE STRICT
  COST 1;


CREATE OR REPLACE FUNCTION pgm_som(data bigint, neuronios bigint,Dstart integer,iter_start integer, eta_start double precision,Dend integer, iter_end integer, eta_end float8, conscience boolean,out neuronios bigint, out cluster bigint )
  RETURNS record AS
'$libdir/pgminer.so', 'pgm_som'
  LANGUAGE c VOLATILE STRICT
  COST 1;