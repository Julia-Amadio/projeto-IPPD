# Guia de Implementação e Documentação - Otimização OpenMP

## 1. Corrigir o Makefile (Obrigatório)
O Makefile atual não ativa OpenMP. Sem a opção correta, as diretivas #pragma omp serão ignoradas e o programa executará sequencialmente.

Se usar GCC:
CC=gcc
FLAGS=-O2 -fopenmp
LDLIBS=-lm

Também é importante declarar a dependência do código-fonte:
$(EXEC): $(EXEC).c

Explicação:
A flag -fopenmp habilita o reconhecimento das diretivas OpenMP e liga a biblioteca responsável pela criação e gerenciamento das threads. Também deve ser verificado qual compilador estará instalado na máquina de avaliação. O Makefile atual utiliza pgcc, que normalmente usa outra opção para OpenMP, como -mp.

---

## 2. Manter uma Versão Sequencial para Comparação (Obrigatório)
O arquivo old_polygon_cut.c pode servir como referência sequencial. É importante compilar as duas versões com níveis de otimização equivalentes:

gcc -O2 old_polygon_cut.c -lm -o polygon_cut_seq
gcc -O2 -fopenmp polygon_cut.c -lm -o polygon_cut

Explicação:
A versão sequencial funciona como linha de base. O ganho do paralelismo deve ser medido comparando a versão OpenMP com o algoritmo original usando a mesma entrada e otimizações equivalentes. É importante não comparar uma versão com -O2 contra outra sem otimização.

---

## 3. Criar Entradas de Teste (Obrigatório)
Preparar casos que cubram:
* Caso mínimo: N = 2, ou seja, quatro vértices.
* Exemplo oficial do enunciado.
* Polígonos pequenos, para conferir manualmente.
* Polígonos médios, para testes rápidos.
* Polígonos grandes, para medir desempenho.
* Entrada contendo vários polígonos, para confirmar a ordem das respostas.
* Caso máximo: N = 500, ou seja, 1.000 vértices, se o tempo da máquina permitir.

Os polígonos devem ser convexos e os vértices devem estar em ordem anti-horária.

Explicação:
Casos pequenos verificam a correção. Casos grandes são necessários para que o custo de criar e sincronizar threads seja pequeno em comparação ao trabalho realizado.

---

## 4. Automatizar a Comparação das Saídas (Obrigatório)
Executar as duas versões sobre exatamente a mesma entrada:

./polygon_cut_seq < entrada.txt > saida_seq.txt
OMP_NUM_THREADS=4 ./polygon_cut < entrada.txt > saida_omp.txt
diff saida_seq.txt saida_omp.txt

O diff não deve apresentar nenhuma diferença. Repetir a versão OpenMP com diferentes quantidades de threads:

OMP_NUM_THREADS=1 ./polygon_cut
OMP_NUM_THREADS=2 ./polygon_cut
OMP_NUM_THREADS=4 ./polygon_cut
OMP_NUM_THREADS=8 ./polygon_cut

Explicação:
A quantidade de threads deve alterar apenas o tempo de execução, nunca o resultado. Saídas iguais com diferentes quantidades de threads ajudam a demonstrar que não há condição de corrida aparente.

---

## 5. Medir o Tempo de Execução (Obrigatório)
Medir as versões sequencial e paralela com a mesma entrada:

/usr/bin/time -f '%e' ./polygon_cut_seq < entrada.txt
/usr/bin/time -f '%e' env OMP_NUM_THREADS=4 ./polygon_cut < entrada.txt

O ideal é:
* Fazer uma execução inicial de aquecimento.
* Executar cada configuração pelo menos cinco vezes.
* Usar a mediana dos tempos.
* Não executar outros programas pesados durante o experimento.
* Usar a mesma máquina para todas as medições.

Explicação:
Uma única medição pode sofrer interferência do sistema operacional. Repetições e mediana produzem um resultado mais confiável.

