﻿CREATE OR REPLACE FUNCTION dfg_calculatePACC(vectorP bigint, vectorE bigint ) RETURNS double precision
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_calculatePACC_wrapper';

CREATE OR REPLACE FUNCTION dfg_calculateCBD(vectorP bigint, vectorE bigint ) RETURNS double precision
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_calculateCBD_wrapper';

CREATE OR REPLACE FUNCTION dfg_createProfilesMatrix(inputs bigint, mapI bigint, mapJ bigint ) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_createProfilesMatrix_wrapper';

CREATE OR REPLACE FUNCTION dfg_createPCorrectMatrix(inputs bigint, targets bigint, mapI bigint, mapJ bigint ) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_createPCorrectMatrix_wrapper';

CREATE OR REPLACE FUNCTION dfg_findKNearests(eps double precision, numNearests integer, coordinates bigint, coordinates_query bigint) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_findKNearests_wrapper';

CREATE OR REPLACE FUNCTION dfg_recommend(knearests bigint, pCorrect bigint, inputs bigint, mapJ bigint ) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_recommend_wrapper';

CREATE OR REPLACE FUNCTION dfg_buildCoordinatesQuery(coordinates bigint, queryInputs bigint, mapI bigint ) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_buildCoordinatesQuery_wrapper';

CREATE OR REPLACE FUNCTION dfg_findKNearests2(numNearests integer, coordinates bigint, coordinates_query bigint) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_findKNearests2_wrapper';

CREATE OR REPLACE FUNCTION dfg_confusionmat( targets bigint, predictions bigint) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_confusionmat_wrapper';

CREATE OR REPLACE FUNCTION dfg_createMapping( inputs bigint, id integer ) RETURNS bigint
    LANGUAGE c VOLATILE AS '$libdir/pgminer.so', 'dfg_createMapping_wrapper';


CREATE OR REPLACE FUNCTION dfg_matrix(IN sql_in text, OUT c matrix) RETURNS matrix AS
$BODY$
DECLARE
-- Cria um objeto matrix a partir de uma query sql
-- 'sql' Deve ser uma string retornando valor::float8 ou uma tabela/view com essa coluna
-- EX1: 'select ARRAY[x, y, z] valor from tabela'
  sql text;
  nlinhas integer;
  ncolunas integer;
  rec record;
  i integer;
BEGIN
  sql := table_or_query( sql_in );
  execute 'SELECT count(*) FROM ' || sql into nlinhas;
  execute 'SELECT array_length(valor, 1) FROM ' || sql || ' LIMIT 1' into ncolunas;
  c.ptr := pgm_matrix_create( nlinhas, ncolunas );
  
  i := 0;
  for rec in execute 'SELECT valor FROM ' || sql loop
    PERFORM pgm_matrix_set_line( c.ptr, i, rec.valor );
    i := i + 1;
  end loop;
END;
$BODY$
LANGUAGE plpgsql VOLATILE COST 1;




CREATE OR REPLACE FUNCTION dfg_vector(IN sql_in text, OUT c vector) RETURNS vector AS
$BODY$
DECLARE
-- Cria um vetor a partir de uma query
-- 'sql' deve ser uma string retornando valor::float8 ou uma tabela/view com essa coluna
-- EX1: 'select id as valor from tabela order by id'
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
$BODY$
LANGUAGE plpgsql VOLATILE COST 1;


drop function dfg_grockit(IN k integer, IN impl integer, OUT pAcc double precision, OUT pCBD double precision, OUT confusion matrix, OUT testPredictions vector, OUT testTargets vector)

CREATE OR REPLACE FUNCTION dfg_grockit(IN k integer, IN impl integer, OUT pAcc double precision, OUT pCBD double precision, OUT confusion matrix, OUT testPredictions vector, OUT testTargets vector) RETURNS record AS
$BODY$
DECLARE
-- Executa o algoritmo
    trainInputs matrix;
    trainTargets vector;
    testInputs matrix;
    --testTargets vector;
    --testPredictions vector;
    
    mapI vector;
    mapJ vector;
    pCorrect matrix;
    knearests matrix;
    coordinates matrix;
    coordinatesQuery matrix;
    
BEGIN

    RAISE NOTICE 'Loading train data...';
    trainInputs := dfg_matrix( 'select array[ user_id, question_id ] valor from dados_treinamento' );
    trainTargets := dfg_vector( E'select case when correct=\'t\' then 1 else 0 end valor from dados_treinamento' );
    --trainInputs := dfg_matrix( E'select array[ user_id, question_id ] valor from dados where tipo=\'TR\'' );
    --trainTargets := dfg_vector( E'select case when correct=\'t\' then 1 else 0 end valor from dados where tipo=\'TR\'' );
    
    RAISE NOTICE 'Creating mappings...';
    mapI.ptr := dfg_createMapping( trainInputs.ptr, 0 );
    mapJ.ptr := dfg_createMapping( trainInputs.ptr, 1 );
    
    RAISE NOTICE 'Creating profiles matrix...';
    coordinates.ptr := dfg_createProfilesMatrix( trainInputs.ptr, mapI.ptr, mapJ.ptr );

    RAISE NOTICE 'Creating pCorrect matrix...';
    pCorrect.ptr := dfg_createPCorrectMatrix( trainInputs.ptr, trainTargets.ptr, mapI.ptr, mapJ.ptr );

    RAISE NOTICE 'Loading test data...';
    testInputs := dfg_matrix( 'select array[ user_id, question_id ] valor from dados_validacao' );
    testTargets := dfg_vector( E'select case when correct=\'t\' then 1 else 0 end valor from dados_validacao' );
    --testInputs := dfg_matrix( E'select array[ user_id, question_id ] valor from dados where tipo=\'TE\'' );
    --testTargets := dfg_vector( E'select case when correct=\'t\' then 1 else 0 end valor from dados where tipo=\'TE\'' );

    RAISE NOTICE 'Finding knearests...';
    coordinatesQuery.ptr := dfg_buildCoordinatesQuery( coordinates.ptr, testInputs.ptr, mapI.ptr );
    IF impl=1 THEN
        knearests.ptr := dfg_findKNearests( 0.0, k, coordinates.ptr, coordinatesQuery.ptr );
    ELSE
        knearests.ptr := dfg_findKNearests2( k, coordinates.ptr, coordinatesQuery.ptr );
    END IF;

    RAISE NOTICE 'Predicting...';
    testPredictions.ptr := dfg_recommend( knearests.ptr, pCorrect.ptr, testInputs.ptr, mapJ.ptr );
    
    RAISE NOTICE 'Evaluating...';
    confusion.ptr := dfg_confusionmat( testTargets.ptr, testPredictions.ptr );
    pAcc := dfg_calculatePACC( testPredictions.ptr, testTargets.ptr );
    pCBD := dfg_calculateCBD( testPredictions.ptr, testTargets.ptr );

END;
$BODY$
LANGUAGE plpgsql VOLATILE COST 1;

-- Para rodar
SELECT * from dfg_grockit(1000, 1);

