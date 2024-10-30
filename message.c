#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define TAM 256

typedef struct no 
{
    unsigned char caracter;
    int freq;
    struct no *esq, *dir, *prox;
} No;

typedef struct
{
    No* inicio;
    int tam;
} Lista;

//-------------------------------------------------- COMPACTAÇÃO --------------------------------------------------//

//Parte 1: Tabela de Frequência dos Bytes do arquivo

void inicializa_tabela (unsigned int tab[])
{
    for(int i = 0; i < TAM; i++) tab[i] = 0;
}

void preenche_tabela (FILE* file, unsigned int freqbytes[])
{
    unsigned char c;
    while(fread(&c, 1, 1, file) >= 1) freqbytes[(unsigned char)c]++;

    rewind(file);
}

//Parte 2: Lista Encadeada (Cada nó um byte do arquivo)

void inicializa_lista (Lista* lista)
{
    lista->inicio = NULL;
    lista->tam = 0;
}

void insercao_ordenada (Lista* lista, No* novo)
{
    if(lista->inicio == NULL) lista->inicio = novo;
    else if(novo->freq <= lista->inicio->freq)
    {
        novo->prox = lista->inicio;
        lista->inicio = novo;
    }
    else
    {
        No* aux = lista->inicio;
        while(aux->prox != NULL && aux->prox->freq < novo->freq) aux = aux->prox;

        novo->prox = aux->prox;
        aux->prox = novo;
    }

    lista->tam++;
}

void preenche_lista (unsigned int freqbytes[], Lista* lista)
{
    No* novo;
    
    for(int i = 0; i < TAM; i++)
    {
        if(freqbytes[i] > 0)
        {
            novo = (No*)malloc(sizeof(No));

            if(novo)
            {
                novo->caracter = i;
                novo->freq = freqbytes[i];
                novo->dir = NULL;
                novo->esq = NULL;
                novo->prox = NULL;

                insercao_ordenada(lista, novo);
            }
            else
            {
                printf("\tERRO AO PREENCHER LISTA\n");
                break;
            }
        }
    }
}

//Parte 3: Montando a Árvore de Huffman

No* remove_no (Lista* lista)
{
    No* aux = NULL;

    if(lista->inicio != NULL)
    {
        aux = lista->inicio;
        lista->inicio = aux->prox;

        aux->prox = NULL;
        lista->tam--;
    }

    return aux;
}

No* montar_arvore (Lista* lista)
{
    No *prim, *seg, *novo;

    while(lista->tam > 1)
    {
        prim = remove_no(lista);
        seg = remove_no(lista);

        novo = (No*)malloc(sizeof(No));

        if(novo)
        {
            novo->caracter = '*';
            novo->freq = prim->freq + seg->freq;

            novo->esq = prim;
            novo->dir = seg;
            novo->prox = NULL;

            insercao_ordenada(lista, novo);
        }
        else
        {
            printf("\tERRO AO MONTAR ÁRVORE DE HUFFMAN\n");
            break;
        }
    }

    return lista->inicio;
}

//Parte 4: Fazendo um dicionário

int altura_arvore (No* raiz)
{
    int esq, dir;
    
    if(raiz == NULL) return -1;
    else
    {
        esq = altura_arvore(raiz->esq) + 1;
        dir = altura_arvore(raiz->dir) + 1;

        if(esq > dir) return esq;
        else return dir;
    }
}

char** inicializa_dicionario (int colunas)
{
    char** dicionario = malloc(sizeof(char*)*TAM);
    for(int i = 0; i < TAM; i++) dicionario[i] = calloc(colunas, sizeof(char));
    return dicionario;
}

void preenche_dicionario (char** dicionario, No* raiz, char* caminho, int colunas)
{
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        strcpy(dicionario[raiz->caracter], caminho);
    }
    else
    {
        char esq[colunas];
        char dir[colunas];
        strcpy(esq, caminho);
        strcpy(dir, caminho);

        strcat(esq, "0");
        strcat(dir, "1");

        if (raiz->esq != NULL) preenche_dicionario(dicionario, raiz->esq, esq, colunas);
        if (raiz->dir != NULL) preenche_dicionario(dicionario, raiz->dir, dir, colunas);
    }
}

//Parte 5: Codificação do arquivo

int tamanho_string (FILE* file, char** dicionario)
{
    int tam = 0;
    
    unsigned char c;
    while(fread(&c, 1, 1, file) >= 1) tam += strlen(dicionario[(unsigned char)c]);

    rewind(file);

    return tam + 1;
}

char* codificar (FILE* file, char** dicionario)
{
    int tam = tamanho_string(file, dicionario);
    char* codigo = calloc(tam, sizeof(char));
    
    unsigned char d;
    while(fread(&d, 1, 1, file) >= 1) strcat(codigo, dicionario[(unsigned char)d]);

    return codigo;
}

