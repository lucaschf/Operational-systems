#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include "filesystem.h"

#define INCORRECT_SYNTAX "Sintaxe incorreta"
#define UNITY_TOO_LONG "Identifiador de unidade muito longo"
#define THERE_IS_ALREADY_A_DISK_WITH_THIS_UNITY_NAME "Ja existe um disco com essa unidade"
#define DISK_SUCCESSFUL_CREATED "Disco criado com sucesso"
#define DISK_CREATION_FAILURE "Falha ao criar disco"
#define FILE_NAME_TO_LONG "Nome do arquivo muito longo"
#define THERE_IS_ALREADY_A_FILE_WITH_THIS_NAME "Já existe um arquivo em disco com este nome"
#define FILE_CREATION_FAILURE "Falha ao criar arquivo"
#define FILE_SUCCESSFUL_CREATED "Arquivo criado com sucesso"
#define TOO_MANY_PARAMETERS "Instrucao com muitos parametros"

Disco *discos[NUM_MAXIMO_DISCOS];
int numeroDiscos = 0;

unsigned short verificaTipoArquivo(TIPO_DE_ARQUIVO *tipoDeArquivo, const void *pVoid);

unsigned int computeOccupiedSpace(Disco *d, Arquivo *arq, unsigned int numeroRegistros);

size_t calculateBlocks(unsigned int bufferSize);

time_t getCurrentTime() {
    time_t rawTime;
    return time(&rawTime);
}

void geraHora(char strHora[]) {
    time_t tm = getCurrentTime();
    struct tm *hora = localtime(&tm);
    sprintf(strHora, "%02d:%02d:%02d", hora->tm_hour, hora->tm_min, hora->tm_sec);
}

Disco *criaDisco(char unidade, unsigned int capacidade) {
    if (buscaPorUnidade(unidade))
        return NULL;

    Disco *disco = (Disco *) malloc(sizeof(Disco));

    if (disco) {
        disco->unidade = unidade;
        disco->capacidade = capacidade;
        disco->espacoOcupado = 0;
        disco->espacoLivre = capacidade;
        disco->blocosLogicosTotais = capacidade / TAMANHO_BLOCO_LOGICO;
        disco->blocosLogicosLivres = disco->blocosLogicosTotais;
        disco->blocosLogicosOcupados = 0;
        disco->arquivosEmDisco = 0;

        for (int i = 0; i < NUMERO_DE_ARQUIVOS_DO_DISCO; i++) {
            disco->tabelaArquivos[i] = NULL;
        }
    }

    return disco;
}

Disco *buscaPorUnidade(char unidade) {
    Disco *disco;

    for (int i = 0; i < numeroDiscos; i++) {
        disco = discos[i];

        if (disco->unidade == unidade)
            return disco;
    }

    return NULL;
}

int formatar(char unidade, Disco *disco) {

    if (!disco)
        return FALSE;

    if (disco->unidade != unidade && buscaPorUnidade(unidade))
        return FALSE;

    disco->unidade = unidade;
    disco->espacoOcupado = 0;
    disco->espacoLivre = disco->capacidade;
    disco->blocosLogicosOcupados = 0;
    disco->blocosLogicosLivres = disco->blocosLogicosTotais;
    disco->arquivosEmDisco = 0;

    for (int i = 0; i < NUMERO_DE_ARQUIVOS_DO_DISCO; i++) {
        free(disco->tabelaArquivos[i]);
        disco->tabelaArquivos[i] = NULL;
    }

    return TRUE;
}

int criar(char unidade, const char *nomeArquivo) {

    Disco *d = buscaPorUnidade(unidade);

    if (!d || (d->capacidade == d->espacoOcupado) || (d->arquivosEmDisco == NUMERO_DE_ARQUIVOS_DO_DISCO) ||
        buscaArquivo(d, nomeArquivo))
        return ERRO_CRIAR_ARQUIVO;

    Arquivo *arq = malloc(sizeof(Arquivo));
    if (arq) {
        arq->estado = FECHADO;
        strcpy(arq->nome, nomeArquivo);
        arq->tamanho = 0;
        arq->blocosLogicos = 0;
        arq->conteudo = NULL;
        arq->fd = geraFileDescriptor(d);

        geraHora(arq->horaCriacao);
        d->arquivosEmDisco++;
        d->tabelaArquivos[calculateFilePosition(arq->fd)] = arq;

        return arq->fd;
    }

    return ERRO_CRIAR_ARQUIVO;
}

