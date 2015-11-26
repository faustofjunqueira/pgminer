
begin

select make_random_matrix(1000,5000)

select blas_multiply(140322315590720,140322315590752);

select pgm_matrix_multiply(140322315590720,140322315590752);

commit;

/*
[1000x1000]*[1000x1000]
BLAS: Total query runtime: 2916 ms.clock: 2860000
 PGM: Total query runtime: 8074 ms.clock: 8010000


*/