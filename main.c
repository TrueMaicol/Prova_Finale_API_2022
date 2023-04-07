#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 78
#define dimBuffer 4096

typedef struct {
    int notIn;
    struct indexNot *next;
} indexNot;

typedef struct {
    int exact,min,count,abilitato;
    indexNot *posNonAmmi;
} vincolo;

typedef struct {
    int n_i,c_i,t,abilitato;
} carattere;

typedef struct {
    char *p;
    int hash;
    struct parolaAmmissibile *next;
} parolaAmmissibile;


int hashFunction(char i) {
    return (i - '-');
}

int creaHash(char *p,int dimParola) {
    int i = 0,hash = 0;
    while(i < dimParola) {
        hash += p[i];
        i++;
    }
    return hash;
}

void setAuxDefault(char *aux, int dim) {
    for(int i=0; i<dim; i++) {
        aux[i] = '&';
    }
}

void setVettoriDefault(carattere vectR[], vincolo vectV[]) {
    for(int i=0; i<N; i++) {
        vectV[i].abilitato = 0;
        vectV[i].exact = -1;
        vectV[i].count = 0;
        vectV[i].min = 0;
        vectV[i].posNonAmmi = NULL;
        vectR[i].abilitato = 0;
    }
}

void deallocaListaIndexNot(indexNot *lista) {
    if(lista != NULL) {
        deallocaListaIndexNot((indexNot *) lista->next);
        free(lista);
    }
}

void deallocaListeIndexNot(vincolo vectV[]) {
    for(int i=0; i<N; i++)
        deallocaListaIndexNot(vectV[i].posNonAmmi);
}

void deallocaFiltrate(parolaAmmissibile *lista) {
    if(lista != NULL) {
        deallocaFiltrate((parolaAmmissibile *) lista->next);
        free(lista);
    }

}

int *gestisciParolaRiferimento(char *r, carattere vect[], int *vectK) {
    char tmp;
    int i=0,k;
    tmp = r[0];
    while(tmp != '\0') {
        k = hashFunction(tmp);
        vectK[i] = k;
        if(vect[k].abilitato == 0) {
            vect[k].n_i = 1;
            vect[k].c_i = 0;
            vect[k].t = 0;
            vect[k].abilitato = 1;
        } else {
            vect[k].n_i++;
        }
        i++;
        tmp = r[i];
    }
    return vectK;
}

int ricercaP(parolaAmmissibile *iniziali, parolaAmmissibile *filtrate, char *p, int dimParola) {
    int hash_p;
    parolaAmmissibile *tmp;
    tmp = filtrate;
    hash_p = creaHash(p,dimParola);
    while(tmp != NULL) {
        if(tmp->hash == hash_p) {
            if(strcmp(tmp->p,p) == 0) {
                return 1;
            } else {
                tmp = (parolaAmmissibile *) tmp->next;
            }
        } else {
            tmp = (parolaAmmissibile *) tmp->next;
        }
    }
    tmp = iniziali;
    while(tmp != NULL) {
        if(tmp->hash == hash_p) {
            if(strcmp(tmp->p,p) == 0) {
                return 1;
            } else {
                tmp = (parolaAmmissibile *) tmp->next;
            }
        } else {
            tmp = (parolaAmmissibile *) tmp->next;
        }
    }
    return 0;
}

int ricercaPosizioneInListaNonAmmissibile(indexNot *lista, int posizioneCorrente) { //0 se posizioneCorrente è presente in lista, 1 altrimenti
    indexNot *tmp;
    tmp = lista;
    while(tmp != NULL) {
        if(tmp->notIn == posizioneCorrente) {
            return 0;
        }
        tmp = (indexNot *) tmp->next;
    }
    return 1;
}

indexNot *inserisciInListaIndexNot(indexNot *lista, int daInserire) {
    indexNot *tmp;
    if(ricercaPosizioneInListaNonAmmissibile(lista,daInserire) == 1) {
        tmp = (indexNot*)calloc(1,sizeof(indexNot));
        tmp->notIn = daInserire;
        tmp->next = (struct indexNot *) lista;
        return tmp;
    } else {
        return lista;
    }
}

