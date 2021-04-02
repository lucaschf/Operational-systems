#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#define FALSE 0
#define TRUE 1

#define ERRO_ABRIR_ARQUIVO 2
#define ERRO_CRIAR_ARQUIVO 3
#define ERRO_FECHAR_ARQUIVO 4
#define ERRO_ESCREVER_ARQUIVO 5
#define ERRO_EXCLUIR_ARQUIVO 6
#define ERRO_LER_ARQUIVO 7
#define FIM_DE_ARQUIVO 8

#define NUM_MAXIMO_DISCOS 50
#define CAPACIDADE_DISCO 1048576 // bytes - 1MB
#define PARAM_LENGTH 300
#define MAX_PARAMS 50

#define CRIAR_DISCO "cd"
#define CRIAR_ARQUIVO "ca"
#define ABRIR_ARQUIVO "aa"
#define ESCREVER_ARQUIVO "ea"
#define LER_ARQUIVO "la"
#define FECHAR_ARQUIVO "fa"
#define EXCLUIR_ARQUIVO "da"
#define LISTAR_DISCO "ld"
#define FORMATAR_DISCO "fd"

// Número máximo de caracteres no nome do arquivo.
#define TAMANHO_NOME_ARQUIVO 13

// Comprimento da hora de criação do arquivo no formato hh:mm:ss.
#define TAMANHO_HORA 9

// Tamanho do bloco lógico em bytes.
#define TAMANHO_BLOCO_LOGICO 4

// Número máximo de arquivos do disco virtual.
#define NUMERO_DE_ARQUIVOS_DO_DISCO 255

// Representação descritiva dos tipos de dados que um arquivo pode armazenar.
#define TIPO_NUMERICO "Numérico"
#define TIPO_TEXTO "Texto"

/**
 * Representa os tipos de dados que um arquivo pode armazenar.
 * Os tipos são numérico (int ou float) e string.
*/
typedef enum {
    NUMERICO,
    STRING
} TIPO_DE_ARQUIVO;

/**
 * Representa os estddos do arquivo.
*/
typedef enum {
    ABERTO,
    FECHADO
} EstadoArquivo;

// Representa os atributos de um arquivo.
typedef struct {

    // Descritor de arquivo usado para identificar um arquivo na tabela de alocação de arquivos do disco.
    unsigned short fd;

    // Nome e hora de criação do arquivo no formato hh:mm:ss.
    char nome[TAMANHO_NOME_ARQUIVO];
    char horaCriacao[TAMANHO_HORA];

    // Tamanho do arquivo em bytes.
    unsigned int tamanho;

    // Número de blocos lógicos do arquivo.
    unsigned int blocosLogicos;

    // Armazena o conteúdo do arquivo. O tipo de dado do conteúdo é definido pelo campo tipo.
    void *conteudo;

    // Representa o tipo de dado que o arquivo pode armazenar.
    TIPO_DE_ARQUIVO tipo;

    EstadoArquivo estado;
} Arquivo;

// Representa os atributos de um disco virtual.
typedef struct {

    // Letra de identificação da unidade de disco.
    char unidade;

    // Capacidade de armazenamento do disco.
    unsigned int capacidade;

    // Espaço ocupado do disco.
    unsigned int espacoOcupado;

    // Espaço livre do disco.
    unsigned int espacoLivre;

    // Número total de blocos lógicos.
    unsigned int blocosLogicosTotais;

    // Número de blocos lógicos ocupados do disco.
    unsigned int blocosLogicosOcupados;

    // Número de blocos lógicos livres do disco.
    unsigned int blocosLogicosLivres;

    // A tabela de alocação de arquivos é usada para armazenar todos os arquivos do disco.
    // A posição de um arquivo nessa tabela é igual ao valor inteiro do seu descritor de arquivo
    Arquivo *tabelaArquivos[NUMERO_DE_ARQUIVOS_DO_DISCO];

    unsigned short arquivosEmDisco;
} Disco;

/**
 * Cria um disco virtual.
 * A capacidade especifica a capacidade máxima de armazenamento em bytes do disco, ou seja, o seu tamanho "físico".
 * O parâmetro unidade especifica um caractere usado para identificação do disco, por exemplo C, D, E, etc.
 * Em caso de sucesso, retorna um ponteiro para estrutura do disco criada, caso contrário, NULL.
*/
Disco *criaDisco(char unidade, unsigned int capacidade);

/**
 * Busca um disco a partir de sua unidade.
 *
 * @param unidade unidade buscada
 * @return ponteiro do disco se encontrado ou NULL se nao encontrado
 */
Disco *buscaPorUnidade(char unidade);

/**
 * Formata logicamente o disco virtual apagando todo o conteúdo de sua tabela de alocação de arquivos,
 * consequentemente apagando todo os arquivos do disco.O parâmetro unidade especifica um caractere usado para
 * identificação do disco, por exemplo C, D, E, etc. Reinicializa os dados da estrutura do disco em caso de sucesso
 * e retorna TRUE, caso contrário, FALSE. Todas as memórias alocadas dinamicamente para armazenar a estrutura e o conteúdo
 * de cada arquivo devem ser devolvidas ao sistema operacional.
*/
int formatar(char unidade, Disco *disco);

/**
 * Cria um arquivo para leitura e escrita no disco identificado pela unidade.
 * Retorna um inteiro que representa o descritor de arquivo ou ERRO_CRIAR_ARQUIVO se o arquivo não pode ser criado.
*/
int criar(char unidade, const char *nomeArquivo);

unsigned short geraFileDescriptor(Disco *disco);

/**
 * Retorna a posicao do arquivo na tabela baseado em seu descritor
 *
 * @param fd descritor do arquivo
 *
 * @return posicao do arquivo;
 */