---

## 6. Calcular Speedup e Eficiência (Obrigatório)
O speedup é calculado como:
speedup = tempo sequencial / tempo paralelo

Exemplo:
* Sequencial: 8 segundos
* 4 threads: 2,5 segundos
* Speedup = 8 / 2,5 = 3,2

A eficiência paralela é calculada como:
eficiencia = speedup / numero de threads

No exemplo:
* eficiencia = 3,2 / 4 = 0,8 = 80%

Tabela de referência para preenchimento:

| Threads | Tempo | Speedup | Eficiência |
|---------|-------|---------|------------|
| 1       | —     | —       | —          |
| 2       | —     | —       | —          |
| 4       | —     | —       | —          |
| 8       | —     | —       | —          |

Explicação:
Speedup mostra quantas vezes o programa ficou mais rápido. Eficiência mostra quanto da capacidade teórica das threads foi aproveitada.

---

## 7. Documentar a Transformação da Recursão (Obrigatório)
Explicar que a função recursiva process() foi substituída por process_all().

Explicação:
A recursão com memoização dificultava a paralelização porque duas threads poderiam chegar ao mesmo subproblema. A versão bottom-up organiza os cálculos por tamanho de intervalo, deixando explícita a ordem das dependências. Também vale mencionar que a fórmula matemática não foi alterada; apenas a ordem de cálculo mudou.

---

## 8. Documentar o Papel de Span (Obrigatório)
Este é um dos pontos mais importantes do relatório.

Explicação:
span representa o tamanho do intervalo do polígono que está sendo resolvido. Ele permanece sequencial porque um intervalo maior depende dos resultados de intervalos menores. Portanto, span = 5 só pode começar depois que span = 3 estiver concluído. Os valores avançam de dois em dois (span = 3, 5, 7, ...) porque os subpolígonos válidos possuem a paridade necessária para a decomposição em quadriláteros.

---

## 9. Documentar a Paralelização do Laço de A (Obrigatório)
Explicação:
Para um mesmo span, cada valor de a calcula uma célula diferente m[a][b]. Essas células dependem apenas de intervalos menores, que já foram concluídos. Por isso, os valores de a podem ser divididos entre as threads.

A diretiva utilizada é:
#pragma omp for schedule(static)

---

## 10. Explicar a Barreira Implícita (Obrigatório)
Explicação:
Ao final de omp for, o OpenMP cria uma barreira automática. Todas as threads precisam terminar o span atual antes de avançarem para o próximo. Isso preserva as dependências da programação dinâmica. Não se deve adicionar nowait nesse laço, pois isso permitiria que algumas threads avançassem enquanto outras ainda estivessem calculando dependências.

---

## 11. Explicar a Região Paralela Persistente (Relevante)
O código cria a equipe de threads apenas uma vez:

#pragma omp parallel
{
    for (span = ...) {
        #pragma omp for
    }
}

Explicação:
Criar e encerrar threads em cada span adicionaria overhead. A região paralela externa permite reutilizar a mesma equipe durante todo o cálculo.

---

## 12. Explicar as Variáveis Privadas (Obrigatório)
As variáveis auxiliares estão na cláusula:
private(span, b, i_offset, j_offset, i, j, temp)

Explicação:
Essas variáveis representam o estado temporário de cada cálculo. Cada thread precisa de sua própria cópia para que uma não sobrescreva os índices e resultados temporários da outra. Também deve ser explicado que isso não duplica subproblemas: o omp for atribui cada valor de a a apenas uma thread.

---

## 13. Explicar por que não foi usado Mutex (Relevante)
Explicação:
Não foi necessário mutex porque o algoritmo foi organizado para que cada thread escreva em uma célula diferente. Mutex serializaria parte do cálculo e adicionaria custo de bloqueio. A organização por span elimina o conflito antes que ele aconteça.

---

