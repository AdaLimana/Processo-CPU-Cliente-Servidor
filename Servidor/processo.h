#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct{
    int pid;
    char *nome;
    time_t criacao;
    int tempoExec;
    int estado;
    int kill;/*setada para um quando o processo a ser matado é o que está "executando" na thread*/
}processo;


processo* criaProcesso(int pid, char *nomeProcesso, int tempoExecProcesso){
  processo *novoProcesso = (processo*)malloc(sizeof(processo));

  if(!novoProcesso){/*Caso não cria processo retorna erro*/
      return novoProcesso;
  }
  novoProcesso->pid = pid;

  novoProcesso->tempoExec = tempoExecProcesso;
  time(&novoProcesso->criacao);/*armazena a hora que foi criado*/

  /*Aloca dinamicamente uma string para o nome processo*/
  novoProcesso->nome = (char*) malloc(strlen(nomeProcesso)*sizeof(char));
  strcpy(novoProcesso->nome, nomeProcesso);
  novoProcesso->estado = 0;/*quando é criado fica como zero, significa que está apto, e não execução*/
  novoProcesso->kill = 0;/*setado para 1 quando o processo que está em execução recebe um kill*/
  return novoProcesso;
}

/*char * mostraHora(processo *processo){ctime retorna uma string com dia mes ano h m s
    return ctime(&processo->criacao);
}*/

struct tm* mostraHora(processo *processo){/*retorna uma estrutura com */
	return gmtime(&processo->criacao);/*dia mes ano h m s */
}					  /*ai só mostra o que quiser*/
					  /*no caso h:m:s*/
