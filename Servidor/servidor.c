#include "lista.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*bibliotecas para comunicação via SOCKET*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*biblioteca para uso de thread*/
#include <pthread.h>

#define QNT_ATENDIMENTO 2 


pthread_mutex_t mutexApto = PTHREAD_MUTEX_INITIALIZER;/*controla o acesso a lista de APTOS*/
pthread_mutex_t mutexExec = PTHREAD_MUTEX_INITIALIZER;/*controla o acesso a lista de EXECUCAO*/
pthread_mutex_t serv = PTHREAD_MUTEX_INITIALIZER; /*controla o acesso ao ID_CLIENTE*/
/*Estrutura para mandar as duas listas para thread "núcleo" */
typedef struct{

        lista lApto;/*lista que armazena os processos aptos*/
        lista lExec;/**lista que armazena os processos em execução*/
        int id_cliente;/*identificador do cliente*/
	int porta;
}listProcesso;



int shutd =0 ;




int setaServidor( struct sockaddr_in *serv, int *id_serv, int port);
void* servico(void* x);
void* nucleo(void* l);

void escolheComando(listProcesso *l, int *pid, pthread_t *nucleo);
int separaComando(char *comando, char *tipoComando, char *nome, int *tempoExec, int *pidKill);
int separaCreate(char *comando, char *nome, int *tempoExec, int indice);
int separaKill(char *comando, int *pidKill, int indice);
int processoLista(lista *l, int *pids, char *nome, int tempoExec);/*coloca um processo na lista*/
int killProcesso(lista *l, int pidKill);
void psProcesso(lista *l);
void mostraTime(time_t *t);
void shutdownProcessos(lista *l);
void legenda();





int main(int argc, char *argv[]){


	pthread_t thread;/*identificaor da thread*/

	listProcesso l;/*argumento para a thread "núcleo"*/

        inicializa(&l.lApto);
        inicializa(&l.lExec);

        int pids = 0;/*controle dos PIDs*/


	if(argc == 2 ){
		l.porta  = atoi(argv[1]);
	}
	else{
		l.porta = 7777;
	}

	if(pthread_create(&thread, NULL, servico, (void *) &l)){

		printf("\nErro ao criar a thread\n");
		exit(1);
	}

	legenda();/*mostra exemplos de comando*/

	escolheComando(&l, &pids, &thread);

	pthread_join(thread, NULL);/*aguarda a thread terminar para finalizar*/
	return 0;
}



void* servico(void* x){/*thread responsável por criar as outras threads para a comunicação*/

	int atendimento = 0;
	listProcesso *l = (listProcesso *) x;
	pthread_t threads[QNT_ATENDIMENTO];/*referência das threads para a comunicação*/

	/*variaveis para a comunicação (SERVIDOR)*/
        struct sockaddr_in servidor;/*estrutura para o servidor*/
        int id_servidor;/*identificador do servidor*/

        /*variaveis para a comunicação (CLIENTE)*/
        struct sockaddr_in cliente[QNT_ATENDIMENTO];/*estrutura para os clientes*/
        int id_cliente[QNT_ATENDIMENTO];/*identificador dos clientes*/
        int tamanho_cliente = sizeof(struct sockaddr_in);/*Armazena o tamanho dos clientes*/

	if(!(setaServidor(&servidor, &id_servidor, l->porta))){
		printf("\n Falha na criacao do socket\n");
		exit(1);
	}

	while(atendimento < QNT_ATENDIMENTO){

		listen(id_servidor, 5);/*Coloca o servidor na escuta*/
		pthread_mutex_lock(&serv);/*bloqueia ate o servidor receber o ID_CLIENETE na funcao (nucleo)*/
		id_cliente[atendimento] = accept(id_servidor, (struct sockaddr *) &cliente[atendimento], &tamanho_cliente);/*Aceita a conexão*/
		if(id_cliente[atendimento]>0){/*Caso tenha recebido uma conexão cria uma thread*/
			l->id_cliente = id_cliente[atendimento];
			if(pthread_create(&threads[atendimento], NULL, nucleo, (void *) l)){
				printf("\nErro ao criar a thread %d\n", atendimento);
				close(id_servidor);
			}
			atendimento++;
		}
	}


	atendimento = 0;/*Espera as thread terminar*/
	while(atendimento < QNT_ATENDIMENTO){

		pthread_join(threads[atendimento], NULL);
		atendimento++;
	}
	close(id_servidor);
	pthread_exit(NULL);

}



