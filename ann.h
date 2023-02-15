/*
MIT License

Copyright (c) 2023 Ethan Werner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


// ann.h - Artificial Neural Nework


#ifndef ANN_H
#define ANN_H


#include <stdint.h>


typedef double fp_t;
typedef uint32_t uint_t;
typedef int32_t int_t;


typedef enum 
{
    IDENTITY,
    BINARY,
    SIGMOID,
    RELU,
    ELU,
    LRELU,
    TANH,
} ann_activation_t;


typedef struct
{
	// The full size of the allocated structure
	uint_t n;

	// The number of layers in the neural network
	uint_t layer_n;

	// The neuron count ( hidden )
	uint_t neuron_n;

	// The total number of weights and biases
	uint_t weight_n;

	// The number of neurons in each layer
	uint_t *layer_neuron_n;

	// The pointer to the neurons
    fp_t *neuron;

	// The weights and biases for each neuron
	//   - w_lji denotes the connection between the jth neuron in layer l, and
	//     the ith connection in the previous layer
	//   - b_lj denotes the bias for the jth neuron in layer l
	// { w_000, w_001, w_002, .... , b_00, ...., w_ijk, b_ij }
	fp_t *weight;

	// The delta between between the actual and the cost function
	fp_t *delta;

	// The activation function used in the hidden layer neurons
	fp_t ( *activation_hidden ) ( fp_t );

	// The partial derivative of the activation function used in the hidden
	// layer neurons
	fp_t ( *activation_hidden_partial ) ( fp_t );

	// The activation function used in the ouput neurons
	fp_t ( *activation_output ) ( fp_t );

	// The partial derivative of the activation function used in the output
	// layer neurons
	fp_t ( *activation_output_partial ) ( fp_t );
} ann_t;


ann_t * ann_init( uint_t, uint_t * );
ann_t * ann_copy( ann_t const * );
void ann_free( ann_t * );
void ann_random( ann_t * );

void ann_propagation_forward( ann_t *, fp_t const * const, fp_t * );
void ann_propagation_backward( ann_t *, fp_t const *, fp_t *, fp_t const *, fp_t );
void ann_train_numeric( ann_t *, fp_t const *, fp_t const *, fp_t );

fp_t ann_error_total( fp_t const *, fp_t const *, uint_t );
void ann_set_activation( ann_t *, ann_activation_t, ann_activation_t );

void ann_print_weight( ann_t * );
void ann_print_neuron( ann_t *, fp_t const * const, fp_t const * const );


#endif // ANN_H


#ifdef ANN_IMPLEMENTATION


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <tgmath.h>


#define PRINT_PRECISION 10

#define SQUARE_ROOT_2         1.4142135623730950488016887242096
#define PI                    3.1415926535897932384626433832795
#define SQUARE_ROOT_PI        1.7724538509055160272981674833411  
#define SQUARE_ROOT_2_OVER_PI 0.7978845608028653558798921198687

#define ELU_ALPHA         0.2
#define LRELU_ALPHA       0.2


static fp_t ann_random_range( fp_t, fp_t );

static fp_t ann_error( fp_t, fp_t );
static fp_t ann_error_partial( fp_t, fp_t );

static fp_t ann_activation_identity( fp_t );
static fp_t ann_activation_identity_partial( fp_t );
static fp_t ann_activation_binary( fp_t );
static fp_t ann_activation_binary_partial( fp_t );
static fp_t ann_activation_sigmoid( fp_t );
static fp_t ann_activation_sigmoid_partial( fp_t );
static fp_t ann_activation_relu( fp_t );
static fp_t ann_activation_relu_partial( fp_t );
static fp_t ann_activation_elu( fp_t );
static fp_t ann_activation_elu_partial( fp_t );
static fp_t ann_activation_lrelu( fp_t );
static fp_t ann_activation_lrelu_partial( fp_t );
static fp_t ann_activation_tanh( fp_t );
static fp_t ann_activation_tanh_partial( fp_t );


static fp_t ( *ACTIVATION[][2] )( fp_t ) = {
    [IDENTITY] = { ann_activation_identity, ann_activation_identity_partial },
    [BINARY] = { ann_activation_binary, ann_activation_binary_partial },
    [SIGMOID] = { ann_activation_sigmoid, ann_activation_sigmoid_partial },
    [RELU] = { ann_activation_relu, ann_activation_relu_partial },
    [ELU] = { ann_activation_elu, ann_activation_elu_partial },
    [LRELU] = { ann_activation_lrelu, ann_activation_lrelu_partial },
    [TANH] = { ann_activation_tanh, ann_activation_tanh_partial },
};



ann_t * ann_init( uint_t layer_n, uint_t *layer_neuron_n )
{
	assert( layer_n >= 2 );

	uint_t neuron_n = 0;
	uint_t weight_n = 0;

	// Calculate the number of neurons and weights / biases
	uint_t l = 1;
	for( ; l < layer_n - 1; l++ )
	{
		neuron_n += layer_neuron_n[l];
		weight_n += layer_neuron_n[l] * (layer_neuron_n[l - 1] + 1);
	}

	weight_n += layer_neuron_n[l] * (layer_neuron_n[l - 1] + 1);

	uint_t n = sizeof( ann_t ) +                             // ANN
		( sizeof( uint_t ) * layer_n ) +                     // layer_neuron_n[]
		( sizeof( fp_t ) * ( neuron_n +                      // neuron[]
		weight_n +                                      // weight[]
		neuron_n + layer_neuron_n[layer_n - 1] ) );          // delta[]

	// Allocate everything as one structure
	ann_t *ann = malloc( n );

	// ann_t | layer_neuron_n[] | neuron[] | weight[] | delta[]
	ann->n = n;
	ann->layer_n = layer_n;
	ann->weight_n = weight_n;
	ann->neuron_n = neuron_n;
	ann->layer_neuron_n = ( uint_t * ) ( ( uint8_t * ) ann + sizeof( ann_t ) );
	memcpy( ann->layer_neuron_n, layer_neuron_n, sizeof( uint_t ) * layer_n );
	ann->neuron = ( fp_t * ) ( ann->layer_neuron_n + ann->layer_n );
	ann->weight = ann->neuron + ann->neuron_n;
	ann->delta = ann->weight + ann->weight_n;

	ann_set_activation(
		ann,
		SIGMOID,
        SIGMOID
	);

	return ann;
}

void ann_free( ann_t *ann )
{
    free( ann );
}

ann_t * ann_copy( ann_t const *ann )
{
    ann_t *copy = malloc( ann->n );
    memcpy( copy, ann, ann->n );
    
    return copy;
}


////////////////////////////////////////////////////////////////////////////////
// FORWARD PROPAGATION
////////////////////////////////////////////////////////////////////////////////

// o_j = s( sum[1,n+1]{w_ij * o_i} )

void ann_propagation_forward( ann_t *ann, fp_t const * const input, fp_t *output )
{
    fp_t *w_ij = ann->weight;
	fp_t const *x = input;     // Input neuron into y. The type fp_t will be used to access neuron_t in the cas of STORE_USER
	fp_t *y = ann->neuron;     // The current neuron being calculated
	
	uint_t l = 1;

	// Hidden Layers
	for( ; l < ann->layer_n - 1; l++ )
	{
	    for( uint_t j = 0; j < ann->layer_neuron_n[l]; j++ )
	    {
            y[j] = 0;
		    for( uint_t i = 0; i < ann->layer_neuron_n[l - 1]; i++ )
		    {
			    y[j] += x[i] * *w_ij++;
		    }
		    
		    y[j] += *w_ij++;
            y[j] = ann->activation_hidden( y[j] );
	    }

        x = y;
		y += ann->layer_neuron_n[l];
	}

	// Last layer
    for( uint_t j = 0; j < ann->layer_neuron_n[l]; j++ )
    {
	    output[j] = 0;

	    for( uint_t i = 0; i < ann->layer_neuron_n[l - 1]; i++ )
	    {
		    output[j] += x[i] * *w_ij++;
	    }

	    output[j] += *w_ij++;
	    output[j] = ann->activation_output( output[j] );
    }
}


////////////////////////////////////////////////////////////////////////////////
// TRAINING
////////////////////////////////////////////////////////////////////////////////

// E = 0.5 * sum[1, o_n+1]{ (o_i - t_i)^2 }
// d_j = 

void ann_propagation_backward( ann_t *ann, fp_t const *input, fp_t *output, fp_t const *target, fp_t rate )
{
    int_t l = ann->layer_n - 1;
    uint_t i, j, q;
    
    // First output layer delta
	fp_t *d_j = ann->delta + ann->neuron_n;

	// Output Deltas
	for( j = 0; j < ann->layer_neuron_n[l]; j++ )
	{
		d_j[j] = ann->activation_output_partial( output[j] ) * ann_error_partial( output[j], target[j] );
	}

	// First weight in the set between the last layer and the current
	fp_t *w_jq = ann->weight +
		ann->weight_n -
		ann->layer_neuron_n[l] * ann->layer_neuron_n[l - 1] - 
        ann->layer_neuron_n[l];

	fp_t *o_j = ann->neuron + ann->neuron_n;
	fp_t *d_q;

	l--;

	// Hidden Deltas
	for( ; l > 0; --l )
	{
		d_q = d_j;
		d_j -= ann->layer_neuron_n[l];
   		o_j -= ann->layer_neuron_n[l];

		for( j = 0; j < ann->layer_neuron_n[l]; j++ )
		{
			d_j[j] = 0;

			for( q = 0; q < ann->layer_neuron_n[l + 1]; q++ )
			{
				d_j[j] += w_jq[q * ( ann->layer_neuron_n[l] + 1 ) + j] * d_q[q];
			}

			d_j[j] *= ann->activation_hidden_partial( o_j[j] );
		}

		w_jq -= ( ann->layer_neuron_n[l - 1] * ann->layer_neuron_n[l] +	ann->layer_neuron_n[l - 1] );
	}

	fp_t *w_ij = ann->weight;
	d_j = ann->delta;

    l = 1;

	// Input training
	for( j = 0; j < ann->layer_neuron_n[l]; j++ )
	{
		for( i = 0; i < ann->layer_neuron_n[l - 1]; i++ )
		{
			*w_ij -= rate * input[i] * d_j[j];
            w_ij++;
		}
        
		*w_ij -= rate * d_j[j];
        w_ij++;
	}

	l++;
	fp_t *i_i = ann->neuron;

	// Hidden training
	for( ; l < ( int_t ) ann->layer_n; l++ )
	{
		d_j += ann->layer_neuron_n[l - 1];

		for( j = 0; j < ann->layer_neuron_n[l]; j++ )
		{
			for( i = 0; i < ann->layer_neuron_n[l - 1]; i++ )
			{
				*w_ij -= rate * i_i[i] * d_j[j];
                w_ij++;
			}

			*w_ij -= rate * d_j[j];
            w_ij++;
		}
    
		i_i += ann->layer_neuron_n[l - 1];
	}
}


void ann_train_numeric( ann_t *ann, fp_t const *input, fp_t const *target, fp_t rate )
{
    const fp_t EPSILON = 1e-8;

    fp_t *weight_numerical = malloc( sizeof( fp_t ) * ann->weight_n );
    uint_t output_n = ann->layer_neuron_n[ann->layer_n - 1];
    fp_t lower[output_n], upper[output_n];
    fp_t *wbn = weight_numerical;
    fp_t *wb = ann->weight;
    fp_t original, gradient;

    for( uint_t l = 1; l < ann->layer_n; l++ )
    {
		for( uint_t j = 0; j < ann->layer_neuron_n[l]; j++ )
		{
			for( uint_t i = 0; i < ann->layer_neuron_n[l - 1] + 1; i++ )
			{
                // Store original weight
                original = *wb;

                *wb = original + EPSILON;
                ann_propagation_forward( ann, input, upper );

                *wb = original - EPSILON;
                ann_propagation_forward( ann, input, lower );

                *wb = original;

                gradient = ann_error_total( upper, target, output_n ) - ann_error_total( lower, target, output_n );
                gradient /= ( 2 * EPSILON );
                
                *wbn++ = *wb++ + -rate * gradient;
			}
		}
    }

    memcpy( ann->weight, weight_numerical, sizeof( fp_t ) * ann->weight_n );
    free( weight_numerical );
}


////////////////////////////////////////////////////////////////////////////////
// ACTIVATION
////////////////////////////////////////////////////////////////////////////////


static fp_t ann_activation_identity( fp_t x )
{
	return x;
}


static fp_t ann_activation_identity_partial( fp_t x )
{
	return 1;
}


static fp_t ann_activation_binary( fp_t x )
{
	return ( x > 0.0 ) ? 1.0 : 0.0;
}


static fp_t ann_activation_binary_partial( fp_t x )
{
	return 0.0;
}

static fp_t ann_activation_sigmoid( fp_t x )
{
	return 1.0 / ( 1.0 + exp( -x ) );
}


static fp_t ann_activation_sigmoid_partial( fp_t x )
{
	return x * ( 1.0 - x );
}


static fp_t ann_activation_relu( fp_t x )
{
	return ( x > 0.0 ) ? x : 0.0;
}


static fp_t ann_activation_relu_partial( fp_t x )
{
	return ( x > 0.0 ) ? 1.0 : 0.0;
}


static fp_t ann_activation_elu( fp_t x )
{
	return ( x > 0.0 ) ? x : ELU_ALPHA * ( expm1( x ) );
}


static fp_t ann_activation_elu_partial( fp_t x )
{
	return ( x > 0.0 ) ? 1.0 : x + ELU_ALPHA;
}


static fp_t ann_activation_lrelu( fp_t x )
{
	return ( x > 0.0 ) ? x : LRELU_ALPHA * x;
}


static fp_t ann_activation_lrelu_partial( fp_t x )
{
    return ( x > 0.0 ) ? 1.0 : LRELU_ALPHA;
}


static fp_t ann_activation_tanh( fp_t x )
{
    return tanh( x );
}


static fp_t ann_activation_tanh_partial( fp_t x )
{
	return 1.0 - ( x * x );
}

////////////////////////////////////////////////////////////////////////////////
// ERROR
////////////////////////////////////////////////////////////////////////////////


// Mean Squared Error
//
// MSE = 0.5 * ( x - y )^2
// ( y - x )

fp_t ann_error( fp_t o, fp_t t )
{
	fp_t e = o - t;
	e = 0.5 * e * e;

	return e;
}


// Partial Derivative of Mean Squared Error  ( ∂/∂y )

fp_t ann_error_partial( fp_t o, fp_t t )
{
	return ( o - t );
}


// Error Function
// E = 0.5 * sum_[i=1,n]{ ( o_i - t_i )^2 }

fp_t ann_error_total( fp_t const *o, fp_t const *t, uint_t n )
{
	fp_t error = 0;
	for( uint_t i = 0; i < n; i++ )
	{
		error += ann_error( o[i], t[i] );
	}

	return error;
}


////////////////////////////////////////////////////////////////////////////////
// SETTER/GETTER
////////////////////////////////////////////////////////////////////////////////


void ann_set_activation( ann_t *ann, ann_activation_t activation_hidden, ann_activation_t activation_output )
{
	ann->activation_hidden = ACTIVATION[activation_hidden][0];
	ann->activation_hidden_partial = ACTIVATION[activation_hidden][1];
	ann->activation_output = ACTIVATION[activation_output][0];
	ann->activation_output_partial = ACTIVATION[activation_output][1];
}


////////////////////////////////////////////////////////////////////////////////
// UTILITY
////////////////////////////////////////////////////////////////////////////////


void ann_random( ann_t *ann )
{
    uint_t l, j, i;
    fp_t *wb = ann->weight;
    for( l = 1; l < ann->layer_n; l++ )
    {
        for( j = 0; j < ann->layer_neuron_n[l]; j++ )
        {
            i = 0;
            for( ; i < ann->layer_neuron_n[l - 1]; i++ )
            {
                *wb++ = ann_random_range( -1.0, 1.0 );
            }

            *wb++ = 0;
        }
    }
}


static fp_t ann_random_range( fp_t low, fp_t high )
{
	return ( low + ( ( fp_t ) rand() ) * ( high - low ) / RAND_MAX );
}


void ann_print_neuron( ann_t *ann, fp_t const * const input, fp_t const * const output )
{
	printf( "NEURON:\n\n" );

	for( uint_t i = 0; i < ann->layer_neuron_n[0]; i++ )
	{
		fprintf( stderr, "  %+.*f", PRINT_PRECISION, input[i] );
	}

	fputs( "\n", stderr );

	fp_t *n = ann->neuron;

	for( uint_t l = 1; l < ann->layer_n - 1; l++ )
	{
		for( uint_t i = 0; i < ann->layer_neuron_n[l]; i++ )
		{
			fprintf( stderr, "  %+.*f", PRINT_PRECISION, *n );
			n++;
		}

		fputs( "\n", stderr );
	}

	for( uint_t i = 0; i < ann->layer_neuron_n[ann->layer_n - 1]; i++ )
	{
		fprintf( stderr, "  %+.*f", PRINT_PRECISION, output[i] );
	}

	fputs( "\n\n", stderr );
}


void ann_print_weight( ann_t *ann )
{
	fprintf( stderr, "\nWEIGHT | BIAS\n\n" );

	fp_t *weight = ann->weight;

	for( uint_t i = 1; i < ann->layer_n; i++ )
	{
		for( uint_t j = 0; j < ann->layer_neuron_n[i - 1] + 1; j++ )
		{
			for( uint_t k = 0; k < ann->layer_neuron_n[i]; k++ )
			{
				fprintf( stderr, "  %+.*f", PRINT_PRECISION, *( weight + j + k * ( ann->layer_neuron_n[i - 1] + 1 ) ) );
			}

			fputs( "\n", stderr );
		}

		weight += ann->layer_neuron_n[i - 1] * ann->layer_neuron_n[i] + ann->layer_neuron_n[i];

		fputs( "\n", stderr );
	}
}


#endif // ANN_IMPLEMENTATION