unsigned short geraFileDescriptor(Disco *disco) {
    unsigned short fd;
    fd = disco->arquivosEmDisco + NUMERO_DE_ARQUIVOS_DO_DISCO;

    while (disco->tabelaArquivos[fd % NUMERO_DE_ARQUIVOS_DO_DISCO] != NULL)
        fd++;

    return fd;
}

unsigned short calculateFilePosition(unsigned short fd) {
    return fd % NUMERO_DE_ARQUIVOS_DO_DISCO;
}

Arquivo *buscaArquivo(const Disco *d, const char *nomeArquivo) {

    if (!d || !nomeArquivo)
        return NULL;

    Arquivo *arquivo;

    for (int i = 0; i < d->arquivosEmDisco; i++) {
        arquivo = d->tabelaArquivos[i];
        if (!strcmp(nomeArquivo, arquivo->nome))
            return arquivo;
    }

    return NULL;
}

int abrir(char unidade, const char *nomeArquivo) {
    Arquivo *target = buscaArquivo(buscaPorUnidade(unidade), nomeArquivo);

    if (!target)
        ERRO_ABRIR_ARQUIVO;

    target->estado = ABERTO;
    return TRUE;
}

int escrever(char unidade, unsigned short fd, const void *buffer, unsigned int tamanho) {

    //region checagem de disco e arquivo
    Disco *d = buscaPorUnidade(unidade);
    if (!d)
        return ERRO_ESCREVER_ARQUIVO;

    Arquivo *arq = d->tabelaArquivos[calculateFilePosition(fd)];
    if (!arq || arq->estado == FECHADO)
        return ERRO_ESCREVER_ARQUIVO;
    //endregion

    if (arq->tamanho)
        tamanho++;

    size_t recordSize = sizeof(void *);
    size_t bufferSize = tamanho * recordSize;

    if(calculateBlocks(bufferSize) > d->blocosLogicosLivres)
        return ERRO_ESCREVER_ARQUIVO;

    if (d->espacoLivre < bufferSize) {
        return ERRO_ESCREVER_ARQUIVO;
    }

    TIPO_DE_ARQUIVO tipoDeArquivo;
    unsigned int items = verificaTipoArquivo(&tipoDeArquivo, buffer);
    size_t itemsWrote;

    if (items == 0)
        return ERRO_ESCREVER_ARQUIVO;

    if (!arq->conteudo) {
        arq->conteudo = malloc(bufferSize);

        if (!arq->conteudo) {
            return ERRO_ESCREVER_ARQUIVO;
        }

        arq->tipo = tipoDeArquivo;

        itemsWrote = 0;
    } else {
        itemsWrote = strlen(arq->conteudo);

        if (tipoDeArquivo == STRING && arq->tipo == NUMERICO)
            return ERRO_ESCREVER_ARQUIVO;

        realloc(arq->conteudo, arq->tamanho + bufferSize - recordSize);
    }

    if (itemsWrote) {
        // adds a whitespace before writing
        strcat(arq->conteudo, " ");
        itemsWrote++;
    }
    memcpy(&arq->conteudo[itemsWrote], buffer, tamanho * recordSize);

    return (int) computeOccupiedSpace(d, arq, arq->tipo == NUMERICO ? items : tamanho);
}

unsigned int computeOccupiedSpace(Disco *d, Arquivo *arq, unsigned int numeroRegistros) {
    unsigned int recordSize;
    unsigned int bufferSize;

    recordSize = arq->tipo == NUMERICO ? sizeof(int) : sizeof(char);
    bufferSize = recordSize * numeroRegistros;

    size_t blocos = calculateBlocks(bufferSize);

    arq->tamanho += bufferSize;
    arq->blocosLogicos += blocos;
    d->espacoOcupado += bufferSize;
    d->espacoLivre -= bufferSize;
    d->blocosLogicosOcupados += blocos;
    d->blocosLogicosLivres -= blocos;

    return bufferSize;
}

size_t calculateBlocks(unsigned int bufferSize) {
    unsigned int blocos = bufferSize / TAMANHO_BLOCO_LOGICO;;

    if(bufferSize % TAMANHO_BLOCO_LOGICO)
        blocos++;

    return blocos;
}

