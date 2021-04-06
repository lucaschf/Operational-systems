#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include "tsmm.h"

char *lerLinhaEntrada(FILE *arq, char *linha){
    return fgets(linha, 200, arq);
}

fila criaFila(){
    fila f=(fila)malloc(sizeof(Fila));
    if(f){
        f->inicio=NULL;
        f->fim =NULL;
        f->tamanho=0;
    }
    return f;
}

void liberaFila(fila f){
    TNodo *p;
    while(f->inicio){
        p=f->inicio;
        f->inicio= f->inicio->prox;
        free(p);
    }
    free(f);
}

int enfileira(fila f, Tarefa t){
    TNodo *p =(TNodo*)malloc(sizeof(TNodo));

    if(!p)
        return 0;

    p->prox=NULL;
    p->info=t;
    if(f->inicio==NULL)
        f->inicio =p;
    else
        f->fim->prox=p;

    f->fim =p;
    f->tamanho++;
    return 1;
}

int desenfileira(fila f,Tarefa *t){
    TNodo *p;
    if(f->inicio==NULL)
        return 0;

    p=f->inicio;
    f->inicio= f->inicio->prox;
    f->tamanho--;
    *t=p->info;
    free(p);
    return 1;
}

int imprimirFila(fila f){
    TNodo *aux;

    if(!f->tamanho){
        printf("\nFila Vazia!\n\n");
        return 1;
    }

    aux = f->inicio;
    while(aux){
        printf("\nTarefa: %s\n",aux->info.tarefa);
        for(int i=0;i<aux->info.quantidadeInstrucoes;i++)
            printf("%s", aux->info.instrucoes[i].instrucoes);
        aux = aux->prox;
        printf("\n");
    }

    return 0;
}

int verificarDadosLinha(char *linha, Tarefa *t, int pos, int tarefa){
    int size = sizeof(size),j=0,tamanho, valido;
    const char new[] = "new", disk[] = "disk",c[] = "[", read[] = "read";
    char *ret;

    if(linha[0] == '#'){
        sscanf(linha, "%*c%*c%*c%d", &tamanho);
        t[tarefa].tamanho = tamanho;
        return 1;
    }

    ret = strstr(linha,new);
    if(ret){
        strcpy(t[tarefa].instrucoes[pos].instrucoes,linha);
        return 1;
    }

    //Verifica a instrução de acesso a disco.
    ret = strstr(linha,read);
    if(ret){
        ret = strstr(linha,disk);
        if(ret){
            strcpy(t[tarefa].instrucoes[pos].instrucoes,linha);
            return 1;
        }
        return -1;
    }

    //Verifica se a instrução de acesso à memória é válida.
    ret = strstr(linha,c);
    if(ret){
        valido = FALSE;
        int j=0;
        for(int i=0;i< sizeof(ret); i++){
            if(ret[i] == ']')
                valido = TRUE;
        }
        if(!valido)
            return -1;
        strcpy(t[tarefa].instrucoes[pos].instrucoes,linha);
        return 0;
    }

    return -1;
}

void inicializaElementosStruct(Tarefa *t){
    t->estado=NOVA;
    t->tempoCPU=0;
    t->tempoES=0;
    t->tamanho=0;
    t->quantidadeInstrucoes=0;
    t->instanteFinal=0;
    t->paginasUtilizadas;
}

int lerArquivo(char *nome,Tarefa *t, int x, unsigned int i, FILE *arq){
    char linha[200];
    arq = fopen(nome, "r");
    if(!arq){
        printf("\nArquivo %s Inválido!\n\n", nome);
        return 1;
    }
    fseek(arq,0,SEEK_SET);
    while(!feof(arq)){
        if(lerLinhaEntrada(arq,linha)!=NULL){
            if(!strcmp(linha,"\n") || !strcmp(linha,"\0"))
                continue;
            if(verificarDadosLinha(linha,t,x,i)==-1)
                return -1;
            x++;
            t[i].quantidadeInstrucoes = x;
        }
    }
    return 0;
}

int gerenciarMemoriaTarefa(Tarefa *tarefa, unsigned int tamanho){
    int div, resto;
    div = tamanho/PAGINAS;
    resto = tamanho%PAGINAS;
    if(resto > 0)
        div++;

    if(tarefa->paginasUtilizadas+div > MAXIMO_PAGINAS)
        return -1;

    return div;
}