void* nucleo(void* l){

	listProcesso *list;
	char mensagem[100];
        list =(listProcesso *)l;
	int id = list->id_cliente;
	pthread_mutex_unlock(&serv);/*desbloquia, apos o armazenamento do ID_CLIENTE para criar novos sockets*/
	memset(mensagem, '\0',100);
	strcpy(mensagem, "Conexao Iniciada");
	send(id, mensagem, 100, 0);

        lista *listA;
        listA = &list->lApto;

        lista *listE;
        listE = &list->lExec;

        no *noExcluiExec; /*armazena o end do processo em execução que será excluido*/
        int pid;/*armazena o pid do processo em execucao que será excluido*/
        int i=1; /*indice para achar a posição do processo que está na lista execução e devera ser excluido após executar */


        no *noAtual;
        processo *proc;
        time_t inicioExec;
        time_t tempoAtual;

	while(1){
                pthread_mutex_lock(&mutexApto);//bloquei o acesso a lista aptos
                if(listA->tamanho){/*Se tem processo na lista*/
			noAtual = (no*)listA->primeiro;
                        proc = noAtual->memoria;
			proc->estado = 1;/*coloca no estado executando*/
                        pid = proc->pid;/*recebe o pid do processo que ira para execuçao para após a execução o mesmo poder se excluido*/
			if(!inserefim(listE, proc)){/*Passa o processo da lista de Aptos para a lista de Execucao*/
                                printf("\nProblema para armazenar na lista de EXECUCAO\n");
                                pthread_exit(NULL);
                        }
                        tiraInicio(listA);/*Apos passa para a lista de Execução, tira o mesmo da lista de Aptos, mas não exclui o nó pois o mesmo ira para a lista execução*/

                        pthread_mutex_unlock(&mutexApto);//desboqueia
                        time(&inicioExec);
                        time(&tempoAtual);

			while((tempoAtual-inicioExec < proc->tempoExec)){

                                if(proc->kill){
                                        break;
                                }
                                if(shutd){
					memset(mensagem, '\0', 100);
		                        strcpy(mensagem ,"Conexao Encerrada");
                		        send(id, mensagem, 100, 0);
                                        pthread_exit(NULL);
                                }
                                time(&tempoAtual);
                        }
                        pthread_mutex_lock(&mutexExec);//bloqueia
                        i=1;

                        while(i <= listE->tamanho){/*Após terminar a Execução, procura o processo na lista de execução e o exclui*/

                                noExcluiExec = retornaNoPosicao(listE, i);
                                proc = noExcluiExec->memoria;
                                if(proc->pid == pid){
                                        excluiEm(listE, i);
                                }
                                i++;
                        }
                        pthread_mutex_unlock(&mutexExec);//desbloqueia

                }
		else if(shutd){
			memset(mensagem, '\0', 100);
        		strcpy(mensagem ,"Conexao Encerrada");
        		send(id, mensagem, 100, 0);
			pthread_mutex_unlock(&mutexApto);//desloqueia
                        pthread_exit(NULL);
                }
                else{
                        pthread_mutex_unlock(&mutexApto);//desloqueia
                }

		/*memset(mensagem, '\0', 100);
		strcpy(mensagem, "continua");
		send(id, mensagem, 100, 0);
		memset(mensagem, '\0', 100);
		if(!recv(id, mensagem, 100, 0)){
			printf("\nCliente %d caiu\n",id);
			pthread_exit(NULL);
		}*/
        }

}




int setaServidor( struct sockaddr_in *serv, int *id_serv, int port){

	*id_serv = socket(AF_INET, SOCK_STREAM, 0);/*Cria o socket*/

	if(*id_serv < 0){/*Verifica se o socket foi criado*/
		return 0;
	}

	serv->sin_family = AF_INET;
	serv->sin_port = htons(port);
	serv->sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(*id_serv, (struct sockaddr *) serv, sizeof(struct sockaddr_in)) < 0){
		perror("\nErro ao \"Bindar\" \n");
		close(*id_serv);
		return 0;
	}
	return 1;
}










void legenda(){
        printf("\e[H\e[2J");/*limpa a tela*/
        printf("\n##################################################################\n");
        printf("## Comando CREATE NOME TEMPO_EXECUCAO,          ex.(create a 10)##\n");
        printf("## Comando KILL PID                             ex.(kill 12)    ##\n");
        printf("## Comando TIME                                 ex.(time )      ##\n");
        printf("## Comando PS                                   ex.(ps )        ##\n");
        printf("## Comando SHUTDOWN                             ex.(shutdown)   ##\n");
        printf("##################################################################\n");
}


