#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define RUNNING 1
#define DESTROYED 2

enum velocity {VEL30 = 1, VEL60 = 2, VEL90 = 3};

struct corredor {
    int id;
    enum velocity vel;
    int x;
    int y;
    pthread_t thread;
    int state; /* mask. See #defines above */
    int * placing; /* placing in each turn */
    int final_turn; /* negative values for who breaks */
    float final_time;
};

int d; // length of the track
int n; // initial number of competitors

struct corredor *** pista; // matrix of pointers to struct corredor
pthread_mutex_t **pista_mutex;

int corredor_count; // number of remaining competitors
pthread_mutex_t corredor_count_mutex;//

pthread_barrier_t *barrier; // vector of two barriers, one for each round

int * placing_count;
int * placing_count_max;
pthread_mutex_t * placing_count_mutex;

int global_turn; // No need for mutex since only one is last each turn

float elapsed_time; // in centiseconds
/* No need for mutex since:
   - it is read only for all thread which are not main
   - barriers guarantee main only updates this when no thread is trying
     to read it
*/

void * thread(void * arg) {
    struct corredor *me = (struct corredor*)arg;
    int oldx, newx, round = 0, turn = -1;
    float r;

    while(1) {
        oldx = me->x/2;
        newx = (me->x + me->vel)/2 % d;
        // Fact: newx = oldx + 1 or newx = oldx
        // (to enforce this, the vel90 = 3 case must be treated specially)

        if (oldx != newx) { /* trying to walk */
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
                for (int i = me->y; i < 10; i++) { // attempt to overtake

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
        } else
            me->x = (me->x + me->vel) % (2*d);


        pthread_barrier_wait(&barrier[round]);
        /* try to go to the left */
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

        if (me->x/2 < oldx) {
            if (turn >= 0) {
                pthread_mutex_lock(&placing_count_mutex[turn]);
                me->placing[turn] = ++placing_count[turn];

                if (me->placing[turn] == placing_count_max[turn]) {
                    pthread_mutex_unlock(&placing_count_mutex[turn]);

                    if (turn == global_turn)
                        global_turn++;

                    if (turn % 2) {
                        me->state &= ~RUNNING;
                        me->final_time = elapsed_time;
                        me->final_turn = turn;

                        pthread_mutex_lock(&corredor_count_mutex);
                        corredor_count--;
                        pthread_mutex_unlock(&corredor_count_mutex);

                        pthread_mutex_lock(&pista_mutex[me->x/2][me->y]);
                        pista[me->x/2][me->y] = NULL;
                        pthread_mutex_unlock(&pista_mutex[me->x/2][me->y]);

                        pthread_barrier_wait(&barrier[round]);
                        pthread_barrier_wait(&barrier[round]);

                        return(0);
                    }
                }
                else
                    pthread_mutex_unlock(&placing_count_mutex[turn]);

                if (turn > 0 && turn % 6 == 0 &&
                        (float)rand()/(float)RAND_MAX < 0.05) {
                    printf("Corredor %d quebrou\n", me->id);

                    me->state &= ~RUNNING;
                    me->final_turn = -turn;
                    pthread_mutex_lock(&corredor_count_mutex);
                    corredor_count--;
                    pthread_mutex_unlock(&corredor_count_mutex);

                    pthread_mutex_lock(&pista_mutex[me->x/2][me->y]);
                    pista[me->x/2][me->y] = NULL;
                    pthread_mutex_unlock(&pista_mutex[me->x/2][me->y]);

                    for (int i = turn + 1; i < 2 * n; i++) {
                        pthread_mutex_lock(&placing_count_mutex[i]);
                        placing_count_max[i]--;
                        pthread_mutex_unlock(&placing_count_mutex[i]);
                    }

                    pthread_barrier_wait(&barrier[round]);
                    pthread_barrier_wait(&barrier[round]);

                    return(0);
                }
            }

            /* Change velocity? */
            r = (float)rand()/(float)RAND_MAX;
            if (me->vel == VEL30 && r < 0.8)
                me->vel = VEL60;
            else if (me->vel == VEL60 && r < 0.4)
                me->vel = VEL30;

            turn++;

        }

        pthread_barrier_wait(&barrier[round]);
        pthread_barrier_wait(&barrier[round]);
        round = !round;

        if (corredor_count == 0)
            return(0);
    }
}

void print_track() {
    int c = (int) log10((double) n);
    for (int j = 0; j < 10; j++){
        for (int i = 0; i < d; i++) {
            if (pista[i][j])
                printf("%*d", c, pista[i][j]->id);
            else {
                printf("_");
                for (int j = 0; j < c - 1; j++)
                    printf(" ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_positions(struct corredor * v, int turn) {
    printf("Colocações na volta %d:\n", turn);
    for (int i = 0; i < n; i++)
        if (v[i].placing[turn])
            printf(" - Corredor %d: colocação %d\n",
                   v[i].id, v[i].placing[turn]);
    printf("\n");
}

int compar(const void* x, const void* y) {
    struct corredor *a = (struct corredor *)x;
    struct corredor *b = (struct corredor *)y;
    if (a->final_turn < b->final_turn)
        return 1;
    if (a->final_turn > b->final_turn)
        return -1;
    return 0;
}

void print_final(struct corredor * v) {
    printf("Colocações finais:\n");
    qsort(v, (size_t) n, sizeof(struct corredor), compar);

    for (int i = 0; i < n; i++)
        if (v[i].final_time > 0)
            printf(" - Colocação %d: corredor %d, última volta "
                   "completada aos %.2f segundos\n",
                    i + 1, v[i].id, v[i].final_time/100);
        else
            printf(" - Corredor %d: quebrou na rodada %d\n",
                   v[i].id, -v[i].final_turn);
    printf("\n");
}

int main(int argc, char* argv[])
{
    /******* Treat arguments ******************************************/
    if (argc < 2) {
        fprintf(stderr, "Usagem: ./ep2 d n\n");
        exit(EXIT_FAILURE);
    }

    d = atoi(argv[1]);
    n = atoi(argv[2]);

    int debug = 0;
    if (argc > 3 && !strcmp("debug", argv[3]))
        debug = 1;
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

    placing_count = malloc(2*n * sizeof(int));
    placing_count_max = malloc(2*n * sizeof(int));
    placing_count_mutex = malloc(2 * n * sizeof(pthread_mutex_t));
    int m = n;
    for (int i = 0; i < 2*n - 1; i += 2) {
        placing_count_max[i] = placing_count_max[i + 1] = m;
        placing_count[i] = placing_count[i + 1] = 0;
        pthread_mutex_init(&placing_count_mutex[i], NULL);
        pthread_mutex_init(&placing_count_mutex[i + 1], NULL);
        m--;
    }

    global_turn = 0;

    elapsed_time = 0.06;

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
        corredor_v[i].state = RUNNING;
        corredor_v[i].placing = malloc(2 * n * sizeof(int));
        for (int j = 0; j < 2 * n; j++)
            corredor_v[i].placing[j] = 0;
        pthread_create(&corredor_v[i].thread, NULL, thread, (void *)(corredor_v + i));
    }
    /******************************************************************/

    int round = 0, c, change_barrier = 0, turn = global_turn;
    while(corredor_count > 0) {
        c = corredor_count;
        pthread_barrier_wait(&barrier[round]);
        pthread_barrier_wait(&barrier[round]);

        if (c != corredor_count || change_barrier) {
            /* Only reinitialize barrier if the number of competitors
             * has changed */
            pthread_barrier_destroy(&barrier[!round]);
            pthread_barrier_init(&barrier[!round], NULL, corredor_count + 1);
            if (c != corredor_count)
                change_barrier = 1;
            else
                change_barrier = 0;
        }

        if (turn != global_turn) {
            print_positions(corredor_v, turn);
            turn++;
        }

        if (debug)
            print_track();

        elapsed_time += 6;

        pthread_barrier_wait(&barrier[round]);

        for (int i = 0; i < n; i++) {
            if ((corredor_v[i].state & (DESTROYED | RUNNING)) == 0) {
                pthread_join(corredor_v[i].thread, NULL);
                corredor_v[i].state |= DESTROYED;
            }
        }
        round = !round;
    }

    print_final(corredor_v);

    /****** Destruction ***********************************************/

    pthread_mutex_destroy(&corredor_count_mutex);
    pthread_barrier_destroy(barrier);
    pthread_barrier_destroy(barrier + 1);
    free(barrier);

    for (int i = 0; i < d; i++) {
        for (int j = 0; j < 10; j++)
            pthread_mutex_destroy(&pista_mutex[i][j]);
        free(pista[i]);
        free(pista_mutex[i]);
    }
    free(pista_mutex);
    free(pista);

    for (int i = 0; i < n; i++) {
        pthread_mutex_destroy(&placing_count_mutex[2*i]);
        pthread_mutex_destroy(&placing_count_mutex[2*i + 1]);
        free(corredor_v[i].placing);
    }
    free(placing_count);
    free(placing_count_max);
    free(placing_count_mutex);
    free(corredor_v);
}