unsigned short calculateFilePosition(unsigned short fd);

/**
 * Busca um arquivo em um disco baseando-se no nome
 *
 * @param d disco onde supostamente o arquivo esta localizado
 * @param nomeArquivo arquivo buscado
 * @return o ponteiro do arquivo encontrado, NULL se nao encontrado
 */
Arquivo *buscaArquivo(const Disco *d, const char *nomeArquivo);

/**
 * Abre um arquivo para leitura e escrita localizado no disco identificado pela unidade.
 * Retorna TRUE se o arquivo foi aberto e ERRO_ABRIR_ARQUIVO se ocorrer algum erro, por exemplo, o arquivo não existe.
*/
int abrir(char unidade, const char *nomeArquivo);

/**
 * Escreve no arquivo n elementos obtidos do buffer. O valor de n é definido por tamanho. O arquivo é identificado
 * pelo descritor de arquivo fd e está localizado no disco identificado pela unidade. Retorna o número de bytes escritos
 * após uma escrita bem sucedida. Se ocorrer algum erro na escrita retorna ERRO_ESCREVER_ARQUIVO.
 *
 * NOTA: O bloco lógico é a unidade básica de alocação de espaço (memória) no disco, portanto sempre que uma escrita é
 * feita no disco deve-se escrever no mínimo um bloco lógico de tamanho definido em TAMANHO_BLOCO_LOGICO.
 * Sendo assim, quando o usuário solicita que seja escrito um número inteiro de 4 bytes (valor de TAMANHO_BLOCO_LOGICO),
 * usa-se um bloco lógico para armazená-lo no arquivo, mas quando é escrito a string "Férias!", usa-se 2 blocos lógicos.
 * Como essa string possui 7 caracteres e cada caractere (char) ocupa 1 byte de memória, ela precisa de 7 bytes de memória.
 * No entanto, como cada bloco lógico possui 4 bytes, a string exige 2 blocos lógicos (8 bytes) para ser armazenada no disco,
 * logo deve-se escrever dois blocos.
*/
int escrever(char unidade, unsigned short fd, const void *buffer, unsigned tamanho);

/**
 * Lê n elementos do arquivo e armazena no buffer. O valor de n é definido por tamanho.
 * O arquivo é identificado pelo descritor de arquivo fd e está localizado no disco identificado pela unidade.
 * Retorna o número de elementos lidos do arquivo em uma leitura bem sucedida. Retorna FIM_DE_ARQUIVO se o fim do
 * arquivo for atingido após a leitura e ERRO_LER_ARQUIVO se ocorrer algum erro na leitura.
 *
 * NOTA: O bloco lógico é a  unidade básica de alocação de espaço (memória) no disco, portanto sempre que uma leitura é feita no disco deve-se
 * ler no mínimo um bloco lógico de tamanho definido em TAMANHO_BLOCO_LOGICO. Sendo assim, quando o usuário solicita
 * que seja lido um número inteiro de 4 bytes (valor de TAMANHO_BLOCO_LOGICO), lê-se um bloco lógico para recuperá-lo
 * do arquivo, mas quando é lido a string "Férias!", lê-se 2 blocos lógicos. Como essa string possui 7/ 9caracteres e
 * cada caractere (char) ocupa 1 byte de memória, ela precisa de 7 bytes de memória.
 * No entanto, como cada bloco lógico possui 4 bytes, a string exige 2 blocos lógicos (8 bytes) para ser armazenada no disco,
 * logo deve-se ler os dois blocos.
*/
int ler(char unidade, unsigned short fd, void *buffer, unsigned tamanho);

/**
 * Fecha um arquivo (descritor de arquivo fd) que está localizado no disco identificado pela unidade. Retorna TRUE se a
 * operação for bem sucedida e ERRO_FECHAR_ARQUIVO se não conseguir fechar o arquivo.
*/
int fechar(char unidade, unsigned short fd);

/**
 * Apaga um arquivo do disco identificado por unidade.
 * Retorna TRUE se a operação for bem sucedida e ERRO_EXCLUIR_ARQUIVO se não conseguir excluir o arquivo do disco virtual.
*/
int excluir(char unidade, const char *nomeArquivo);

/**
 * Exibe um relatório com as seguintes informações para cada arquivo do disco identificado por unidade:
 * 1. Nome;
 * 2. Tipo;
 * 3. Tamanho em bytes;
 * 4. Número de blocos lógicos;
 * 5. Hora de criação.
 * No fim da listagem deve exibir também os seguintes dados do disco virtual:
 * 1. Número de arquivos;
 * 2. A capacidade de armazenamento;
 * 3. O espaço ocupado do disco;
 * 4. O espaço livre do disco;
 * 5. O número total de blocos lógicos;
 * 6. O número de blocos lógicos ocupados;
 * 7. O número de blocos lógicos livres.
 * Os valores dos itens 5 a 7 devem ser exibidos em bytes.
 * Retorna o endereço de memória que possui o conteúdo do relatório. Se ocorrer um erro ao obter esses dados retorna NULL.
*/
char *infoDisco(char unidade);

int tokenize(char *source, char destdest[][PARAM_LENGTH],unsigned int expectedTokens, char *delimiter);

void execute(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

int isOnlyNumbers(char items[], unsigned int size);

void executaCriacaoDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void executaCriacaoArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void executaEscritaArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void executaLeituraArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void manipulaEstadoAberturaArquivo(int argc, EstadoArquivo estado, const char args[MAX_PARAMS][PARAM_LENGTH]);

void executaExclusaoArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void executaFormatacaoDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);

void listaDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]);
#endif