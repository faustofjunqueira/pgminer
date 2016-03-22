begin

select pgm_array2matrix(array[[2,0],[0,-3],[0,0]])
select ut , s , vt from pgm_svd(pgm_array2matrix(array[[1,2],[3,4]]),2);

select u, s, vt from pgm_lsvd(pgm_array2matrix(array[[2,0],[0,-3],[0,0]]),2);


--                      140567270405672(matrix);                        140567270405704;(vetor)                   140567270405736(matrix)
select pgm_matrix2array(140567270405672); select pgm_vectordouble2array(140567270405704)  select pgm_matrix2array(140567270405736);

select matrix2array();

commit