## 14. Documentar schedule(static) (Relevante)
Explicação:
Para um mesmo span, todos os valores de a percorrem a mesma quantidade de combinações de i e j. Como a carga é aproximadamente uniforme, schedule(static) distribui o trabalho com pouco overhead. Não há necessidade inicial de dynamic, porque o gerenciamento dinâmico custaria mais e a carga já é equilibrada.

---

## 15. Documentar a Inicialização Paralela (Obrigatório)
Na função init():
#pragma omp parallel for collapse(2) schedule(static)

Explicação:
Cada par (i, j) escreve exclusivamente em d[i][j] e m[i][j]. Como uma iteração não depende da outra, as duas dimensões podem ser distribuídas entre as threads. 

Sobre collapse(2): trata os dois laços como um único conjunto maior de iterações, aumentando a quantidade de trabalho disponível para distribuição.

---

## 16. Documentar a Redução Final (Obrigatório)
Na busca pelo menor resultado:
#pragma omp parallel for private(temp) reduction(min:smaller)

Explicação:
Cada thread encontra um mínimo local. Ao final, o OpenMP combina os mínimos locais e armazena o menor deles em smaller. Isso evita atualizar uma variável global com mutex ou seção crítica.

---

## 17. Analisar a Escalabilidade (Obrigatório no Relatório)
Os resultados provavelmente não crescerão linearmente indefinidamente. Pontos para comentar:
* Overhead de criação e sincronização das threads.
* Barreiras entre valores de span.
* Limite de núcleos físicos.
* Competição pela memória e pelo cache.
* Trechos sequenciais do programa.
* Polígonos pequenos podem ficar mais lentos com muitas threads.
* Acima do número de núcleos físicos, mais threads podem não ajudar.

Explicação:
O paralelismo beneficia principalmente entradas grandes. Em entradas pequenas, o trabalho economizado pode ser menor que o custo de coordenar as threads.

---

## 18. Registrar a Complexidade (Relevante)
Sendo V o número total de vértices:
* Memória: O(V2)
* Número de estados: O(V2)
* Cada estado testa combinações de i e j.
* Trabalho total aproximado: O(V4)

Explicação:
A complexidade elevada produz bastante trabalho computacional, tornando o problema adequado para paralelização. O OpenMP não altera a complexidade assintótica, mas divide parte do trabalho entre os núcleos.

---

## 19. Fazer Pequenos Ajustes de Qualidade no Código (Recomendado)
Pontos de melhoria sem alterar o algoritmo:
* Verificar o retorno do scanf das coordenadas.
* Usar int main(void) em vez de int main().
* Padronizar comentários e indentação.
* Evitar comentários excessivamente informais.
* Considerar constantes ou funções para o valor usado como "infinito".

Compilar com avisos:
-Wall -Wextra -Wpedantic

O aviso atual do scanf deve ser resolvido antes da entrega para deixar a compilação limpa.

---

## 20. Verificar os Requisitos Exatos da Entrega (Obrigatório)
O enunciado da maratona menciona:
* Código-fonte.
* Makefile.
* Script de execução.
* Leitura pela entrada padrão.
* Escrita pela saída padrão.
* Preservação da ordem dos resultados.

Confirmar com a disciplina:
* Nome exigido para o executável.
* Formato do relatório.
* Necessidade de script de execução.
* Compilador disponível.
* Número de threads permitido.
* Arquivos que devem estar no pacote final.

---

## Ordem Recomendada dos Próximos Passos
1. Corrigir o Makefile e habilitar OpenMP.
2. Corrigir o aviso de scanf.
3. Criar entradas pequenas, médias e grandes.
4. Comparar automaticamente as saídas.
5. Medir tempos com diferentes quantidades de threads.
6. Calcular speedup e eficiência.
7. Produzir tabela e, se necessário, gráfico.
8. Escrever a documentação seguindo os tópicos acima.
9. Testar o pacote final em um diretório limpo.
10. Conferir nomes e formato exigidos para entrega.