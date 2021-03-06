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

int global_turn;
pthread_mutex_t global_turn_mutex;

float elapsed_time; // in centiseconds

int final_run;
pthread_mutex_t final_run_mutex;
/* 0: nobody was chosen to run at 90km/h
 * 1: one runner was chosen to run at 90km/h, but he is not running at
 *    that speed yet
 * 2: there is one runner running at 90km/h
 */

void * thread(void * arg) {
    struct corredor *me = (struct corredor*)arg;
    int oldx, newx, round = 0, turn = -1, vel_const = 2;
    float r;

    while(1) {
        oldx = me->x/vel_const;
        newx = (me->x + me->vel)/vel_const % d;
        // Fact: newx = oldx + 1 or newx = oldx

        if (oldx != newx) { /* trying to walk */
            pthread_mutex_lock(&pista_mutex[newx][me->y]);

            if (pista[newx][me->y] == NULL) {
                pista[newx][me->y] = me;
                pthread_mutex_unlock(&pista_mutex[newx][me->y]);

                pthread_mutex_lock(&pista_mutex[oldx][me->y]);
                pista[oldx][me->y] = NULL;
                pthread_mutex_unlock(&pista_mutex[oldx][me->y]);

                me->x = (me->x + me->vel) % (vel_const*d);
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

                            me->x = (me->x + me->vel) % (vel_const*d);
                            me->y = i;
                            break;
                        }
                        pthread_mutex_unlock(&pista_mutex[newx][i]);
                    } else
                        pthread_mutex_unlock(&pista_mutex[oldx][i]);
                }
            }
        } else
            me->x = (me->x + me->vel) % (vel_const*d);


        pthread_barrier_wait(&barrier[round]);
        /* try to go to the left */
        for (int i = me->y - 1; i >= 0; i--) {
            pthread_mutex_lock(&pista_mutex[me->x/vel_const][i]);
            if (pista[me->x/vel_const][i] == NULL) {
                pista[me->x/vel_const][i] = me;

                pthread_mutex_lock(&pista_mutex[me->x/vel_const][me->y]);
                pista[me->x/vel_const][me->y] = NULL;
                pthread_mutex_unlock(&pista_mutex[me->x/vel_const][me->y]);

                me->y = i;
            }
            pthread_mutex_unlock(&pista_mutex[me->x/vel_const][i]);
        }

        pthread_mutex_lock(&corredor_count_mutex);
        if (corredor_count == 1) {
            corredor_count--;
            pthread_mutex_unlock(&corredor_count_mutex);
            me->state &= ~RUNNING;
            me->final_turn = turn;
            pthread_barrier_wait(&barrier[round]);
            pthread_barrier_wait(&barrier[round]);
            return(0);
        }
        else
            pthread_mutex_unlock(&corredor_count_mutex);

        if (me->x/vel_const < oldx) {
            me->final_time = elapsed_time;

            if (turn >= 0) {
                pthread_mutex_lock(&placing_count_mutex[turn]);
                me->placing[turn] = ++placing_count[turn];

                if (me->placing[turn] >= placing_count_max[turn]) {
                    pthread_mutex_unlock(&placing_count_mutex[turn]);
                    pthread_mutex_lock(&global_turn_mutex);
                    if (turn == global_turn)
                        global_turn++;
                    pthread_mutex_unlock(&global_turn_mutex);

                    if (turn % 2) {
                        me->state &= ~RUNNING;
                        me->final_turn = turn;

                        pthread_mutex_lock(&corredor_count_mutex);
                        corredor_count--;
                        pthread_mutex_unlock(&corredor_count_mutex);

                        pthread_mutex_lock(&pista_mutex[me->x/vel_const][me->y]);
                        pista[me->x/vel_const][me->y] = NULL;
                        pthread_mutex_unlock(&pista_mutex[me->x/vel_const][me->y]);

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
                    me->final_turn = -(turn + 1);
                    pthread_mutex_lock(&corredor_count_mutex);
                    corredor_count--;
                    pthread_mutex_unlock(&corredor_count_mutex);

                    pthread_mutex_lock(&pista_mutex[me->x/vel_const][me->y]);
                    pista[me->x/vel_const][me->y] = NULL;
                    pthread_mutex_unlock(&pista_mutex[me->x/vel_const][me->y]);

                    for (int i = turn + 1; i < 2*n; i++) {
                        pthread_mutex_lock(&placing_count_mutex[i]);
                        placing_count_max[i]--;
                        pthread_mutex_unlock(&placing_count_mutex[i]);
                    }

                    pthread_barrier_wait(&barrier[round]);
                    pthread_barrier_wait(&barrier[round]);

                    return(0);
                }

                r = (float)rand()/(float)RAND_MAX;

                pthread_mutex_lock(&placing_count_mutex[turn]);
                if (placing_count_max[turn] == 2) {
                    pthread_mutex_unlock(&placing_count_mutex[turn]);
                    pthread_mutex_lock(&final_run_mutex);
                    if (final_run == 0) {
                        if (r < 0.05) {
                            final_run = 2;
                            me->vel = VEL90;
                        }
                        else if (r < 0.1)
                            final_run = 1;
                    }
                    else if (final_run == 1) {
                        final_run = 2;
                        me->vel = VEL90;
                    }
                    pthread_mutex_unlock(&final_run_mutex);
                }
                else
                    pthread_mutex_unlock(&placing_count_mutex[turn]);

                pthread_mutex_lock(&final_run_mutex);
                if (final_run == 2 && vel_const == 2) {
                    me->x *= 3;
                    vel_const = 6;
                }
                pthread_mutex_unlock(&final_run_mutex);

                /* Change velocity? */
                if (me->vel == VEL30 && r < 0.8)
                    me->vel = VEL60;
                else if (me->vel == VEL60 && r < 0.4)
                    me->vel = VEL30;
            }
            turn++;
        }

        pthread_barrier_wait(&barrier[round]);
        pthread_barrier_wait(&barrier[round]);

        round = !round;

    }
}