//Parte 6: Gerando árvore em pré-ordem

int tam_pre_ordem (No* raiz)
{
    if(raiz == NULL) return 0;
    else 
    {
        if (raiz->esq == NULL && raiz->dir == NULL)
        {
            if(raiz->caracter == '*' || raiz->caracter == '\\') return 2;
            else return 1;
        } 
        else return (1 + tam_pre_ordem(raiz->esq) + tam_pre_ordem(raiz->dir));
    }
}

void pre_ordem (No* raiz, char apo[], int* i)
{
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        if(raiz->caracter == '*' || raiz->caracter == '\\')
        {
            apo[(*i)] = '\\';
            (*i)++;
        }

        apo[(*i)] = raiz->caracter;
        (*i)++;

        return;
    }
    else
    {
        apo[(*i)] = '*';
        (*i)++;

        if (raiz->esq != NULL) pre_ordem(raiz->esq, apo, i);
        if (raiz->dir != NULL) pre_ordem(raiz->dir, apo, i);
    }
}

//Parte 7: Calculando o tamanho do lixo e da string

void lixo (unsigned char* tam, unsigned char* trash, unsigned char codificado[], int qtd_nos)
{
    unsigned char temp;
    int tam_trash = 8 - (strlen(codificado)%8);

    if(qtd_nos > 255)
    {
        *tam = qtd_nos%256;
        temp = qtd_nos/256;

        *trash = tam_trash << 5;
        *trash = (*trash) | temp;
    }
    else
    {
        *tam = qtd_nos;
        *trash = tam_trash << 5;
    }
}

//Parte 8: Gerando arquivo compactado

void escreve_compactado (FILE* arquivo, char codificado[], char apo[], unsigned char* trash, unsigned char* tam, int qtd_nos)
{
    fwrite(trash, sizeof(unsigned char), 1, arquivo);
    fwrite(tam, sizeof(unsigned char), 1, arquivo);

    for(int pass = 0; pass < qtd_nos; pass++) 
    {
        fwrite(&apo[pass], sizeof(unsigned char), 1, arquivo);
    }

    int j = 7;
    unsigned char temp;
    unsigned char byte = 0;

    for(int i = 0; i < strlen(codificado); i++)
    {
        temp = 1;

        if(codificado[i] == '1')
        {
            temp = temp << j;
            byte = byte | temp;
        }

        j--;

        if(j < 0)
        {
            fwrite(&byte,sizeof(unsigned char),1,arquivo);
            byte = 0;
            j = 7;
        }
    }

    if(j != 7) fwrite(&byte, sizeof(unsigned char), 1, arquivo); 
}

//CABEÇA

void compactar ()
{
    //Parte 1: Tabela de Frequência dos Bytes do arquivo
    unsigned int freqbytes[TAM];
    FILE* file = fopen("entrada.in", "rb");
    if(!file)
    {
        printf("\tERRO AO ABRIR ARQUIVO\n");
        return;
    }
    inicializa_tabela(freqbytes);
    preenche_tabela(file, freqbytes);

    //Parte 2: Lista Encadeada (Cada nó um byte do arquivo)
    Lista lista;
    inicializa_lista(&lista);
    preenche_lista(freqbytes, &lista);

    //Parte 3: Montando a Árvore de Huffman
    No* arvore = montar_arvore(&lista);

    //Parte 4: Fazendo um dicionário
    char** dicionario;
    int colunas = altura_arvore(arvore) + 1;
    dicionario = inicializa_dicionario(colunas);
    preenche_dicionario(dicionario, arvore, "", colunas);

    //Parte 5: Codificação do arquivo
    char* codificado = codificar(file, dicionario);
    fclose(file);
    
    //Parte 6: Gerando árvore em pré-ordem
    int qtd_nos = tam_pre_ordem(arvore);
    unsigned char apo[qtd_nos];
    int temp = 0;
    pre_ordem(arvore, apo, &temp);

    //Parte 7: Calculando o tamanho do lixo e da string
    unsigned char tam;
    unsigned char trash = 0;
    lixo(&tam, &trash, codificado, qtd_nos);

    //Parte 8: Gerando arquivo compactado
    FILE* arquivo = fopen("compactado.huff", "wb");
    if(arquivo) escreve_compactado(arquivo, codificado, apo, &trash, &tam, qtd_nos);
    else printf("\tERRO AO CRIAR ARQUIVO COMPACTADO\n");
    fclose(arquivo);
}

//-------------------------------------------------- DESCOMPACTAÇÃO --------------------------------------------------//

//Parte 1: Guardando o tamanho do lixo e da árvore

unsigned int eh_bit_um (unsigned char byte, int i)
{
    unsigned char temp = (1 << i);

    return byte & temp;
}