int gerenciarMemoriaFisica(Memoria * memoria, unsigned int tamanho){
    int div, resto;
    /*
    Memória Física = 64KB = 2^16
    Tamanho Tarefa = 4KB = 2^12
    Páginas = 512 bytes

    Paginação:
    Nº de Frames = 65536/512 = 128 (0 a 127) -> 7bits
    Nº de Páginas = 4096/512 = 8 (0 a 7) -> 3bits
    Deslocamento = 512 bytes -> 9bits
    */

    //Verifica se há espaço para alocar na memória física.
    div = tamanho/PAGINAS;
    resto = tamanho%PAGINAS;

    //Se sobrar páginas, utiliza mais uma página para elas.
    if(resto > 0)
        div++;

    if(memoria->paginasUtilizadas+div > MAXIMO_FRAMES)
        return -1;

    return div;

}

int verificarPosicaoVetor(Memoria *memoria, Tarefa *tarefa,unsigned int posicao){
    /*
    E = p+d;
    p = E div P;
    d = E mod P;
    */
    return 0;
}

int identificarInstrucaoTarefa(char *instrucao, Memoria *memoria, Tarefa *tarefa){
    const char new[] = "new", disk[] = "disk", c[] = "[";
    char *ret, pos[5];
    unsigned int posicao,tamanho;
    int retornoMF, retornoMT;

    //Verifica se é uma instrução de alocação.
    ret = strstr(instrucao,new);
    if(ret){
        sscanf(ret, "%*s%*c%d", &tamanho);
        gerenciarMemoriaFisica(memoria,tamanho);
        retornoMF = gerenciarMemoriaFisica(memoria,tamanho);
        if(retornoMF == -1){
            printf("\nMemória cheia!");
            return -1;
        }

        retornoMT = gerenciarMemoriaTarefa(tarefa,tamanho);
        if(retornoMT == -1){
            printf("\nMemória cheia!");
            return -1;
        }
        memoria->paginasUtilizadas += retornoMF;
        tarefa->paginasUtilizadas += retornoMT;

        return MEMORIA;
    }

    //Verifica se é Acesso a disco.
    ret = strstr(instrucao,disk);
    if(ret)
        return DISCO;

    //Verifica se é Acesso a memória.
    ret = strstr(instrucao,c);
    if(ret){
        int j=0;
        //printf("\n%s",ret);
        for(int i=0;i< sizeof(ret); i++){
            if(ret[i] == '[' || ret[i] == ']')
                continue;
            pos[j] = ret[i];
            j++;
        }
        posicao = atoi(pos);
        verificarPosicaoVetor(memoria,tarefa,posicao);
        return posicao;
    }
    return 0;
}

void relatorio(int tt[], unsigned int quantidadeTarefas, int tw[],unsigned int tempoTotal, int tempoES){
    int soma=0,somaEspera=0, tp=0;
    float tempoMedioExe,tempoMedioEspera;
    for(unsigned int i=0;i< quantidadeTarefas;i++){
        soma += tt[i];
        somaEspera +=  tt[i] - tw[i];
        tp += tw[i];
    }

    printf("\n\n\t\t RELATÓRIO\n");
    tempoMedioExe = soma/(float)quantidadeTarefas;
    printf("\nTempo médio de execução: %.1fs", tempoMedioExe);
    tempoMedioEspera = somaEspera/(float)quantidadeTarefas;
    printf("\nTempo médio de espera: %.1fs", tempoMedioEspera);
    printf("\nTaxa de Ocupação do Processador: %.1f%%", (tp/(float)tempoTotal)*100);
    printf("\nTaxa de Ocupação do Disco: %.1f%%", (tempoES/(float)tempoTotal)*100);
}