void print_track() {
    int c = (int) log10((double) n);
    for (int j = 0; j < 10; j++){
        for (int i = 0; i < d; i++) {
            if (pista[i][j])
                printf("%*d", c + 1, pista[i][j]->id);
            else {
                printf("_");
                for (int j = 0; j < c; j++)
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
        if (v[i].final_turn > 0)
            printf(" - Colocação %d: corredor %d, última volta "
                   "completada aos %.2f segundos\n",
                    i + 1, v[i].id, v[i].final_time/100);
        else
            printf(" - Corredor %d: quebrou na rodada %d\n",
                   v[i].id, -v[i].final_turn);
    printf("\n");
}

int main(int argc, char* argv[]) {
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
    placing_count_mutex = malloc(2*n * sizeof(pthread_mutex_t));
    int m = n;
    for (int i = 0; i < 2*n; i += 2) {
        placing_count_max[i] = placing_count_max[i + 1] = m;
        placing_count[i] = placing_count[i + 1] = 0;
        pthread_mutex_init(&placing_count_mutex[i], NULL);
        pthread_mutex_init(&placing_count_mutex[i + 1], NULL);
        m--;
    }

    global_turn = 0;
    pthread_mutex_init(&global_turn_mutex, NULL);

    elapsed_time = 6;

    final_run = 0;
    pthread_mutex_init(&final_run_mutex, NULL);

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
        corredor_v[i].placing = malloc(2*n * sizeof(int));
        for (int j = 0; j < 2*n; j++)
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

        if (final_run == 2)
            elapsed_time += 2;
        else
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

    for (int i = 0; i < d || i < 2*n; i++) {
        if (i < d) {
            for (int j = 0; j < 10; j++)
                pthread_mutex_destroy(&pista_mutex[i][j]);
            free(pista[i]);
            free(pista_mutex[i]);
        }
        if (i < n)
            free(corredor_v[i].placing);
        if (i < 2*n)
            pthread_mutex_destroy(&placing_count_mutex[i]);
    }

    free(pista_mutex);
    free(pista);
    free(placing_count);
    free(placing_count_max);
    free(placing_count_mutex);
    free(corredor_v);
}