void bytes_iniciais (FILE* arquivo, int* tam_lixo, int* tam_arvore)
{
    unsigned char trash, tam;
    fread(&trash, sizeof(unsigned char), 1, arquivo);
    fread(&tam, sizeof(unsigned char), 1, arquivo);

    int temp;
    int i;
    for(i = 7; i >= 5; i--)
    {
        if(eh_bit_um(trash, i))
        {
            temp = 1;
            for(int j = i - 5; j > 0; j--) temp = 2*temp;
            (*tam_lixo) += temp;
        }
    }

    for(i = 4; i >= 0; i--)
    {
        if(eh_bit_um(trash, i))
        {
            temp = 1;
            for(int j = i; j > 0; j--) temp = 2*temp;
            (*tam_arvore) += temp;
        }
    }

    (*tam_arvore) = (*tam_arvore)*256;
    for(i = 7; i >= 0; i--)
    {
        if(eh_bit_um(tam, i))
        {
            temp = 1;
            for(int j = i; j > 0; j--) temp = 2*temp;
            (*tam_arvore) += temp;
        }
    }
}

//Parte 2: Transformando a árvore em pré-ordem em uma árvore binária

void le_pre_ordem (FILE* arquivo, unsigned char apo[], int tam_pre_ordem)
{
    unsigned char byte;
    for(int i = 0; i < tam_pre_ordem; i++)
    {
        fread(&byte, sizeof(unsigned char), 1, arquivo);
        apo[i] = byte;
    }
}

No* cria_no (unsigned char caracter) 
{
    No* novo = (No*)malloc(sizeof(No));

    if (novo) 
    {
        novo->caracter = caracter;
        novo->esq = NULL;
        novo->dir = NULL;
        novo->prox = NULL;
    }
    else printf("\tERRO AO MONTAR ARVORE DE HUFFMAN\n");

    return novo;
}

No* desmonta_pre_ordem (unsigned char apo[], int* i, int tam_pre_ordem) 
{
    if (*i == tam_pre_ordem) return NULL;
    
    if (apo[*i] != '*')
    {
        if(apo[*i]=='\\') (*i)++;

        unsigned char filho = apo[(*i)]; 
        No* folha = cria_no(filho);
        (*i)++;

        return folha;
    }

    unsigned char atual = apo[(*i)];
    No* novo = cria_no(atual);

    (*i)++;
    novo->esq = desmonta_pre_ordem(apo, i, tam_pre_ordem);
    novo->dir = desmonta_pre_ordem(apo, i, tam_pre_ordem);

    return novo;
}

//Parte 3: Gerando arquivo descompactado

void gera_saida (FILE* saida, FILE* arquivo, No* raiz, int tam_lixo)
{
    int i;
    unsigned char byte, temp;
    No* aux = raiz;

    fread(&temp, sizeof(unsigned char), 1, arquivo);

    while(fread(&byte, sizeof(unsigned char), 1, arquivo) >= 1)
    {
        for(i = 7; i >= 0; i--)
        {
            if(eh_bit_um(temp, i)) aux = aux->dir;
            else aux = aux->esq;

            if(aux->esq == NULL && aux->dir == NULL)
            {
                fwrite(&(aux->caracter), sizeof(unsigned char), 1, saida);
                aux = raiz;
            }
        }

        temp = byte;
    }

    for(i = 7 - tam_lixo; i >= 0; i--)
    {
        if(eh_bit_um(temp, i)) aux = aux->dir;
        else aux = aux->esq;

        if(aux->esq == NULL && aux->dir == NULL)
        {
            fwrite(&(aux->caracter), sizeof(unsigned char), 1, saida);
            aux = raiz;
        }
    }
}

//CABEÇA
void descompactar ()
{
    //Parte 1
    FILE* arquivo = fopen("compactado.huff", "rb");
    int tam_lixo = 0;
    int tam_pre_ordem = 0;
    if(arquivo) bytes_iniciais(arquivo, &tam_lixo, &tam_pre_ordem);
    else printf("\tERRO AO ABRIR ARQUIVO COMPACTADO\n");

    //Parte 2
    int i = 0;
    unsigned char apo[tam_pre_ordem];
    le_pre_ordem(arquivo, apo, tam_pre_ordem);
    No* raiz = desmonta_pre_ordem(apo, &i, tam_pre_ordem);

    //Parte 3
    FILE* saida = fopen("saida.out", "wb");
    if(saida) gera_saida(saida, arquivo, raiz, tam_lixo);
    else printf("\tERRO AO GERAR ARQUIVO DESCOMPACTADO\n");
    fclose(saida);

    fclose(arquivo);
}

//-------------------------------------------------- MAIN --------------------------------------------------//

int main ()
{
    printf("SELECIONE O QUE DESEJA FAZER:\n\n");
    printf("(1) Compactar um arquivo\n");
    printf("(2) Descompactar um arquivo\n\n");

    int operacao;
    scanf("%d", &operacao);

    if(operacao == 1) compactar();
    else if(operacao == 2) descompactar();
}