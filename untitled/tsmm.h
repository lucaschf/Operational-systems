#ifndef TSMM_H_INCLUDED
#define TSMM_H_INCLUDED
#define MAX 4
#define QUANTUM 2
#define TRUE 1
#define FALSE 0
#define MAXIMO_FRAMES 127
#define MAXIMO_PAGINAS 7
#define PAGINAS 512

typedef enum tipo{ALOCAR=1, MEMORIA, DISCO}TIPO;
typedef enum estados{NOVA,PRONTA,EXECUTANDO,SUSPENSA,TERMINADA}ESTADO;

typedef struct{
    char instrucoes[200];
}Instrucoes;

typedef struct{
    char tarefa[200];
    ESTADO estado;
    unsigned int tempoCPU;
    unsigned int tempoES;
    unsigned int tamanho;
    Instrucoes instrucoes[200];
    unsigned int quantidadeInstrucoes;
    unsigned int instanteInicial;
    unsigned int instanteFinal;
    unsigned int paginasUtilizadas;
}Tarefa;
Tarefa t[10];

typedef struct Nodo {
    Tarefa info;
    struct Nodo* prox;
} TNodo;

typedef struct {
    TNodo *inicio,*fim;
    int tamanho;
} Fila;
typedef Fila *fila;

typedef struct{
    int memoriaFisica[127];
    unsigned int paginasUtilizadas;
} Memoria;


int iniciar();
fila criaFila();
void liberaFila(fila);
int enfileira(fila, Tarefa);
int desenfileira(fila,Tarefa*);
int filaCheia(fila);
int filaVazia(fila);
int tamanhoFila(fila);
int imprimirFila(fila);
char *lerLinhaEntrada(FILE *, char *);
int verificarDadosLinha(char *,Tarefa *, int ,int );
void inicializaElementosStruct(Tarefa *);
void escalonador(fila, fila);
int lerArquivo(char *,Tarefa *, int, unsigned int,FILE *);
int identificarInstrucaoTarefa(char *,Memoria *,Tarefa *);
void relatorio(int *,unsigned int,int *,unsigned int, int);
int gerenciarMemoriaFisica(Memoria *, unsigned int);
int gerenciarMemoriaTarefa(Tarefa *, unsigned int);
int verificarPosicaoVetor(Memoria *, Tarefa *,unsigned int);

#endif