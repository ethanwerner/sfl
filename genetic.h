// genetic.h - Genetic Algorithm

#ifndef GENETIC_H
#define GENETIC_H

#include <stdint.h>
#include <assert.h>


void genetic_generation(
    void *,
    void *,
    uint32_t,
    double,
    double,
    double,
    double ( * )( void *, void const * ),
    void ( * )( void *, void *, void *, double ),
    void ( * )( void *, double )
);

#endif // GENETIC_H


#ifdef GENETIC_IMPLEMENTATION // IMPLEMENTATION


#include <stdlib.h>
#include <stdio.h>


static void genetic_fitness_sort( uintptr_t *, void *, uint32_t, double ( * )( void *, void const * ) );


void genetic_generation(
    void *population,
    void *argument_fitness,
    uint32_t population_n,
    double rate_selection,
    double rate_crossover,
    double rate_mutation,
    double ( *genetic_fitness )( void *, void const * ),
    void ( *genetic_crossover )( void *, void *, void *, double ),
    void ( *genetic_mutation )( void *, double )
)
{
    uintptr_t *p = population;
    
    assert( 0 < rate_selection && rate_selection <= 1.0);
    assert( 0 < rate_crossover && rate_crossover <= 1.0);
    assert( 0 < rate_mutation && rate_mutation <= 1.0);
    
    // Fitness
    genetic_fitness_sort( population, argument_fitness, population_n, genetic_fitness );
    // Crossover
    uint32_t selection_i = rate_selection * population_n;
    uint32_t x0, x1;
    for( uint32_t i = selection_i; i < population_n; i++ )
    {
        x0 = rand() % selection_i;
        x1 = rand() % selection_i;
        
        genetic_crossover( ( void * ) p[i], ( void * ) p[x0], ( void * ) p[x1], rate_crossover );
        genetic_mutation( ( void * ) p[i], rate_mutation );
    }
    
    //genetic_fitness_sort( population, argument_fitness, population_n, genetic_fitness );
}


static void genetic_fitness_sort( 
    uintptr_t *p, 
    void *argument_fitness,
    uint32_t population_n,
    double ( *genetic_fitness )( void *, void const * ) 
)
{
    double *fitness = calloc( population_n, sizeof( double ) );
    double f = 0;
    uintptr_t current;
    
    uint32_t j;
    uint32_t i;
    for( i = 0; i < population_n; i++ )
    {
        f = genetic_fitness( ( void * ) p[i], argument_fitness );
        current = p[i];

        j = i;
        for( ; j > 0 && f > fitness[j - 1]; j-- );
        
        // Shift array back by one for sorting
        for( uint32_t k = i; k > j; k-- )
        {
            p[k] = p[k - 1];
            fitness[k] = fitness[k - 1];
        }
        
        p[j] = current;
        fitness[j] = f;
    }
    
    free( fitness );
}


#endif