unsigned short verificaTipoArquivo(TIPO_DE_ARQUIVO *tipoDeArquivo, const void *pVoid) {
    const char *data = (const char *) pVoid;
    int len;
    float ignore;

    const int digits = 100;
    char temp[digits];

    *tipoDeArquivo = NUMERICO;

    char splitData[MAX_PARAMS][PARAM_LENGTH];
    int c = tokenize(data, splitData, MAX_PARAMS, " ");
    for (int i = 0; i < c; i++) {
        strcpy(temp, splitData[i]);
        int ret = sscanf(temp, "%f%n", &ignore, &len); // NOLINT(cert-err34-c)
        if (ret != 1 || temp[len])
            *tipoDeArquivo = STRING;
    }

    return c;
}

int ler(char unidade, unsigned short fd, void *buffer, unsigned int tamanho) {

    //region checagem de disco e arquivo
    Disco *d = buscaPorUnidade(unidade);
    if (!d)
        return ERRO_LER_ARQUIVO;

    Arquivo *arq = d->tabelaArquivos[calculateFilePosition(fd)];
    if (!arq || arq->estado == FECHADO)
        return ERRO_LER_ARQUIVO;
    //endregion

    if (tamanho > 0 && tamanho <= arq->tamanho) {
        memcpy_s(buffer, tamanho, arq->conteudo, tamanho);
        return tamanho == arq->tamanho ? FIM_DE_ARQUIVO : (int) tamanho;
    }

    return ERRO_LER_ARQUIVO;
}

int fechar(char unidade, unsigned short fd) {
    Disco *d = buscaPorUnidade(unidade);

    if (!d)
        return ERRO_FECHAR_ARQUIVO;

    Arquivo *arquivo = d->tabelaArquivos[calculateFilePosition(fd)];
    if (!arquivo)
        return ERRO_FECHAR_ARQUIVO;

    arquivo->estado = FECHADO;

    return TRUE;
}

int excluir(char unidade, const char *nomeArquivo) {
    Disco *disco;

    disco = buscaPorUnidade(unidade);
    Arquivo *target = buscaArquivo(disco, nomeArquivo);

    if (!target)
        return ERRO_EXCLUIR_ARQUIVO;

    if (target->estado == ABERTO && fechar(unidade, target->fd) != TRUE)
        return ERRO_EXCLUIR_ARQUIVO;

    disco->espacoOcupado -= target->tamanho;
    disco->espacoLivre += target->tamanho;
    disco->arquivosEmDisco--;
    disco->blocosLogicosLivres += target->blocosLogicos;
    disco->blocosLogicosOcupados -= target->blocosLogicos;

    unsigned short filePos = calculateFilePosition(target->fd);
    free(disco->tabelaArquivos[filePos]);
    disco->tabelaArquivos[filePos] = NULL;

    return TRUE;
}

char *infoDisco(char unidade) {
    const int lenght = 300;

    Disco *d = buscaPorUnidade(unidade);

    if (!d)
        return NULL;

    // TODO migrate size to dinamic based on file data
    char *data = malloc(sizeof(char) * 3000);
    if (!data)
        return NULL;

    char temp[lenght];

    sprintf(data, "\nConteudo do disco %c\n", unidade);
    sprintf(temp, "\n\tNome"
                  "\t\tTipo"
                  "\t\tHora"
                  "\t\tTamanho"
                  "\t\tBlocos");

    strcat(data, temp);

    int count = 1;
    for (int i = 0; i < NUMERO_DE_ARQUIVOS_DO_DISCO && count <= d->arquivosEmDisco; i++) {
        Arquivo *arq = d->tabelaArquivos[i];

        if (arq) {
            sprintf(
                    temp,
                    "\n\t%s"
                    "\t\t%s"
                    "\t\t%s"
                    "\t\t%d bytes"
                    "\t\t%d",
                    arq->nome,
                    arq->tipo == NUMERICO ? TIPO_NUMERICO : TIPO_TEXTO,
                    arq->horaCriacao,
                    arq->tamanho,
                    arq->blocosLogicos
            );

            strcat(data, temp);
            count++;
        }
    }

    sprintf(temp, "\n\nNumero de arquivos: %d", d->arquivosEmDisco);
    strcat(data, temp);

    sprintf(temp, "\n\nCapacidade do disco: %d bytes", d->capacidade);
    strcat(data, temp);

    sprintf(temp, "\nEspaco ocupado: %d", d->espacoOcupado);
    strcat(data, temp);

    sprintf(temp, "\nEspaco livre: %d bytes", d->espacoLivre);
    strcat(data, temp);

    sprintf(temp, "\n\nTotal de blocos logicos: %d", d->blocosLogicosTotais);
    strcat(data, temp);

    sprintf(temp, "\nBlocos logicos ocupados: %d", d->blocosLogicosOcupados);
    strcat(data, temp);

    sprintf(temp, "\nBlocos logicos livres %d", d->blocosLogicosLivres);
    strcat(data, temp);

    return data;
}

