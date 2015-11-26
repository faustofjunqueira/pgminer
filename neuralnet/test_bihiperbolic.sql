create temporary table serie ( a serial, b int);

insert into serie(b) select null from iris a,iris b,iris c limit 1000;

alter table serie drop column b;

select * from serie;

create temporary table dom(x double precision);

insert into dom select a*(0.01) from serie;

select x,(select funcao from pgm_bi_hiperbolic(x)) as funcao,(select derived from pgm_bi_hiperbolic(x)) derived from dom;
