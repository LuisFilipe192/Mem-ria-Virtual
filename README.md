# Memória Virtual

## Descrição do projeto

A Memória Virtual é um projeto desenvolvido em C que tem como objetivo simular um gerenciador de memória virtual, traduzindo endereços lógicos em endereços físicos, com base no projeto *Designing a Virtual Memory Manager* do livro *Operating System Concepts* (Silberschatz et al., 10ª edição).

Cada endereço lógico de 16 bits é dividido em um número de página e um deslocamento. O programa consulta primeiro a TLB e, em caso de falha, a tabela de páginas. Quando a página não está na memória, ocorre um page fault, e a página é carregada do arquivo `BACKING_STORE.bin`.

A memória física possui apenas 128 frames, preenchidos do 0 ao 127. Quando a memória está cheia, um algoritmo de substituição escolhe qual frame será atualizado. O programa implementa os algoritmos FIFO e LRU tanto para a substituição de páginas quanto para a substituição da TLB, e a busca na TLB é feita com uma thread para cada entrada.

O resultado de cada tradução e o resumo da execução são gravados no arquivo `correct.txt`.

O projeto foi desenvolvido em C para ambiente Linux, utilizando o Ubuntu por meio do WSL.

## Estrutura dos arquivos

O projeto está organizado da seguinte forma:

- `vm.c` — contém a validação das entradas, a tradução dos endereços, a tabela de páginas, a TLB, os algoritmos de substituição e a geração do arquivo de saída.
- `Makefile` — utilizado para compilar o programa e remover os arquivos gerados.
- `README.md` — documentação do projeto.
- `evidencias.log` — registro dos testes executados durante o desenvolvimento.

Os arquivos `addresses.txt` e `BACKING_STORE.bin` são fornecidos pelo professor, não fazem parte do repositório e devem ser colocados no mesmo diretório do executável.

## Como compilar

Para compilar o projeto, basta executar o comando:

```bash
make
```

O comando gera o executável:

```
vm
```

Para remover o executável e o arquivo de saída gerado, utilize:

```bash
make clean
```

## Como executar

Após compilar o projeto, execute o programa informando o arquivo de endereços, o algoritmo de substituição de página e o algoritmo de substituição da TLB:

```bash
./vm addresses.txt lru fifo
```

Nesse exemplo, a substituição de página utiliza o LRU e a da TLB utiliza o FIFO. Os algoritmos aceitos são `fifo` e `lru`.

O arquivo de endereços deve conter um endereço lógico por linha, com valores inteiros entre 0 e 65535.

## Funcionamento da memória virtual

### Tradução dos endereços

Cada endereço lógico é dividido em duas partes: os 8 bits mais altos indicam o número da página, e os 8 bits mais baixos indicam o deslocamento dentro dela. O endereço físico é calculado como `frame * 256 + deslocamento`, e o valor é o byte armazenado nessa posição da memória física.

### Tabela de páginas e page faults

A função `get_frame()` consulta a tabela de páginas. Se a página não estiver na memória, ocorre um page fault: a página é lida do `BACKING_STORE.bin` pela função `load_page()` e colocada no próximo frame livre. Quando os 128 frames estão ocupados, a função `select_victim()` escolhe o frame que será substituído, e a página que estava nele é removida da tabela de páginas e da TLB.

### Substituição de páginas

- **FIFO** — substitui o frame carregado há mais tempo, por meio de um ponteiro circular que percorre os frames de 0 a 127.
- **LRU** — substitui o frame que está há mais tempo sem ser acessado. O último uso de cada frame é atualizado em todos os acessos, inclusive nos que acertam na TLB.

### TLB

A TLB possui 16 entradas. A função `tlb_lookup()` procura a página na TLB e, em caso de acerto, o frame é obtido diretamente dela. Em caso de falha, a tradução segue para a tabela de páginas, e o par página e frame é inserido na TLB pela função `tlb_insert()`.

- **FIFO** — utiliza um ponteiro circular que percorre as entradas de 0 a 15.
- **LRU** — utiliza uma entrada vazia, se houver; caso contrário, substitui a entrada usada há mais tempo.

O campo `TLB` do arquivo de saída indica a posição da TLB utilizada: a posição em que a página foi encontrada ou aquela em que foi inserida.

### Busca na TLB com threads

A busca na TLB cria uma thread para cada uma das 16 entradas. Cada thread compara a página procurada com a página da sua entrada e registra o resultado em uma estrutura própria, sem compartilhar variáveis de escrita com as outras threads. Após aguardar todas com `pthread_join()`, o programa verifica qual thread encontrou a página.

## Tratamento de erros

O programa valida todas as entradas antes de traduzir os endereços. Quando encontra um erro, exibe uma mensagem no terminal e encerra com código de saída 1, sem gerar o arquivo de saída.

São tratados os seguintes casos:

- Quantidade de argumentos diferente de três.
- Algoritmo de substituição de página ou da TLB diferente de `fifo` ou `lru`.
- Arquivo de endereços inexistente ou que não pode ser aberto.
- Linhas com formatação inválida, como letras, valores negativos, valores maiores que 65535 ou mais de um endereço na mesma linha, indicando o número da linha.
- Arquivo de endereços sem nenhum endereço.
- Arquivo `BACKING_STORE.bin` inexistente ou com tamanho diferente de 65536 bytes.

Linhas em branco e espaços no início ou no fim de cada linha são ignorados, e quebras de linha no padrão do Windows (\r\n) são aceitas.

## Testes realizados

Durante o desenvolvimento, o programa foi compilado e executado utilizando os arquivos fornecidos pelo professor.

Foram realizados testes para verificar:

- A tradução dos endereços com 256 frames, sem substituição, resultando em 244 page faults.
- A substituição de páginas com 128 frames utilizando FIFO e LRU.
- A TLB com substituição FIFO e LRU.
- A busca na TLB com uma thread por entrada.
- O tratamento de argumentos inválidos, arquivos inexistentes e linhas mal formatadas.

O arquivo `correct.txt` gerado com `./vm addresses.txt lru fifo` foi comparado com a saída do professor utilizando o comando `diff`, sem apresentar diferenças. Os resultados obtidos para cada combinação de algoritmos foram:

| Página | TLB | Page Faults | TLB Hits |
|---|---|---|---|
| fifo | fifo | 538 | 54 |
| fifo | lru | 538 | 55 |
| lru | fifo | 539 | 54 |
| lru | lru | 539 | 55 |

## Sistema operacional utilizado

O projeto foi desenvolvido e testado no seguinte ambiente:

- Sistema operacional: Ubuntu Linux via WSL.
- Linguagem: C.
- Compilador: GCC.
- Biblioteca de threads: pthread.
- Ferramenta de compilação: Make.