void removeNewline(char *str) {
    if (str == NULL)
        return;

    unsigned int length = strlen(str);

    if (str[length - 1] == '\n')
        str[length - 1] = '\0';
}

int tokenize(const char *source, char dest[][PARAM_LENGTH], unsigned int expectedTokens, char *delimiter) {
    int count = 1;

    char temp[strlen(source)];
    strcpy(temp, source);

    char *t = strtok(temp, delimiter);

    while (t) {
        if (count < expectedTokens)
            strcpy(dest[count - 1], t);
        count++;

        t = strtok(NULL, delimiter);
    }

    return count - 1;
}

void menu() {
    char choice[PARAM_LENGTH] = "";
    char splitedInstruction[MAX_PARAMS][PARAM_LENGTH];

    do {
        if (strcmp(choice, "exit") == 0)
            exit(0);

        printf("fs> ");
        setbuf(stdin, NULL);
        fgets(choice, PARAM_LENGTH, stdin);
        removeNewline(choice);

        int c = tokenize(choice, splitedInstruction, MAX_PARAMS, " ");

        if (c > MAX_PARAMS) {
            printf("\n%s\n\n", TOO_MANY_PARAMETERS);
            continue;
        }

        execute(c, splitedInstruction);
    } while (1);
}

void execute(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {

    const char *instruction = args[0];

    if (!strcmp(instruction, CRIAR_DISCO)) {
        executaCriacaoDisco(argc, args);
        return;
    }

    if (!strcmp(instruction, CRIAR_ARQUIVO)) {
        executaCriacaoArquivo(argc, args);
        return;
    }

    if (!strcmp(instruction, ABRIR_ARQUIVO)) {
        manipulaEstadoAberturaArquivo(argc, ABERTO, args);
        return;
    }

    if (!strcmp(instruction, ESCREVER_ARQUIVO)) {
        executaEscritaArquivo(argc, args);
        return;
    }

    if (!strcmp(instruction, LER_ARQUIVO)) {
        executaLeituraArquivo(argc, args);
        return;
    }

    if (!strcmp(instruction, FECHAR_ARQUIVO)) {
        manipulaEstadoAberturaArquivo(argc, FECHADO, args);
        return;
    }

    if (!strcmp(instruction, EXCLUIR_ARQUIVO)) {
        executaExclusaoArquivo(argc, args);
        return;
    }

    if (!strcmp(instruction, LISTAR_DISCO)) {
        listaDisco(argc, args);
        return;
    }

    if (!strcmp(instruction, FORMATAR_DISCO)) {
        executaFormatacaoDisco(argc, args);
        return;
    }

    printf("\n%s não é um comando reconhecido.", instruction);
}

int checkSyntax(int received, int expected) {
    if (received != expected) {
        printf("%s\n", INCORRECT_SYNTAX);
        return 0;
    }

    return 1;
}

Disco *checkDisco(const char *unidade) {
    if (strlen(unidade) > 1) {
        printf("Unidade de disco inválida\n");
        return NULL;
    }

    Disco *d = buscaPorUnidade(unidade[0]);
    if (!d) {
        printf("Unidade de disco nao encontrada\n");
        return NULL;
    }

    return d;
}

Arquivo *checkArquivo(const Disco *d, const char *nome) {
    if (strlen(nome) > TAMANHO_NOME_ARQUIVO) {
        printf("\nNome do arquivo inválida\n");
        return NULL;
    }

    Arquivo *arq = buscaArquivo(d, nome);
    if (!arq) {
        printf("\nArquivo nao encontrado no disco especificado\n");
        return NULL;
    }

    return arq;
}

void executaCriacaoDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {

    if (!checkSyntax(argc, 2))
        return;

    if (numeroDiscos == NUM_MAXIMO_DISCOS) {
        printf("\nNumero maximo de discos atingido\n");
        return;
    }

    const char *name = args[1];
    if (strlen(name) > 1) {
        printf("%s\n", UNITY_TOO_LONG);
        return;
    }

    if (buscaPorUnidade(name[0])) {
        printf("%s\n", THERE_IS_ALREADY_A_DISK_WITH_THIS_UNITY_NAME);
        return;
    }

    Disco *d = (criaDisco(name[0], CAPACIDADE_DISCO));
    if (d) {
        discos[numeroDiscos++] = d;
        printf("%s\n", DISK_SUCCESSFUL_CREATED);
    } else
        printf("%s\n", DISK_CREATION_FAILURE);
}

void executaCriacaoArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {
    if (!checkSyntax(argc, 3))
        return;

    Disco *d = checkDisco(args[1]);
    if (!d)
        return;

    if (strlen(args[2]) > TAMANHO_NOME_ARQUIVO) {
        printf("%s\n", FILE_NAME_TO_LONG);
        return;
    }

    if (buscaArquivo(d, args[2])) {
        printf("%s\n", THERE_IS_ALREADY_A_FILE_WITH_THIS_NAME);
        return;
    }

    if (criar(d->unidade, args[2])) {
        printf("%s\n", FILE_SUCCESSFUL_CREATED);
        return;
    } else
        printf("%s\n", FILE_CREATION_FAILURE);
}

void executaEscritaArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {

    if (argc < 3) {
        printf("%s\n", INCORRECT_SYNTAX);
        return;
    }

    Disco *d = checkDisco(args[1]);
    if (!d)
        return;

    Arquivo *arq = checkArquivo(d, args[2]);
    if (!arq)
        return;

    char args2[PARAM_LENGTH];
    strcpy(args2, args[3]);

    for (int i = 4; i < argc; i++) {
        strcat(args2, " ");
        strcat(args2, args[i]);
    }

    int result = escrever(d->unidade, arq->fd, args2, strlen(args2));

    if (result == ERRO_ESCREVER_ARQUIVO && strlen(arq->conteudo) != ERRO_ESCREVER_ARQUIVO) {
        printf("Erro ao escrever no arquivo\n");
        return;
    }

    printf("%d bytes escritos no arquivo\n", result);
}

void executaLeituraArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {

    if (!checkSyntax(argc, 3))
        return;

    Disco *d = checkDisco(args[1]);
    if (!d)
        return;

    Arquivo *arq = checkArquivo(d, args[2]);
    if (!arq)
        return;

    unsigned int size = strlen(arq->conteudo);
    void *buff[size];

    if (ler(d->unidade, arq->fd, buff, size) == ERRO_LER_ARQUIVO && strlen((char *)buff) != ERRO_LER_ARQUIVO) {
        printf("Falha na leitura do arquivo\n");
        return;
    }
    printf("%s\n", ((char *) buff));
}

void manipulaEstadoAberturaArquivo(int argc, EstadoArquivo estado, const char args[MAX_PARAMS][PARAM_LENGTH]) {
    if (!checkSyntax(argc, 3))
        return;

    char unidade = *args[1];
    Arquivo *arquivo = checkArquivo(checkDisco(args[1]), args[2]);

    if (!arquivo)
        return;

    if (estado == ABERTO) {
        if (abrir(unidade, args[2]) == TRUE)
            printf("Arquivo aberto com sucesso\n");
        else
            printf("Falha ao abrir arquivo\n");
    } else {
        if (fechar(unidade, arquivo->fd) == TRUE)
            printf("Arquivo fechado com sucesso\n");
        else
            printf("Falha ao fechar arquivo\n");
    }
}

void executaExclusaoArquivo(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {
    if (!checkSyntax(argc, 3))
        return;

    char unidade = *args[1];
    Arquivo *arquivo = checkArquivo(checkDisco(args[1]), args[2]);

    if (!arquivo){
        printf("Falha ao excluir\n");
        return;
    }

    if (excluir(unidade, args[2]) == TRUE) {
        printf("Arquivo exluido com sucesso\n");
    } else
        printf("Falha ao excluir\n");
}

void executaFormatacaoDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {
    if (!checkSyntax(argc, 2))
        return;

    const char *unidade = args[1];

    Disco *d = checkDisco(unidade);

    if (!d)
        return;

    if (formatar(unidade[0], d) != TRUE) {
        printf("Falha ao formatar disco\n");
    } else
        printf("Formatação realizada com sucesso.\n");
}

void listaDisco(int argc, const char args[MAX_PARAMS][PARAM_LENGTH]) {
    if (!checkSyntax(argc, 2))
        return;

    const char *unidade = args[1];
    if (strlen(unidade) > 1) {
        printf("%san", UNITY_TOO_LONG);
        return;
    }

    char *data = infoDisco(unidade[0]);
    if (data) {
        printf("%s\n", data);
        return;
    }

    printf("Falha ao obter dados\n");
}

int main() {
    setlocale(LC_ALL, "pt-Br");
    menu();
    return 0;
}