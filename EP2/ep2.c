#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum velocidade {VEL30, VEL60, VEL90};

struct corredor {
    int id;
    enum velocidade vel;
    int x;
    int y;
    pthread_t thread;
    int dead;
};

struct corredor *** pista; // matrix of pointers to struct corredor
int corredor_count;
pthread_mutex_t corredor_count_mutex;
int d;

pthread_barrier_t *barrier;

pthread_mutex_t **pista_mutex;


void * thread(void * arg) {
    struct corredor *me = (struct corredor*)arg;
    int newx, round = 0;
    float r;

    while(1) {
        r = (float)rand()/(float)RAND_MAX;
        if (r < 0.05) {
            printf("Corredor %d morreu\n", me->id);
            me->dead = 1;
            pthread_mutex_lock(&corredor_count_mutex);
            corredor_count--;
            pthread_mutex_unlock(&corredor_count_mutex);


            pthread_mutex_lock(&pista_mutex[me->x][me->y]);
            if (pista[me->x][me->y] == me)
                pista[me->x][me->y] = NULL;
            pthread_mutex_unlock(&pista_mutex[me->x][me->y]);

            pthread_barrier_wait(barrier + round);
            /* printf("%d passou a primeira barreira\n", me->id); */
            pthread_barrier_wait(barrier + round);
            /* printf("%d passou a segunda barreira e vai retornar\n", me->id); */

            return(0);

        } else {

            newx = (me->x + 1) % d;
            pthread_mutex_lock(&pista_mutex[newx][me->y]);
            pista[newx][me->y] = me;
            pthread_mutex_unlock(&pista_mutex[newx][me->y]);

            pthread_mutex_lock(&pista_mutex[me->x][me->y]);
            if (pista[me->x][me->y] == me)
                pista[me->x][me->y] = NULL;
            pthread_mutex_unlock(&pista_mutex[me->x][me->y]);

            me->x = newx;
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
            pthread_mutex_init(pista_mutex[i] + j, NULL);
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
        corredor_v[i].x = d - pos;
        corredor_v[i].y = imod5;
        corredor_v[i].vel = VEL30;
        corredor_v[i].id = i;
        corredor_v[i].dead = 0;
        pthread_create(&corredor_v[i].thread, NULL, thread, (void *)(corredor_v + i));
    }
    /******************************************************************/

    int round = 0;
    while(corredor_count > 1) {
        pthread_barrier_wait(barrier + round);
        pthread_barrier_destroy(barrier + !round);
        pthread_barrier_init(barrier + !round, NULL, corredor_count + 1);
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
            pthread_mutex_destroy(pista_mutex[i] + j);
    }
    pthread_barrier_destroy(barrier);
    pthread_barrier_destroy(barrier + 1);
    free(pista);

}
