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
    //frequência inicial dos bytes é zero
    for(int i = 0; i < TAM; i++) tab[i] = 0;
}

void preenche_tabela (FILE* file, unsigned int freqbytes[])
{
    //lê os bytes do arquivo e faz um acréscimo na posição correspondente do array
    unsigned char c;
    while(fread(&c, 1, 1, file) >= 1) freqbytes[(unsigned char)c]++;

    //Retorna o arquivo para o início
    rewind(file);
}

//Parte 2: Lista Encadeada (Cada nó um byte do arquivo)

void inicializa_lista (Lista* lista)
{
    //Inicialmente, a lista é nula
    lista->inicio = NULL;
    //A lista inicia vazia, tamanho zero
    lista->tam = 0;
}

void insercao_ordenada (Lista* lista, No* novo)
{
    //Se a lista está vazia, o nó adicionado é o início
    if(lista->inicio == NULL) lista->inicio = novo;
    else if(novo->freq <= lista->inicio->freq)
    {
        //Se o nó a ser adicionado tem frequência menor que o primeiro da lista

        //O nó aponta para o primeiro nó da lista
        novo->prox = lista->inicio;
        //O início da lista se torna o nó adicionado
        lista->inicio = novo;
    }
    else
    {
        //Auxiliar para caminhar através da lista
        No* aux = lista->inicio;
        //Enquanto a frequência do nó a ser adicionado for maior que a do seguinte ao auxiliar,
        //o auxiliar vai para o seguinte
        while(aux->prox != NULL && aux->prox->freq < novo->freq) aux = aux->prox;

        //o nó a ser adicionado aponta para o seguinte da lista
        novo->prox = aux->prox;
        //O auxiliar agora aponta para o nó adicionado
        aux->prox = novo;
    }

    //A lista aumenta de tamanho
    lista->tam++;
}

void preenche_lista (unsigned int freqbytes[], Lista* lista)
{
    No* novo;
    
    //Percorre a tabela de frequências
    for(int i = 0; i < TAM; i++)
    {
        if(freqbytes[i] > 0)
        {
            //Aloca memória para o nó se sua frequência for maior que 0
            novo = (No*)malloc(sizeof(No));

            if(novo)
            {
                //Guarda o caracter correspondente
                novo->caracter = i;
                //Guarda a frequência desse caracter
                novo->freq = freqbytes[i];
                //inicializa o nó apontando somente para NULL
                novo->dir = NULL;
                novo->esq = NULL;
                novo->prox = NULL;

                //Insere o nó na lista
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
        //Guarda o nó inicial da lista
        aux = lista->inicio;
        //O novo início da lista se torna o segundo nó dela
        lista->inicio = aux->prox;

        //O nó removido da lista agora aponta para NULL
        aux->prox = NULL;
        //A lista diminui de tamanho
        lista->tam--;
    }

    //Retorna o nó removido
    return aux;
}

No* montar_arvore (Lista* lista)
{
    No *prim, *seg, *novo;

    //Enquanto a lista não for um único nó correspondente à raiz da árvore, executa
    while(lista->tam > 1)
    {
        //Remove e guarda o primeiro nó da lista
        prim = remove_no(lista);
        //Remove e guarda o segundo nó da lista
        seg = remove_no(lista);

        //Cria um nó raiz
        novo = (No*)malloc(sizeof(No));

        if(novo)
        {
            //Caracter padrão para um nó raiz
            novo->caracter = '*';
            //A frequência do nó raiz corresponde à soma de seus filhos
            novo->freq = prim->freq + seg->freq;

            //Guarda o nó de maior frequência à esquerda
            novo->esq = prim;
            //Guarda o nó de menor frequência à direita
            novo->dir = seg;
            novo->prox = NULL;

            //Insere a árvore criada na lista
            insercao_ordenada(lista, novo);
        }
        else
        {
            printf("\tERRO AO MONTAR ÁRVORE DE HUFFMAN\n");
            break;
        }
    }

    //Retorna o único nó restante na lista, que é a raiz da árvore
    return lista->inicio;
}

//Parte 4: Fazendo um dicionário

int altura_arvore (No* raiz)
{
    int esq, dir;
    
    //Se chegou a um NULL, desconsidera parte da profundidade
    if(raiz == NULL) return -1;
    else
    {
        //Profundidade ao lado esquerdo do nó
        esq = altura_arvore(raiz->esq) + 1;
        //Profundidade ao lado direito do nó
        dir = altura_arvore(raiz->dir) + 1;

        //Retorna a maior profundidade entre as duas
        if(esq > dir) return esq;
        else return dir;
    }
}

char** inicializa_dicionario (int colunas)
{
    //Aloca memória para as colunas da matriz
    char** dicionario = malloc(sizeof(char*)*TAM);
    //Aloca memória para as linhas da matriz
    for(int i = 0; i < TAM; i++) dicionario[i] = calloc(colunas, sizeof(char));

    return dicionario;
}

void preenche_dicionario (char** dicionario, No* raiz, char* caminho, int colunas)
{
    //É um nó folha
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        //Coloca na linha correspondente o caminho para chegar ao nó folha
        strcpy(dicionario[raiz->caracter], caminho);
    }
    //É um nó raiz
    else
    {
        //Copia o caminho atual para duas strings
        char esq[colunas];
        char dir[colunas];
        strcpy(esq, caminho);
        strcpy(dir, caminho);

        //Adiciona 0 para ir à esquerda
        strcat(esq, "0");
        //Adiciona 1 para ir à direita
        strcat(dir, "1");

        //Segue o caminho para a esquerda
        if (raiz->esq != NULL) preenche_dicionario(dicionario, raiz->esq, esq, colunas);
        //Segue o caminho para a direita
        if (raiz->dir != NULL) preenche_dicionario(dicionario, raiz->dir, dir, colunas);
    }
}

//Parte 5: Codificação do arquivo

int tamanho_string (FILE* file, char** dicionario)
{
    //inicia o tamanho com 0
    int tam = 0;
    
    //Adiciona ao tamanho total o tamanho do caminho de cada byte presente no arquivo
    unsigned char c;
    while(fread(&c, 1, 1, file) >= 1) tam += strlen(dicionario[(unsigned char)c]);

    //Retorna o arquivo para o início
    rewind(file);

    return tam + 1;
}

char* codificar (FILE* file, char** dicionario)
{
    //Calcula o tamanho da string
    int tam = tamanho_string(file, dicionario);
    //Aloca memória para guardar a string codificada
    char* codigo = calloc(tam, sizeof(char));
    
    //Insere no fim da string o código que corresponde ao caracter lido
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
        //Se for nó folha, executa
        if (raiz->esq == NULL && raiz->dir == NULL)
        {
            //Se for * ou \, precisa de dois espaços na string para ser colocado
            if(raiz->caracter == '*' || raiz->caracter == '\\') return 2;
            //Caso não, precisa de um único espaço
            else return 1;
        } 
        //Se for nó raiz, guarda um espaço para esse nó e segue andando na árvore
        else return (1 + tam_pre_ordem(raiz->esq) + tam_pre_ordem(raiz->dir));
    }
}