void escolheComando(listProcesso *l, int *pid, pthread_t *nucleo){

        /*Var. p/ controle o comando dado pelo usuário*/
        char comando[50];
        char tipoComando[50];
        char nome[50];
        int tempoExec = 0;
        int pidKill = 0;
        time_t tempo;
        int opcao = 0;

	while(1){

                gets(comando);
                opcao = separaComando(comando, tipoComando, nome, &tempoExec, &pidKill);

                if(opcao){
                        if(opcao == 1){/*se o comando for create a funcao retorna (1) */
                                if(processoLista(&l->lApto, pid, nome, tempoExec)){
                                        legenda();
                                        printf("\nProcesso Criado\n");
                                }
                                else{
                                        legenda();
                                        printf("\nProcesso nao Criado\n");
                                }
                        }
			else if(opcao == 2){/*se o comando for (kill) a função retorna (2)*/
                                if( killProcesso(&l->lApto, pidKill)){/*Procura primeiro na lista de Aptos*/
                                        legenda();
                                        printf("\nkill processo PID=%d\n", pidKill);
                                }
                                else if(killProcesso(&l->lExec, pidKill)){/*Procura na lista de Execução*/
                                        legenda();
                                        printf("\nkill processo PID=%d\n", pidKill);
                                }
                                else{
                                        legenda();
                                        printf("\nprocesso PID=%d nao existe\n", pidKill);
                                }
                        }
                        else if(opcao == 3){/*se o comando for (ps) a função retorna (3)*/
                                legenda();
                                printf("\nPID\tNOME PROCESSO\tTEMPO EXEC\tTEMPO CRIACAO\tESTADO\n");
                                psProcesso(&l->lExec);/*mostra os processos da lista de execução*/
                                psProcesso(&l->lApto);/*mostra os processos da lista de aptos*/
                        }
                        else if(opcao == 4){/*se o comando for (time) a função retorna (4)*/
                                time(&tempo);
                                mostraTime(&tempo);
                        }
			else if(opcao == 5){/*se o comando for (shutdown) a função retorna*/
                                legenda();
                                shutd = 1; /*informa a thread nucleo para finalizar-se*/
                                pthread_join(*nucleo, NULL);/*espera a thread nucleo finalizar*/

                                shutdownProcessos(&l->lExec);/*Finaliza os processos da lista de execução */
                                shutdownProcessos(&l->lApto);/*Finaliza os processos da lista de Aptos*/
                                break;
                        }
                }
		else{
                        legenda();
                        printf("\nComando invalido\n");
                }

        }

}


int separaComando(char *comando, char *tipoComando, char *nome, int *tempoExec, int *pidKill){

        int indice=0;
        int j=0;

        /*Separa até a primeira parte da string que deve ser o comando*/
        while(comando[indice]!=' ' && comando[indice]!='\0'){
                tipoComando[j] = comando[indice];
                indice++;
                j++;
        }

        tipoComando[j] = '\0';/*Coloca o caracter terminador na string tipoComando*/

        /*Comando CREATE ?*/
        if(!strcmp(tipoComando,"create")){
                if(separaCreate(comando, nome, tempoExec, indice)){
                        return 1;
                }
                return 0;
        }

        /*Comando KILL ?*/
        if(!strcmp(tipoComando, "kill")){
                if(separaKill(comando, pidKill, indice)){
                        return 2;
                }
                return 0;
        }

	 /*Comando PS ?*/
        if(!strcmp(tipoComando, "ps")){
                return 3;
        }

        /*Comando TIME ?*/
        if(!strcmp(tipoComando, "time")){
                return 4;
        }

        /*Comando SHUTDOWN ?*/
        if(!strcmp(tipoComando, "shutdown")){
                return 5;
        }

        return 0;

}

int separaCreate(char *comando, char *nome, int *tempoExec, int indice){
        char tempo[10];
        int j=0;
        if(comando[indice] == '\0'){/*verifica se o comando  'create' tem um nome */
                return 0;           /* do prcesso e um tempo de execução*/
        }
        indice++;

        while(comando[indice] != ' ' && comando[indice] != '\0'){/*separa o nome do processo*/
                nome[j] = comando[indice];
                j++;
                indice++;
        }
        nome[j] = '\0';/*Coloca o finalizador na string*/
        j=0;

        if(comando[indice] == '\0'){/*verifica se o comano é so 'create' e 'nome' */
                return 0;           /*pois o mesmo precisa de um tempo de execução*/
        }
        indice++;

	while(comando[indice] != '\0'){/*separa o tempo de execução do processo*/
                tempo[j] = comando[indice];
                j++;
                indice++;
        }
        tempo[j] = '\0'; /*Coloca o finalizador na string*/

        *tempoExec = atoi(tempo); /*converte os numeros da string em int, caso não possua números retorna zero*/

        if(!(*tempoExec)){/*não pode ter tempo de exec igual a zero*/
                return 0;
        }

        return 1;
}


