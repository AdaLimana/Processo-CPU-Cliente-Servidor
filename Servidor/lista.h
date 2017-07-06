
#include "processo.h"
#include<stdio.h>

typedef struct{
	processo *memoria;
	int *prox;
}no;

typedef struct{

	int *primeiro;
	int *ultimo;
	int tamanho;
}lista;

void inicializa(lista *l){

	l->primeiro=NULL;
	l->ultimo=NULL;
	l->tamanho=0;
}

int tamanho(lista *l){

	return l->tamanho;
}

no* insereinicio(lista *l, processo *novo){

	no *noinsere;
	noinsere=(no *)malloc(sizeof(no));
	if(!noinsere)
		return noinsere;

	noinsere->memoria=novo;
	noinsere->prox=l->primeiro;
	l->primeiro=(int *)noinsere;
	if(l->tamanho==0)
		l->ultimo=(int *)noinsere;

	l->tamanho++;
	return noinsere;
}

no* inserefim(lista *l, processo *novo){
	no *noinsere;
	noinsere=(no *)malloc(sizeof(no));
	if(!noinsere)
		return noinsere;
	noinsere->memoria=novo;
	noinsere->prox=NULL;


	if(l->tamanho==0){
		l->primeiro=(int *) noinsere;
		l->ultimo=(int *)noinsere;


	}
	else{
		no *penultimo;
		penultimo=(no *)l->ultimo;
		l->ultimo=(int *)noinsere;
		penultimo->prox=(int *)noinsere;

	}
	printf(" ");
	l->tamanho++;
	return noinsere;
}

no* insereem(lista *l, processo *novo, int posicao){
	if((posicao-1)>(tamanho(l)))
		return NULL;

	int i=1;
	no *alocano, *apontador, *apontado;

	apontador=(no *)l->primeiro;
	apontado=apontador;

	while(i<posicao){
		apontador=apontado;
		apontado=(no *)apontador->prox;
		i++;
	}

	if(i==1){

		alocano= (no *) malloc(sizeof(no));
		if(!alocano)
			return alocano;

		alocano->prox=(int *)apontador;
		alocano->memoria=novo;
		l->primeiro=(int *)alocano;
		l->ultimo=(int *)alocano;
		l->tamanho++;
	}
	else{

		alocano= (no *) malloc(sizeof(no));
		if(!alocano)
			return alocano;

		apontador->prox=(int *)alocano;
		alocano->prox=(int *)apontado;
		alocano->memoria=novo;

		if(apontado==0)
			l->ultimo=(int *)alocano;

		l->tamanho++;
	}
	return alocano;
}

int excluiInicio(lista *l){

	no *excluido;
	no *novoPrimeiro;

	excluido = (no *) l->primeiro;

	if(!excluido){

		return 0;
	}

	novoPrimeiro = (no *) excluido->prox;
	l->primeiro = (int *) novoPrimeiro;
	l->tamanho--;
        free(excluido->memoria);
	free(excluido);
	return 1;

}

no* tiraInicio(lista *l){/*em vez de excluir ele tira da lista*/
	no *tira;	/* o nó não pertence mais a lista no entanto o end não é excluido*/
        no *novoPrimeiro;

        tira = (no *) l->primeiro;

        if(!tira){

                return tira;
        }
	l->tamanho--;
	novoPrimeiro = (no *) tira->prox;
        l->primeiro = (int *) novoPrimeiro;

        return tira;

}

int excluiEm(lista *l, int posicao){
	int i = 1;
	no* atual;
	no* prox;
	no* ante;
	atual = (no*)l->primeiro;

	while(i < posicao){/*Acha a posição o NÓ que será excluido*/
		prox = (no*) atual->prox;
		ante = atual;
		atual = prox;
		i++;
	}

	if(i == 1){/*se for verdade o excluio é o primeiro*/
		if(!excluiInicio(l)){
			return 0;
		}
		//printf("\nEXCLUIU PRIMEIRO\n");
		return 1;

	}

	if(!atual->prox){/*se for verdade o excluido é o último*/
		//printf("\nEXCLUIU ULTIMO\n");
		free(atual->memoria);
        	free(atual);
		ante->prox = NULL;
		l->ultimo = (int *) ante;
		l->tamanho--;
		return 1;
	}
	//printf("\nEXCLUIU MEIO\n");
	prox = (no*)atual->prox;
	ante->prox = (int*)prox;
	free(atual->memoria);
	free(atual);
	l->tamanho--;
	return 1;

}


void liberalista(lista *l){
	no *noatual;
	no *proximo;
	noatual=(no *)l->primeiro;
	while(noatual){
		proximo=(no *)noatual->prox;
                free(noatual->memoria);
		free(noatual);
		noatual=proximo;
	}

	//inicializa(l);
}

no* retornaNoPosicao(lista *l, int posicao){
	int i=1;
	no *atual;
	no *prox;
	atual = (no*) l->primeiro;
	prox = atual;
	while(i<posicao){

		prox = (no*) atual->prox;
		atual = prox;
		i++;

	}
	return atual;
}

int invertelista(lista *l){
	if(tamanho(l)<2){
		//printf("\n###########################\n");
		return 0;
	}
	no *apontado, *apontador, *aux_primeiro, *aux_prox;

	apontado=(no *)l->primeiro;
	aux_primeiro=apontado;
	apontador=(no *)apontado->prox;
	apontado->prox=NULL;
	aux_prox=(no *)apontador->prox;
	apontador->prox=(int *)apontado;
	apontado=apontador;

	while(aux_prox){

		apontador=aux_prox;
		aux_prox=(no *)apontador->prox;
		apontador->prox=(int *)apontado;
		apontado=apontador;
	}
	l->ultimo=(int *)aux_primeiro;
	l->primeiro=(int *)apontado;
	return 1;
}

void print(lista *l){
	printf("\nEnd inicial= %x \t end fina= %x,\t tamanho lista= %d\n",l->primeiro,l->ultimo,l->tamanho);
	int nos=1;
        processo *x;
	no *noatual;
	noatual=(no *)l->primeiro;
	while(noatual){
            x=noatual->memoria;
		printf("\nNo num= %d,\tend no= %x,\telemento no= %d,\tend proximo no= %x\n",nos,noatual,x->pid,noatual->prox);
		noatual=(no *)noatual->prox;
		nos++;
	}
}
