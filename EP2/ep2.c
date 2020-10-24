#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// TODO: Aumentar paralelização dando unlock nos mutex antes de acabar
// as condicionais
// lock(a)
// if (a)
//     unlock(a)
//     ...
// else
//     unlock(a)
//     ...

enum velocity {VEL30 = 1, VEL60 = 2, VEL90 = 3};

struct corredor {
    int id;
    enum velocity vel;
    int x;
    int y;
    pthread_t thread;
    int dead;
};
// A posição x é guardada como o dobro do que de fato é
// Para ler/escrever, dividimos inteiramente por 2
// quando tiver alguém correndo a 90, essa constante passa a ser 6
// para isso vamos multiplcar todos os x por 3.

struct corredor *** pista; // matrix of pointers to struct corredor
pthread_mutex_t **pista_mutex;

int corredor_count; // number of remaining competitors
pthread_mutex_t corredor_count_mutex; //

int d; // length of the track

pthread_barrier_t *barrier; // vector of two barriers, one for each round



void * thread(void * arg) {
    struct corredor *me = (struct corredor*)arg;
    int oldx, newx, round = 0, turn = 0;
    float r;

    while(1) {
        oldx = me->x/2;
        newx = (me->x + me->vel)/2 % d;
        // Fact: newx = oldx + 1 or newx = oldx
        // (to enforce this, the VEL90 = 3 case must be treated specially)

        if (oldx != newx) { /* Vou tentar andar */
            pthread_mutex_lock(&pista_mutex[newx][me->y]);

            if (pista[newx][me->y] == NULL) {
                pista[newx][me->y] = me;
                pthread_mutex_unlock(&pista_mutex[newx][me->y]);

                pthread_mutex_lock(&pista_mutex[oldx][me->y]);
                pista[oldx][me->y] = NULL;
                pthread_mutex_unlock(&pista_mutex[oldx][me->y]);

                me->x = (me->x + me->vel) % (2*d);
            }

            else {
                pthread_mutex_unlock(&pista_mutex[newx][me->y]);
                for (int i = me->y; i < 10; i++) { // tenta ultrapassar

                    pthread_mutex_lock(&pista_mutex[oldx][i]);
                    if (pista[oldx][i] == NULL) {
                        pthread_mutex_unlock(&pista_mutex[oldx][i]);

                        pthread_mutex_lock(&pista_mutex[newx][i]);
                        if (pista[newx][i] == NULL) {
                            pista[newx][i] = me;
                            pthread_mutex_unlock(&pista_mutex[newx][i]);

                            pthread_mutex_lock(&pista_mutex[oldx][me->y]);
                            pista[oldx][me->y] = NULL;
                            pthread_mutex_unlock(&pista_mutex[oldx][me->y]);

                            me->x = (me->x + me->vel) % (2*d);
                            me->y = i;
                            break;
                        }
                        pthread_mutex_unlock(&pista_mutex[newx][i]);
                    } else
                        pthread_mutex_unlock(&pista_mutex[oldx][i]);
                }
            }
            pthread_mutex_unlock(&pista_mutex[newx][me->y]);
        } else
            me->x = (me->x + me->vel) % (2*d);


        pthread_barrier_wait(barrier + round);
        /* Tenta andar à esquerda */
        for (int i = me->y - 1; i >= 0; i--) {
            pthread_mutex_lock(&pista_mutex[me->x/2][i]);
            if (pista[me->x/2][i] == NULL) {
                pista[me->x/2][i] = me;

                pthread_mutex_lock(&pista_mutex[me->x/2][me->y]);
                pista[me->x/2][me->y] = NULL;
                pthread_mutex_unlock(&pista_mutex[me->x/2][me->y]);

                me->y = i;
            }
            pthread_mutex_unlock(&pista_mutex[me->x/2][i]);
        }

        r = (float)rand()/(float)RAND_MAX;
        if (me->x/2 < oldx) {
            printf("%d completou uma volta!\n", me->id);
            turn++;
            if (turn % 6 == 0 && r < 0.05) {
                fprintf(stderr, "Corredor %d morreu\n", me->id);
                me->dead = 1;
                pthread_mutex_lock(&corredor_count_mutex);
                corredor_count--;
                pthread_mutex_unlock(&corredor_count_mutex);

                pthread_mutex_lock(&pista_mutex[me->x/2][me->y]);
                if (pista[me->x][me->y] == me)
                    pista[me->x][me->y] = NULL;
                pthread_mutex_unlock(&pista_mutex[me->x/2][me->y]);

                pthread_barrier_wait(barrier + round);
                pthread_barrier_wait(barrier + round);

                return(0);
            }
        }

        pthread_barrier_wait(barrier + round);
        pthread_barrier_wait(barrier + round);
        round = !round;
    }
}