int eseguiConfronto(parolaAmmissibile **iniziali, parolaAmmissibile *filtrate,char *res,char *r, char *p, int dimParola, carattere vectR[],vincolo vectV[], char *aux) {
    int k,hashTmp;
    hashTmp = hashFunction(p[0]);
    if(ricercaP(iniziali[hashTmp],filtrate,p,dimParola) == 0) {
        return 0;
    } else {
        if(strcmp(r,p) == 0) {
            return 2;
        }
        setAuxDefault(res,dimParola);
        for(int j=0; j<dimParola; j++) { //vado a identificare i caratteri in posizione corretta
            if(p[j] == r[j]) {
                res[j] = '+';
                aux[j] = p[j]; //costruiamo la stringa ausiliaria per i vincoli
                k = hashFunction(p[j]);
                vectR[k].c_i++;
                //devo creare la struttura vincolo da associare alla casella k
                if(vectV[k].abilitato == 0) {
                    vectV[k].abilitato = 1;
                }
                vectV[k].count++;
            }
        }
        for(int j=0; j<dimParola; j++) {
            k = hashFunction(p[j]);
            if(res[j] != '+') {  //abbiamo così gestito le posizioni corrette (non ancora creati i vincoli, da inserire poi), vect[j]->t conta la frequenza di p[i] scorrette
                if(vectV[k].abilitato == 0) {
                    vectV[k].abilitato = 1;
                }
                if(vectR[k].abilitato == 0) {   //se non ho mai incontrato questo carattere
                    res[j] = '/';
                    vectV[k].exact = 0;
                } else {    // se t >= (ni-ci) allora /, altrimenti |
                    if(vectR[k].t >= (vectR[k].n_i - vectR[k].c_i)) {
                        res[j] = '/';
                        vectV[k].posNonAmmi = inserisciInListaIndexNot(vectV[k].posNonAmmi,j);
                        vectV[k].exact = vectV[k].count; // e se usassi al posto che count ni?
                    } else {
                        res[j] = '|';
                        //devo inserire j nella lista delle posizioni in cui il carattere k non può essere presente, se tale posizione è già presente in lista non fare nulla
                        vectV[k].posNonAmmi = inserisciInListaIndexNot(vectV[k].posNonAmmi,j);
                        vectV[k].count++;
                    }
                    vectR[k].t++;
                }
            }
        }
        for(int j=0; j<dimParola; j++) {
            k = hashFunction(p[j]);
            if(vectV[k].abilitato == 1) {
                if(vectV[k].count >= vectV[k].min) {
                    vectV[k].min = vectV[k].count;
                }
                vectV[k].count = 0;
            }
            if(vectR[k].abilitato == 1) {
                vectR[k].c_i = 0;
                vectR[k].t = 0;
            }
        }
        return 1;
    }
}

int confrontaVincoli(char *p,vincolo vectV[], char *aux, int dimParola, int *vectK) {
    char tmp;
    int i=0,ammissibile,k,vectCount[N],vectKdaFiltrare[dimParola];
    ammissibile = 1;
    for(int j=0; j<dimParola; j++) {
        k = hashFunction(p[j]);
        vectKdaFiltrare[j] = k;
        vectCount[k] = 0;

        vectCount[vectK[j]] = 0;

    }

    while(i < dimParola) {
        tmp = p[i];


        if(tmp == aux[i]) {
            ammissibile = 1;
            vectCount[vectKdaFiltrare[i]]++;
        } else if(aux[i] != '&') { //se è diverso da & => sappiamo che c'è un carattere in posizione corretta che è diverso da quello visto nella parola => non ammissibile
            return 0;
        } else if(aux[i] == '&') {
            if(vectV[vectKdaFiltrare[i]].abilitato == 1) {  //se io non ho MAI incontrato il carattere p[i] vuol dire che non esiste vincolo allora vado avanti senza fare nulla
                if(vectV[vectKdaFiltrare[i]].exact == 0) {  //vuol dire che tale carattere sappiamo non esistere in r => parola non ammissibile
                    return 0;
                } else {
                    if(ricercaPosizioneInListaNonAmmissibile(vectV[vectKdaFiltrare[i]].posNonAmmi,i) == 0) {
                        return 0;
                    } else {
                        //la posizione corrente è una posizione ammissibile per questo carattere di hash k
                        vectCount[vectKdaFiltrare[i]]++;
                    }
                }
            }
        }
        i++;
    }

    for(int j=0; j<dimParola; j++) {

        if(vectV[vectK[j]].abilitato == 1) {
            if(vectV[vectK[j]].exact == -1) {
                if(vectCount[vectK[j]] >= vectV[vectK[j]].min) {
                    ammissibile = 1;
                } else {
                    return 0;
                }
            } else {
                if(vectV[vectK[j]].exact == vectCount[vectK[j]]) {
                    ammissibile = 1;
                } else {
                    return 0;
                }
            }
        } //se non è abilitato non so nulla riguardo a tale lettera
        if(vectV[vectKdaFiltrare[j]].abilitato == 1) {
            if(vectV[vectKdaFiltrare[j]].exact == -1) {
                if(vectCount[vectKdaFiltrare[j]] >= vectV[vectKdaFiltrare[j]].min) {
                    ammissibile = 1;
                } else {
                    return 0;
                }
            } else {
                if(vectV[vectKdaFiltrare[j]].exact == vectCount[vectKdaFiltrare[j]]) {
                    ammissibile = 1;
                } else {
                    return 0;
                }
            }
        } //se non è abilitato non so nulla riguardo a tale lettera
    }
    return ammissibile;
}

