# Casos de teste

Arquivos de entrada prontos pra testar o `polygon_cut` (e comparar com `old_polygon_cut.c`).

## Como compilar

    gcc -O2 -fopenmp ../polygon_cut.c -o polygon_cut -lm
    gcc -O2 ../old_polygon_cut.c -o polygon_cut_seq -lm

(troque -fopenmp por -mp se o compilador for pgcc/nvc)

## Como rodar um caso

    ./polygon_cut < tests/input_minimo.txt

## Como comparar as duas versoes (correcao)

    ./polygon_cut_seq < tests/input_grande.txt > saida_seq.txt
    ./polygon_cut < tests/input_grande.txt > saida_omp.txt
    diff saida_seq.txt saida_omp.txt

## Os arquivos

| Arquivo               | Cobre                          | Vertices | Saida esperada              |
|------------------------|--------------------------------|----------|------------------------------|
| input_minimo.txt       | caso minimo (N=2)              | 4        | 0.0000                       |
| input_enunciado.txt    | exemplo oficial do enunciado   | 8 + 4    | 4519.6176 / 0.0000           |
| input_pequeno.txt      | pequeno, conferivel na mao     | 6        | 20.0000                      |
| input_medio.txt        | medio, testes rapidos          | 60       | 60379.4192                   |
| input_grande.txt       | grande, medir desempenho       | 400      | 103754.4025                  |
| input_multiplos.txt    | varios poligonos numa entrada  | -        | 0.0000 / 20.0000 / 60379.4192|
| input_maximo.txt       | caso maximo (N=500)            | 1000     | 123863.0481 (so validado na versao paralela ate agora) |

## Gerar novos casos

    python3 tests/gen_polygon.py 100 > tests/input_novo.txt

Gera um poligono convexo regular com 2*N vertices (N = argumento), em ordem anti-horaria,
com coordenadas dentro do limite do enunciado (0 a 10000).

**Atencao com input_maximo.txt**: por causa da complexidade O(V^4), a versao
sequencial pode levar varios minutos nesse caso. A versao paralela leva ~70s.