int separaKill(char *comando, int *pidKill, int indice){

        char pid[10];/*recebe o pid do comando*/
        int j=0;
        if(comando[indice] == '\0'){/*verifica se o comando kill está certo*/
                return 0;
        }
        indice++;

        while(comando[indice] != '\0'){/*pega da string comando o pid informado*/
                pid[j] = comando[indice];
                j++;
                indice++;
        }
        pid[j] = '\0'; /*Coloca o marcado final na string*/

        if(pid[0] == '0'){/*testa aqui, pois quando converte*/
                *pidKill = 0;/*string para inteiro com atoi()*/
                return 1;    /*qualquer caracter que não for*/
        }                    /*número recebe zero*/

        *pidKill = atoi(pid);/*conver a string para inteiro*/

        if(!(*pidKill)){/*se for zero quer dizer que o caracter não era número*/
                return 0;
        }
        return 1;

}

int processoLista(lista *l, int *pids,char *nome,int tempoExec){/*coloca um processo na lista*/
        processo *p;
        p = criaProcesso((*pids), nome, tempoExec);/*cria o processo e retorna a referência do mesmo*/

        if(p){/*verifica se o processo foi criado*/
                inserefim(l,p);/*Coloca o novo processo no fim da lista*/
                (*pids)++;
                return 1;
        }
        return 0;
}

int killProcesso(lista *l, int pidKill){
        no *noPid;/*recebe o NÓ da POSICÃO em questão*/
        processo *proPid;/*recebe o PROCESSO do NÓ em questão*/
        int i = 1;/*variavel para comecar a busca pelo primeiro NÓ*/

        while(i<=l->tamanho){/*busca em todos os NÓs*/
                noPid = retornaNoPosicao(l, i);
                proPid =  noPid->memoria;
                if(proPid->pid == pidKill){
                        if(proPid->estado){/*Se o processo está em execução precisa ser parado*/
                                proPid->kill = 1;/*ou seja, tirado da thread*/
                                return 1;
                        }
                        excluiEm(l, i);
                        return 1;
                }
                i++;
        }
        return 0;
}

void psProcesso(lista *l){

        struct tm *x;/*recebe a estrutura da hora*/

        int i = 0;
        no *noAtual;
        processo  *pAtual;

        if(l->tamanho){/*se tem processo criado*/
                noAtual = (no*)l->primeiro;
                pAtual = noAtual->memoria;

                while(i < l->tamanho){
                        printf("%d\t",pAtual->pid);
                        printf("%s",pAtual->nome);
                        printf("\t\t");
                        printf("%d\t\t",pAtual->tempoExec);

                        x=mostraHora(pAtual);
                        printf("%d:%d:%d\t\t",x->tm_hour, x->tm_min, x->tm_sec);

			if(pAtual->estado){
                                printf("%s\n","executando");
                        }
                        else{
                                printf("%s\n","apto");
                        }
                        i++;
                        if(i < l->tamanho){/*Na ultima iteração não tem proximo, então não existe memória para ele*/
                                noAtual = (no*)noAtual->prox;
                                pAtual= noAtual->memoria;
                        }
                }
        }

}


void mostraTime(time_t *t){
        struct tm *time;
        time = gmtime(t);
        legenda();
        printf("\n%d:%d:%d\n", time->tm_hour, time->tm_min, time->tm_sec);

}

void shutdownProcessos(lista *l){
        int i=0;

        no *noAtual;
        processo *procAtual;
        if(l->tamanho){
                noAtual =(no*) l->primeiro;
                procAtual = noAtual->memoria;
                while(i < l->tamanho){
                        printf("\nFinalizando Processo\t%d\n",procAtual->pid);
                        if(noAtual->prox){/*Se for o último, não tem próximo*/
                                noAtual = (no*) noAtual->prox;
                                procAtual = noAtual->memoria;
                        }
                        i++;
                }
                liberalista(l);/*Exclui todos os processos da lista que fi passada*/
        }
}


