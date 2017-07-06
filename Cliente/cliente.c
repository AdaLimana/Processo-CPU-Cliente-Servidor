/*
  Programa Cliente, responsavel por "executar"
  os processos criados no Servidor. O programa
  Cliente simula um processador com N nucleos,
  o numero N eh definido pela constante QNT_CHAMADAS.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/*bibliotecas para comunicacao via SOCKET*/
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

/*biblioteca para uso de THREAD*/
#include <pthread.h>

#define QNT_CHAMADAS 2

/*
  Estrutura passada como parametro na thread
  (funcao nucleo),para informar a porta e ip
  da conexao.
*/
typedef struct{
	int porta;
	char ip[30];
}endereco;

/*
  Assinatura da funcao nucleo, funcao executada
  na thread criada em paralelo com a thread
  principal main.
*/
void *nucleo(void *x);

/*
  Assinatura da funcao zeraString
*/


int main(int argc, char *argv[]){

	endereco end;/*Estrutura passada para a thread (nucleo)*/
	pthread_t thread[QNT_CHAMADAS];/*identificador da thread*/
	int chamada = 0;/*Contador de chamadas criadas*/
	
	if(argc < 2 ){/*Se o argumento passado por linha de comando for invalido finaliza a execucao*/
		printf("\nargumentos invalidos\n");
		return 0;
	}

	end.porta = atoi(argv[1]);	/*converte o primeiro agurmento referente a porta da conexao
					com o servidor, que eh um vetor de char, em inteiros*/

	strcpy(end.ip, argv[2]);/*Copia o segundo argumento, que eh o ip do servidor*/
	
	while(chamada < QNT_CHAMADAS){/*Cria QNT_CHAMADAS threads*/

		if(pthread_create(&thread[chamada], NULL, nucleo, (void *) &end)){	/*Cria uma thread e armazena em*/
										 	/*thread[chamada] o identificador*/
			printf("\nErro ao criar a thread\n");				/*da thread criada. A thread criada*/
			exit(1);							/*executara a fincao (nucleo)*/
		}
		chamada ++;
	}
	chamada = 0;

	while(chamada < QNT_CHAMADAS){/*Une o fluxo das theads para finalizar*/
		
		pthread_join(thread[chamada], NULL);/*Aguarda a thread do indentificador (thread[chamada]) finalizar*/
		chamada++;
	}
        return 0;
}

void *nucleo(void *x){

	char mensagem[100];/*vetor que armazena a mensagem enviada/recebida via socket*/

	endereco *e = (endereco *)x;/*converte o endereco apontado do x para seu tipo de origem (endereco)*/

	int sockfd; /*Identifica o socket cliente criado*/
        
	struct sockaddr_in cliente;/*Estrutura padrao para socket*/
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0); /*Criando o socket*/

	/*Setando o socket*/
        cliente.sin_family = AF_INET;
        cliente.sin_port = htons(e->porta);
        cliente.sin_addr.s_addr = inet_addr(e->ip);

        /*Inicia a conexão com o servidor*/
         if(connect(sockfd, (struct sockaddr*)&cliente, sizeof(cliente)) < 0){
                perror("Erro na conexão");
                exit(1);
        }
        memset(mensagem, '\0', 100);/*Preenche com '\0' a string*/
        recv(sockfd, mensagem, 100, 0);/*Aguarda o recebimento de uma mensagem do Servidor*/
        puts(mensagem);


        memset(mensagem, '\0',100);/*Preenche com '\0' a string*/
        recv(sockfd, mensagem,100,0);/*Aguarda o recebimento de uma mensagem do Servidor*/
        puts(mensagem);
	pthread_exit(NULL);/*Finaliza a thread*/

}