void DividiLista(parolaAmmissibile *lista,parolaAmmissibile **x, parolaAmmissibile **y) {
    parolaAmmissibile *singolo,*doppio;
    singolo = lista;
    doppio = (parolaAmmissibile *) lista->next;

    while(doppio != NULL) {
        doppio = (parolaAmmissibile *) doppio->next;
        if(doppio != NULL) {
            singolo = (parolaAmmissibile *) singolo->next;
            doppio = (parolaAmmissibile *) doppio->next;
        }
    }
    *x = lista;
    *y = (parolaAmmissibile *) singolo->next;
    singolo->next = NULL;

}

parolaAmmissibile *Merge(parolaAmmissibile *x, parolaAmmissibile *y) {
    parolaAmmissibile *ordinata;
    ordinata = NULL;
    int confronto;
    if(x == NULL) { //vuol dire che ho già inserito l'altra lista che voglio concatenare
        return y;
    } else if(y == NULL) {
        return x;
    }
    confronto = strcmp(x->p,y->p);
    if(confronto < 0) { //x viene prima nell'ordine ascii
        ordinata = x;
        ordinata->next = (struct parolaAmmissibile *) Merge((parolaAmmissibile *) x->next, y);
    } else if(confronto > 0) {
        ordinata = y;
        ordinata->next = (struct parolaAmmissibile *) Merge(x, (parolaAmmissibile *) y->next);
    }
    return ordinata;
}

void MergeSort(parolaAmmissibile **lista) {
    parolaAmmissibile *testa,*x,*y;
    testa = *lista;
    if(testa == NULL || testa->next == NULL) {
        return;
    }
    DividiLista(testa,&x,&y);
    MergeSort(&x);
    MergeSort(&y);
    *lista = Merge(x,y);
}


void stampaFiltrate(parolaAmmissibile **iniziali,parolaAmmissibile *filtrate, int numConfronto) {
    parolaAmmissibile *tmp;
    if(numConfronto == 0) { //se non abbiamo ancora eseguito nessun confronto => devo stampare l'intera lista delle ammissibili perchè non ho vincoli
        //devo ordinare ogni lista, perchè adesso ho un vettore di liste, ogni cella corrisponde ad una lista con parole tutte con una certa iniziale
        for(int i=0; i<N; i++) {
            if(iniziali[i] != NULL) {
                MergeSort(&iniziali[i]);
                tmp = iniziali[i];
                while(tmp != NULL) {
                    printf("%s\n",tmp->p);
                    tmp = (parolaAmmissibile *) tmp->next;
                }
            }

        }
    } else {    //ho già fatto dei confronti ed esisteranno dei vincoli, stampa le filtrate
        tmp = filtrate;
        while(tmp != NULL) {
            printf("%s\n", tmp->p);
            tmp = (parolaAmmissibile *) tmp->next;
        }
    }
}