void pre_ordem (No* raiz, char apo[], int* i)
{
    //Se for um nó folha, executa
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        //Se o caracter for * ou \, acrescente um \ para indicar que é um nó folha
        if(raiz->caracter == '*' || raiz->caracter == '\\')
        {
            apo[(*i)] = '\\';
            (*i)++;
        }

        //Insere o caracter na string
        apo[(*i)] = raiz->caracter;
        (*i)++;

        return;
    }
    //Se for um nó raiz, executa
    else
    {
        //Coloca um * para indicar que é nó raiz
        apo[(*i)] = '*';
        (*i)++;

        //Segue a árvore para a esquerda
        if (raiz->esq != NULL) pre_ordem(raiz->esq, apo, i);
        //Segue a árvore para a direita
        if (raiz->dir != NULL) pre_ordem(raiz->dir, apo, i);
    }
}

//Parte 7: Calculando o tamanho do lixo e da string

void lixo (unsigned char* tam, unsigned char* trash, unsigned char codificado[], int qtd_nos)
{
    unsigned char temp;
    //O tamanho do lixo é quanto falta pra completar o último byte
    int tam_trash = 8 - (strlen(codificado)%8);

    if(qtd_nos > 255)
    {
        //8 bits finais do tamanho
        *tam = qtd_nos%256;
        //5 bits iniciais do tamanho
        temp = qtd_nos/256;

        //Coloca o tamanho do lixo nos 3 primeiros bits
        *trash = tam_trash << 5;
        //Junta os 3 bits do lixo com os 5 bits iniciais do tamanho
        *trash = (*trash) | temp;
    }
    else
    {
        //byte do tamanho
        *tam = qtd_nos;
        //Coloca o tamanho do lixo nos 3 primeiros bits
        *trash = tam_trash << 5;
    }
}

//Parte 8: Gerando arquivo compactado

void escreve_compactado (FILE* arquivo, char codificado[], char apo[], unsigned char* trash, unsigned char* tam, int qtd_nos)
{
    //Escreve no início do arquivo os bytes de lixo e de tamanho da árvore
    fwrite(trash, sizeof(unsigned char), 1, arquivo);
    fwrite(tam, sizeof(unsigned char), 1, arquivo);

    //Escreve a árvore em pré-ordem
    for(int pass = 0; pass < qtd_nos; pass++) 
    {
        fwrite(&apo[pass], sizeof(unsigned char), 1, arquivo);
    }

    int j = 7;
    unsigned char temp;
    unsigned char byte = 0;

    //Insere os bytes codificados no arquivo
    for(int i = 0; i < strlen(codificado); i++)
    {
        temp = 1;

        if(codificado[i] == '1')
        {
            //Manda o bit para seu devido lugar no byte
            temp = temp << j;
            //Completa o byte com o que já tinha anteriormente
            byte = byte | temp;
        }
        //Se for 0, já é inserido automaticamente no byte

        j--;

        //Verifica se já formou um byte
        if(j < 0)
        {
            //Insere o byte no arquvivo
            fwrite(&byte,sizeof(unsigned char),1,arquivo);

            //Reseta as configurações para inserir o próximo byte
            byte = 0;
            j = 7;
        }
    }

    //Insere os bits restantes caso não tenha conseguido completar um byte
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
    //Seleciona o bit a ser analisado
    unsigned char temp = (1 << i);

    //Verifica se o bit é 1
    return byte & temp;
}