void escalonador(fila f, fila filaReserva){
    Memoria memoria;
    TNodo *aux, *aux2;
    Tarefa t, tReserva;
    fila filaSuspensos = criaFila();
    unsigned int ut=1, pos, retorno, utDisco=1,instanteFinal=0, posTempo=0, quantidadeTarefas=f->tamanho + filaReserva->tamanho;
    int acessoDisco = TRUE, tt[quantidadeTarefas], tw[quantidadeTarefas], tempoES=0;

    //Reservado para o sistema, 40 páginas.
    memoria.paginasUtilizadas = 40;

    //Enquanto houver tarefas na fila, continua executando.
    while (f->tamanho){
        aux = f->inicio;
        t = aux->info;

        //Verifica se tem espaço na memória (lógica e física) para a NOVA tarefa.
        if(aux->info.estado == NOVA){
            aux->info.estado = PRONTA;
            if(gerenciarMemoriaFisica(&memoria,aux->info.tamanho)==-1 || gerenciarMemoriaTarefa(&t,aux->info.tamanho)==-1){
                printf("\nMemória cheia! Tarefa: %s", t.tarefa);
                continue;
            }
        }

        /* Caso uma tarefa seja finalizada, obtém-se as informações sobre os instantes de
           tempo, além de verificar se há mais tarefas na fila de tarefas, para que possam
           ser executadas.
        */

        if(aux->info.estado == TERMINADA){
            tempoES += t.tempoES;
            tt[posTempo] = instanteFinal-t.instanteInicial;
            tw[posTempo] = aux->info.tempoCPU+1;
            posTempo++;
            desenfileira(f, &t);
            //FilaReserva
            if(!filaReserva->tamanho){
                continue;
            }
            else{
                aux2 = filaReserva->inicio;
                tReserva = aux2->info;
                desenfileira(filaReserva,&tReserva);
                enfileira(f,tReserva);
            }
        }
        instanteFinal++;
        aux->info.estado = EXECUTANDO;
        pos = aux->info.tempoCPU;

        //Se houver apenas uma instrução então já altera o estado para TERMINADA.
        if(aux->info.quantidadeInstrucoes==1){
            aux->info.estado = TERMINADA;
            //ut = QUANTUM;
            retorno = identificarInstrucaoTarefa(t.instrucoes[pos].instrucoes,&memoria,&t);

            continue;
        }
            //Se ainda houver instruções.
        else{
            retorno = identificarInstrucaoTarefa(t.instrucoes[pos].instrucoes,&memoria,&t);
            //Se a instrução for de acesso a disco, insere a tarefa na fila de suspensos.
            if(retorno == -1)
                continue;
            if(retorno == DISCO){
                aux->info.estado = SUSPENSA;
                aux->info.tempoES += 5;
                acessoDisco = TRUE;
                //f->inicio = aux;
                enfileira(filaSuspensos,aux->info);
                ut = QUANTUM;
            }

            aux->info.quantidadeInstrucoes--;
            aux->info.tempoCPU++;
        }

        //Caso o tempo de acesso ao disco tenha terminado.
        if(acessoDisco && utDisco==5){
            aux2 = filaSuspensos->inicio;
            acessoDisco = FALSE;
            utDisco = 1;
            desenfileira(filaSuspensos,&t);
            enfileira(f,t);
        }

        //Se a unidade de tempo for igual a 2, vai para a próxima tarefa.
        if(ut == QUANTUM){
            if(aux->prox == NULL){
                //f->inicio = aux;
                desenfileira(f,&t);
                enfileira(f,t);
            }
            else{
                //f->inicio = aux;
                desenfileira(f,&t);
                enfileira(f,t);
            }
            ut = 1;
            continue;
        }
        ut++;
        utDisco++;
    }

    liberaFila(filaSuspensos);
    relatorio(tt,quantidadeTarefas,tw,instanteFinal,tempoES);
}

int iniciar(int argc, char *argv[]){
    setlocale(LC_ALL,"portuguese");
    FILE *arq;
    fila f = criaFila(), filaReserva = criaFila();
    char nome[10];
    int x, retorno;

    for(unsigned int i=1; i<argc;i++){
        x=-1;
        inicializaElementosStruct(&t[i]);
        printf("\nnome: %s", argv[1]);
        sprintf(nome, "%s.tsk",argv[i]);
        retorno = lerArquivo(nome,t,x,i,arq);
        if(retorno==-1){
            printf("\nA tarefa %s não será executada, pois tem instruções diferentes do tipo 1,2 e 3.\n", argv[i]);
            continue;
        }else if(retorno == 1)
            continue;
        sprintf(t[i].tarefa,"%s",nome);
        t[i].instanteInicial = i-1;
        //Se houver mais que 4 tarefas, vai para a fila reserva, para não executar mais do que 4 ao mesmo tempo.
        if(i>MAX)
            enfileira(filaReserva,t[i]);
        else
            enfileira(f,t[i]);
    }

    if(!imprimirFila(f))
        escalonador(f,filaReserva);

    imprimirFila(filaReserva);

    liberaFila(f);
    liberaFila(filaReserva);
    printf("\n");
    return 0;
}

int main(int argc, char *argv[]) {
    argc = 2;
    argv[1] = "C:\\Users\\lucas\\Desktop\\TalitaSilva\\t3";
    return iniciar(argc, argv);
}