void filtraLista(parolaAmmissibile **iniziali,parolaAmmissibile **filtrate, vincolo vectV[],char *aux, int dimParola, int numCofronto, int *totFiltrate, int *vectK) {
    parolaAmmissibile *prev,*curr;
    int k;
    *totFiltrate = 0;
    prev = NULL;

    if(numCofronto == 1) {  //siamo al primo confronto => lista è quella intera
        for(int i=0; i<N; i++) {
            prev = NULL;
            curr = iniziali[i];
            while(curr != NULL) {
                if(confrontaVincoli(curr->p,vectV,aux,dimParola,vectK) == 1) { // 1 significa che è ammissibile
                    //devi spostare da ammissibili a filtrate, lo spostamento lo faccio in testa alla lista delle filtrate per semplicità e poi riordino la lista
                    if(prev == NULL) {  //sono sulla testa di una delle liste delle iniziali
                        iniziali[i] = (parolaAmmissibile *) curr->next;
                        curr->next = (struct parolaAmmissibile *) *filtrate;
                        *filtrate = curr;
                        curr = iniziali[i];
                    } else {
                        prev->next = curr->next;
                        curr->next = (struct parolaAmmissibile *) *filtrate;
                        *filtrate = curr;

                        curr = (parolaAmmissibile *) prev->next;
                    }
                    *totFiltrate = *totFiltrate + 1;
                } else {
                    prev = curr;
                    curr = (parolaAmmissibile *) curr->next;
                }
            }
        }
        MergeSort(filtrate); //ordina in ordine alfabetico le filtrate

    } else if(numCofronto > 1) { //siamo ai confronti successivi al primo allora avremo già che filtrate ha delle parole al suo interno
        curr = *filtrate;
        prev = NULL;
        while(curr != NULL && confrontaVincoli(curr->p,vectV,aux,dimParola,vectK) == 0) { //devi spostare la testa della lista, continua fino a che la testa della lista non è da spostare
            *filtrate = (parolaAmmissibile *) curr->next;
            k = hashFunction(curr->p[0]);
            curr->next = (struct parolaAmmissibile *) iniziali[k];
            iniziali[k] = curr;
            curr = (parolaAmmissibile *) *filtrate;
        }
        if(curr != NULL) {
            *totFiltrate = *totFiltrate + 1;
            prev = curr;
            curr = (parolaAmmissibile *) curr->next;
        }
        while(curr != NULL) {
            if(confrontaVincoli(curr->p,vectV,aux,dimParola,vectK) == 1) { // 1 significa che è ammissibile
                //aumento il contatore e lascio la parola dove è perchè è ancora compatibile con i vincoli e quindi deve rimanere in lista
                *totFiltrate = *totFiltrate + 1;
                prev = curr;
                curr = (parolaAmmissibile *) curr->next;
            } else { //non è compatibile, allora devo eliminare e deallocare
                prev->next = curr->next;
                k = hashFunction(curr->p[0]);
                curr->next = (struct parolaAmmissibile *) iniziali[k];
                iniziali[k] = curr;
                curr = (parolaAmmissibile *) prev->next;
            }
        }
    }
}

void inutile(int *nonUtile) {
    *nonUtile = 0;
}

void riportaFiltrateInAmmissibili(parolaAmmissibile **iniziali, parolaAmmissibile *filtrate) {
    parolaAmmissibile *tmp;
    int k;
    tmp = filtrate;
    //stampaLista(filtrate);
    while(tmp != NULL) {
        k = hashFunction(tmp->p[0]);
        //printf("RIPORTA: %s\n",tmp->p);
        filtrate = (parolaAmmissibile *) tmp->next;
        tmp->next = (struct parolaAmmissibile *) iniziali[k];
        iniziali[k] = tmp;
        tmp = (parolaAmmissibile *) filtrate;
    }
}

char *leggiComando(char *p) {
    char tmp;
    int i = 0;
    tmp = getchar_unlocked();
    if(tmp == EOF)
        return NULL;
    while(tmp != '\0' && tmp != '\n') {
        p[i] = tmp;
        tmp = getchar_unlocked();
        i++;
    }
    p[i] = '\0';
    return p;
}

