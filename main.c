#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "gol.h"
#include "gol.c"
#define _XOPEN_SOURCE 600

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_step = {0, 0, 0, 0};
    stats_t stats_total = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro>!\n\n", argv[0]);
        return 0;
    }

    //Quantas threads?
    int n_threads = atoi(argv[2]);
    if (!n_threads) {
        printf("Número de threads deve ser > 0\n");
        return 1;
    }

    //Inicializa a barrier para n_threads
    pthread_barrier_init(&barrier, NULL, n_threads);

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    fscanf(f, "%d %d", &size, &steps);
    int n_celulas = size*size;

    if (n_threads > n_celulas)
    {
        n_threads = n_celulas;
    }

    //Instancia array de threads
    pthread_t threads[n_threads];

    //Informações necessárias para realizar os cálculos
    aux info[n_threads];
    int begin = 0;
    int intervalo = n_celulas / n_threads;
    int resto = n_celulas % n_threads;

    for (int i = 0; i < n_threads; i++)
    {
        int end = begin + intervalo;
        if (resto)
        {
            --resto;
            ++end;
        }
        info[i].n_threads = n_threads;
        info[i].size = size;
        info[i].begin = begin;
        info[i].end = end;
        end = begin;
    }
    

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_step);
#endif

    for (int i = 0; i < steps; i++)
    {
        stats_step.borns = 0;
        stats_step.loneliness = 0;
        stats_step.overcrowding = 0;
        stats_step.survivals = 0;

        //TODO: iniciar threads que executam "play"
        for (int j = 0; j < n_threads; j++)
        {
            info[j].board = prev;
            info[j].newboard = next;
            info[j].stats = stats_step;
            pthread_create(&threads[j], NULL, play_parallel, (void *) &info[j]);
        }
        
        // stats_step = play(prev, next, size);
        for (int j = 0; j < n_threads; j++)
        {
            pthread_join(threads[j], NULL);
            stats_total.borns += info[j].stats.borns;
            stats_total.survivals += info[j].stats.survivals;
            stats_total.loneliness += info[j].stats.loneliness;
            stats_total.overcrowding += info[j].stats.overcrowding;
        }

#ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(next, size);
        print_stats(stats_step);
#endif
        tmp = next;
        next = prev;
        prev = tmp;
    }

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}