void bytes_iniciais (FILE* arquivo, int* tam_lixo, int* tam_arvore)
{
    unsigned char trash, tam;
    //Guarda o primeiro byte (lixo)
    fread(&trash, sizeof(unsigned char), 1, arquivo);
    //Guarda o segundo byte (tamanho da árvore)
    fread(&tam, sizeof(unsigned char), 1, arquivo);

    int temp;
    int i;
    
    //Pega os três primeiros bits do byte de lixo
    for(i = 7; i >= 5; i--)
    {
        if(eh_bit_um(trash, i))
        {
            //Converte o bit de binário para decimal
            temp = 1;
            for(int j = i - 5; j > 0; j--) temp = 2*temp;

            //Guarda o tamanho do lixo
            (*tam_lixo) += temp;
        }
    }

    //Pega os cinco últimos bits do byte de lixo
    for(i = 4; i >= 0; i--)
    {
        if(eh_bit_um(trash, i))
        {
            //Converte o bit de binário para decimal
            temp = 1;
            for(int j = i; j > 0; j--) temp = 2*temp;

            //Guarda a primeira parte do tamanho da árvore
            (*tam_arvore) += temp;
        }
    }

    (*tam_arvore) = (*tam_arvore)*256;

    //Pega o byte da árvore
    for(i = 7; i >= 0; i--)
    {
        if(eh_bit_um(tam, i))
        {
            //Coverte o bit de binário para decimal
            temp = 1;
            for(int j = i; j > 0; j--) temp = 2*temp;
            
            //Acresce a segunda parte ao tamanho da árvore
            (*tam_arvore) += temp;
        }
    }
}

//Parte 2: Transformando a árvore em pré-ordem em uma árvore binária

void le_pre_ordem (FILE* arquivo, unsigned char apo[], int tam_pre_ordem)
{
    //Guarda a árvore em pré-ordem em uma string
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
    //Se o índice chegou no tamanho da árvore, finaliza
    if (*i == tam_pre_ordem) return NULL;
    
    //Se a string tiver um caracter diferente de *, encontrou um nó folha
    if (apo[*i] != '*')
    {
        //Se o caracter lido foi \, o caracter do nó é o caracter seguinte 
        if(apo[*i]=='\\') (*i)++;

        //Cria um nó para a árvore com o caracter lido
        unsigned char filho = apo[(*i)]; 
        No* folha = cria_no(filho);
        (*i)++;

        return folha;
    }

    //Cria um nó raiz
    unsigned char atual = apo[(*i)];
    No* novo = cria_no(atual);
    (*i)++;

    //O nó aponta à esquerda para o nó gerado com o próximo caracter lido
    novo->esq = desmonta_pre_ordem(apo, i, tam_pre_ordem);
    //O nó aponta à direita para o nó gerado com o caracter lido após a esquerda encontrar um nó folha
    novo->dir = desmonta_pre_ordem(apo, i, tam_pre_ordem);

    return novo;
}

//Parte 3: Gerando arquivo descompactado

void gera_saida (FILE* saida, FILE* arquivo, No* raiz, int tam_lixo)
{
    int i;
    unsigned char byte, temp;
    No* aux = raiz;

    //Lê o primeiro byte do arquivo codificado
    fread(&temp, sizeof(unsigned char), 1, arquivo);

    while(fread(&byte, sizeof(unsigned char), 1, arquivo) >= 1)
    {
        for(i = 7; i >= 0; i--)
        {
            //Caminha na árvore de acordo com o bit lido
            if(eh_bit_um(temp, i)) aux = aux->dir;
            else aux = aux->esq;

            //Se encontrou um nó folha, executa
            if(aux->esq == NULL && aux->dir == NULL)
            {
                //Escreve no arquivo de saída o caracter encontrado no nó folha
                fwrite(&(aux->caracter), sizeof(unsigned char), 1, saida);
                //Reseta para encontrar o próximo caracter
                aux = raiz;
            }
        }

        //Vai para o byte seguinte e lê o próximo
        temp = byte;
    }

    //Lê o último byte, ignorando o que é lixo
    for(i = 7; i >= tam_lixo; i--)
    {
        //Caminha na árvore de acordo com o bit lido
        if(eh_bit_um(temp, i)) aux = aux->dir;
        else aux = aux->esq;

        //Se encontrou um nó folha, executa
        if(aux->esq == NULL && aux->dir == NULL)
        {
            //Escreve no arquivo de saída o caracter encontrado no nó folha
            fwrite(&(aux->caracter), sizeof(unsigned char), 1, saida);
            //Reseta para encontrar o próximo caracter
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