int main() {
    parolaAmmissibile *filtrate,*supporto,*iniziali[N];
    carattere vectR[N];
    vincolo vectVincoli[N];
    char *p,*r,*res,*aux,confronto,*buffer,*temporaneo;
    int n,k,maxTry,i,trovata,totFiltrate,nonUtile,numConfronti,contaBuffer,cellaCurrentString,j,modificata,*vectK,deviUscire,hashTmp;
    nonUtile = 0;
    contaBuffer = 0;
    deviUscire = 0;
    filtrate = NULL;
    for(int l=0; l<N; l++) {
        iniziali[l] = NULL;
    }

    nonUtile = scanf("%d\n",&k);
    if(k > 16) { //questo controllo mi serve perchè nella stringa che uso per controllo nel while devo poter contenere anche +stampa_filtrate e +nuova_partita
        n = k;
    } else {
        n = 20;
    }
    aux = (char*)calloc(1,sizeof(char)*(k+1)); //stringa ausiliaria
    res = (char*)calloc(1,sizeof(char)*(k+1)); //striga risultato
    p = (char*)calloc(1,sizeof(char)*(n+1));  //stringa temporanea
    r = (char*)calloc(1,sizeof(char)*(k+1));  //stringa di riferimento
    buffer = (char*)calloc(dimBuffer,sizeof(char)*(k+1));
    vectK = (int*)malloc(sizeof(int)*k);
    p = leggiComando(p);
    while(deviUscire == 0) {    //questi inserimenti sono fuori dalla partita non devo confrontarli con i vincoli
        //ammissibili = inserisciInListaPrimo(p,ammissibili,k);
        if(contaBuffer < dimBuffer) {
            // contaBuffer * (k+1) è l'indice della i-esima parola nel buffer

            cellaCurrentString = contaBuffer * (k+1);
            strcpy(&buffer[cellaCurrentString],p);



            contaBuffer++;
            buffer[cellaCurrentString -1] = '\n';

            p = leggiComando(p);
            if(p[0] == '+') {
                if(p[1] == 'i') {
                    p = leggiComando(p);
                }
                if(p[1] == 'n') {
                    deviUscire = 1;
                }
            }
            if(feof(stdin)) {
                return 0;
            }

        } else {

            supporto = (parolaAmmissibile*)calloc(contaBuffer,sizeof(parolaAmmissibile));
            temporaneo = (char*)calloc(contaBuffer,sizeof(char)*(k+1));

            strcpy(temporaneo,buffer);

            cellaCurrentString = 0;
            i = 0;
            while(i < contaBuffer - 1) {

                supporto[i].p = &temporaneo[cellaCurrentString];
                supporto[i].hash = creaHash(supporto[i].p,k);

                hashTmp = hashFunction(supporto[i].p[0]);
                supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                iniziali[hashTmp] = &supporto[i];
                i++;
                cellaCurrentString = i * (k+1);
                temporaneo[cellaCurrentString - 1] = '\0';

            }
            supporto[i].p = &temporaneo[cellaCurrentString];
            supporto[i].hash = creaHash(supporto[i].p,k);
            hashTmp = hashFunction(supporto[i].p[0]);
            supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
            iniziali[hashTmp] = &supporto[i];
            i++;
            cellaCurrentString = i * (k+1);
            temporaneo[cellaCurrentString - 1] = '\0';
            contaBuffer = 0;
        }
    }

    if(contaBuffer > 0) {
        supporto = (parolaAmmissibile*)calloc(contaBuffer,sizeof(parolaAmmissibile));
        temporaneo = (char*)calloc(contaBuffer,sizeof(char)*(k+1));
        strcpy(temporaneo,buffer);
        cellaCurrentString = 0;
        i = 0;
        while(i < contaBuffer - 1) {
            temporaneo[cellaCurrentString + k] = '\0';
            supporto[i].p = &temporaneo[cellaCurrentString];
            supporto[i].hash = creaHash(supporto[i].p,k);
            hashTmp = hashFunction(supporto[i].p[0]);
            supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
            iniziali[hashTmp] = &supporto[i];
            i++;
            cellaCurrentString = i * (k+1);

        }

        supporto[i].p = &temporaneo[cellaCurrentString];
        supporto[i].hash = creaHash(supporto[i].p,k);
        hashTmp = hashFunction(supporto[i].p[0]);
        supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
        iniziali[hashTmp] = &supporto[i];
        i++;
        cellaCurrentString = i * (k+1);
        temporaneo[cellaCurrentString - 1] = '\0';
    }
    do {
        setAuxDefault(aux,k);   //impostiamo la stringa ausiliaria piena di & che servono per i confronti
        setVettoriDefault(vectR, vectVincoli);
        filtrate = NULL;
        r = leggiComando(r);
        vectK = gestisciParolaRiferimento(r,vectR,vectK); //riempi il vettore R con i dati della parola di riferimento

        nonUtile = scanf("%d\n",&maxTry);

        j = 0;
        numConfronti = 0;
        trovata = 0;


        while(j < maxTry && trovata == 0) {


            p = leggiComando(p);
            if(p[0] == '+') {
                if(p[11] == 'i') {


                    p = leggiComando(p);
                    modificata = 0;
                    contaBuffer = 0;
                    deviUscire = 0;
                    while(deviUscire == 0) {

                        if(contaBuffer < dimBuffer) {


                            cellaCurrentString = contaBuffer * (k+1);
                            strcpy(&buffer[cellaCurrentString],p);



                            contaBuffer++;
                            buffer[cellaCurrentString -1] = '\n';

                            p = leggiComando(p);
                            if(feof(stdin)) {
                                return 0;
                            }
                            if(p[0] == '+') {
                                if(p[11] == 'f')
                                    deviUscire = 1;
                            }

                        } else {

                            supporto = (parolaAmmissibile*)calloc(contaBuffer,sizeof(parolaAmmissibile));
                            temporaneo = (char*)calloc(contaBuffer,sizeof(char)*(k+1));

                            strcpy(temporaneo,buffer);

                            cellaCurrentString = 0;
                            i = 0;
                            while(i < contaBuffer) {

                                supporto[i].p = &temporaneo[cellaCurrentString];
                                supporto[i].hash = creaHash(supporto[i].p,k);

                                if(numConfronti != 0) {
                                    if(confrontaVincoli(supporto[i].p,vectVincoli,aux,k,vectK) == 1) {  // da inserire nelle filtrate
                                        supporto[i].next = (struct parolaAmmissibile *) filtrate;
                                        filtrate = &supporto[i];
                                        modificata = 1;
                                    } else {
                                        hashTmp = hashFunction(supporto[i].p[0]);
                                        supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                                        iniziali[hashTmp] = &supporto[i];
                                    }
                                } else {
                                    hashTmp = hashFunction(supporto[i].p[0]);
                                    supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                                    iniziali[hashTmp] = &supporto[i];
                                }
                                i++;
                                cellaCurrentString = i * (k+1);
                                temporaneo[cellaCurrentString - 1] = '\0';
                            }
                            contaBuffer = 0;
                        }
                    }
                    if(contaBuffer > 0) {
                        supporto = (parolaAmmissibile*)calloc(contaBuffer,sizeof(parolaAmmissibile));
                        temporaneo = (char*)calloc(contaBuffer,sizeof(char)*(k+1));
                        strcpy(temporaneo,buffer);
                        cellaCurrentString = 0;
                        i = 0;
                        while(i < contaBuffer) {
                            supporto[i].p = &temporaneo[cellaCurrentString];
                            supporto[i].hash = creaHash(supporto[i].p,k);
                            if(numConfronti != 0) {
                                if(confrontaVincoli(supporto[i].p,vectVincoli,aux,k,vectK) == 1) {  // da inserire nelle filtrate
                                    supporto[i].next = (struct parolaAmmissibile *) filtrate;
                                    filtrate = &supporto[i];
                                    modificata = 1;
                                } else {
                                    hashTmp = hashFunction(supporto[i].p[0]);
                                    supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                                    iniziali[hashTmp] = &supporto[i];
                                }
                            } else {
                                hashTmp = hashFunction(supporto[i].p[0]);
                                supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                                iniziali[hashTmp] = &supporto[i];
                            }
                            i++;
                            cellaCurrentString = i * (k+1);
                            temporaneo[cellaCurrentString - 1] = '\0';

                        }

                    }
                    if(modificata == 1)
                        MergeSort(&filtrate);
                } else if(p[11] == 't') {
                    stampaFiltrate(iniziali,filtrate,numConfronti);
                }
            } else {
                confronto = eseguiConfronto(iniziali,filtrate, res, r, p, k, vectR, vectVincoli, aux);
                //il filtro delle parole secondo i vincoli appena appresi lo si fa solamente se la parola appena inserita non è in lista oppure se è diversa da quella di riferimento
                if(confronto == 0) {
                    printf("not_exists\n");
                } else if(confronto == 1) {
                    printf("%s\n",res);    //ricorda che devi stampare totFiltrate dopo la ,
                    //adesso devo filtrare le parole secondo il nuovo confronto appena eseguito
                    //se sono al primo confronto devo passare alla funzione la lista per intero, altrimenti gli passo la lista delle parole filtrate
                    j++;
                    numConfronti++;
                    filtraLista(iniziali,&filtrate,vectVincoli,aux,k,numConfronti,&totFiltrate,vectK);
                    printf("%d\n",totFiltrate);
                } else if(confronto == 2) {
                    printf("ok\n");
                    trovata = 1;
                }
            }
            if(j == maxTry && trovata == 0)
                printf("ko\n");
        }
        p = leggiComando(p);
        if(p == NULL)
            return 0;
        if(numConfronti != 0) {
            riportaFiltrateInAmmissibili(iniziali, filtrate);
            deallocaListeIndexNot(vectVincoli);
        }
        if(p[0] == '+') {
            if (p[1] == 'i') {
                p = leggiComando(p);
                contaBuffer = 0;
                deviUscire = 0;
                while (deviUscire == 0) {
                    //proviamo ad usare il buffer anche qui per vedere se la situazione migliora, anche se lo farà sicuramente dato che limitiamo il numero di malloc che facciamo
                    if (contaBuffer < dimBuffer) {
                        // contaBuffer * (k+1) è l'indice della i-esima parola nel buffer

                        cellaCurrentString = contaBuffer * (k + 1);
                        strcpy(&buffer[cellaCurrentString], p);
                        contaBuffer++;
                        buffer[cellaCurrentString - 1] = '\n';
                        p = leggiComando(p);
                        if (feof(stdin)) {
                            return 0;
                        }
                        if (p[0] == '+') {
                            if (p[11] == 'f')
                                deviUscire = 1;
                        }
                    } else {
                        supporto = (parolaAmmissibile *) calloc(contaBuffer, sizeof(parolaAmmissibile));
                        temporaneo = (char *) calloc(contaBuffer, sizeof(char) * (k + 1));
                        strcpy(temporaneo, buffer);
                        cellaCurrentString = 0;
                        i = 0;
                        while (i < contaBuffer) {
                            supporto[i].p = &temporaneo[cellaCurrentString];
                            supporto[i].hash = creaHash(supporto[i].p, k);
                            hashTmp = hashFunction(supporto[i].p[0]);
                            supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                            iniziali[hashTmp] = &supporto[i];
                            i++;
                            cellaCurrentString = i * (k + 1);
                            temporaneo[cellaCurrentString - 1] = '\0';
                        }
                        contaBuffer = 0;
                    }
                }
                if (contaBuffer > 0) {
                    supporto = (parolaAmmissibile *) calloc(contaBuffer, sizeof(parolaAmmissibile));
                    temporaneo = (char *) calloc(contaBuffer, sizeof(char) * (k + 1));
                    strcpy(temporaneo, buffer);
                    cellaCurrentString = 0;
                    i = 0;
                    while (i < contaBuffer) {
                        supporto[i].p = &temporaneo[cellaCurrentString];
                        supporto[i].hash = creaHash(supporto[i].p, k);
                        hashTmp = hashFunction(supporto[i].p[0]);
                        supporto[i].next = (struct parolaAmmissibile *) iniziali[hashTmp];
                        iniziali[hashTmp] = &supporto[i];
                        i++;
                        cellaCurrentString = i * (k + 1);
                        temporaneo[cellaCurrentString - 1] = '\0';

                    }
                }
                p = leggiComando(p);
            }
        }
        filtrate = NULL;
        inutile(&nonUtile);
    } while(strcmp(p,"+nuova_partita") == 0);
    return 0;
}