void print() {
    int cols = 80; // maybe get current terminal width here
    for (int j = 0; j < 10; j++){
        for (int i = 0; i < cols - 3 && i < d; i++) {
            if (pista[i][j])
                printf("%d", pista[i][j]->id);
            else
                printf("_");
        }
        printf("\n");
    }
    getchar();
}

int main(int argc, char* argv[])
{
    /******* Treat arguments ******************************************/
    if (argc < 2) {
        fprintf(stderr, "Usagem: ./ep2 d n\n");
        exit(EXIT_FAILURE);
    }

    d = atoi(argv[1]);
    int n = atoi(argv[2]);
    /******************************************************************/

    /******* Initialization *******************************************/

    time_t t;
    srand((unsigned) time(&t));

    corredor_count = n;
    pthread_mutex_init(&corredor_count_mutex, NULL);

    barrier = malloc(2 * sizeof(pthread_barrier_t));
    pthread_barrier_init(barrier, NULL, (unsigned) (n + 1));
    pthread_barrier_init(barrier + 1, NULL, (unsigned) (n + 1));

    pista = malloc(d * sizeof(struct corredor**));
    pista_mutex = malloc(d * sizeof(struct pthread_mutex_t *));
    for (int i = 0; i < d; i++) {
        pista[i] = malloc(10 * sizeof(struct corredor*));
        pista_mutex[i] = malloc(10 * sizeof(pthread_mutex_t));
        for (int j = 0; j < 10; j++) {
            pista[i][j] = NULL;
            pthread_mutex_init(&pista_mutex[i][j], NULL);
        }
    }

    struct corredor * corredor_v;
    corredor_v = malloc(n * sizeof(struct corredor));

    int pos = 0, imod5;
    for (int i = 0; i < n; i++) {
        imod5 = i % 5;
        if (imod5 == 0)
            pos++;
        pista[d - pos][imod5] = corredor_v + i;
        corredor_v[i].x = (d - pos)*2;
        corredor_v[i].y = imod5;
        corredor_v[i].vel = VEL30;
        corredor_v[i].id = i;
        corredor_v[i].dead = 0;
        pthread_create(&corredor_v[i].thread, NULL, thread, (void *)(corredor_v + i));
    }
    /******************************************************************/
    corredor_v[5].vel = VEL60;

    int round = 0, c;
    while(corredor_count > 1) {
        c = corredor_count;
        pthread_barrier_wait(barrier + round);
        pthread_barrier_wait(barrier + round);

        if (c != corredor_count) {
            pthread_barrier_destroy(barrier + !round);
            pthread_barrier_init(barrier + !round, NULL, corredor_count + 1);
        }

        print();
        pthread_barrier_wait(barrier + round);

        for (int i = 0; i < n; i++) {
            if (corredor_v[i].dead) {
                pthread_join(corredor_v[i].thread, NULL);
                corredor_v[i].dead = 0;
            }
        }
        round = !round;
    }

    /* destroi o vencedor */

    /****** Destruction ***********************************************/
    for (int i = 0; i < d; i++) {
        free(pista[i]);
        for (int j = 0; j < 10; j++)
            pthread_mutex_destroy(&pista_mutex[i][j]);
    }
    pthread_barrier_destroy(barrier);
    pthread_barrier_destroy(barrier + 1);
    free(pista);

}
