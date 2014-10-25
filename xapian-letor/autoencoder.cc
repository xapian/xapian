/* 
 * Copyright (C) 2012 Rishabh Mehrotra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
 
#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <autoencoder.h>
#include <iostream>
#include "dimred/ya_ate_dimred.h"

#include <list>
#include <map>
#include <iostream>


using namespace std;
using namespace Xapian;
using namespace yala;

Autoencoder::Autoencoder() {
}

void 
Autoencoder::train_autoencoder() {

    // Set up a random number generator and seed it
    MathRandom<MathMersenneTwister> rng;
    unsigned long seed=123456789;
    rng.seed(seed);
  
    // Get a dimensionality reduction object
    //YAATEReduce redmet;
    YADimReduce<double> *redmet=NULL;
    redmet=new YAATEReduce<double>;
    // Low dimensionality
    int low_dim=4;

    // Tell redmet which random number generator to use.
    redmet.set_rng(&rng);
    // Give the layer sizes for the neural network
    int layers[4]={35 64 32 4};
    redmet.ae_layers(YA_WRAP(layers,1,4));


    double trac=0.8;
    redmet.train_split(tfrac);


    redmet.rbm_iters(10);
    redmet.bp_iters(30);
    
    //demo dataset
    int nS=400;
    double input_matrix[nS*3];
    double output_matrix[nS*2];

    // Creating wrappers
    YA_WRAP(double) winput(input_matrix,nS,3);
    YA_WRAP(double) woutput(output_matrix,nS,2);

    redmet.rbm_size(10);
    redmet.bp_size(100);
    
    //Autoencoder Works
    redmet.find(winput,woutput,low_dim,eigopts);
}

