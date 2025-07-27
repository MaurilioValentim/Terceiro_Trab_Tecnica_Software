/*
 * shared_vars.h
 *
 *  Created on: 4 de jul de 2025
 *      Author: Prof. Guilherme Márcio Soares
 */

#ifndef SHARED_VARS_H_
#define SHARED_VARS_H_

#define SCALE           50U      // Fundo de escala (V)
#define MAX_DAC_VAL     4096U    // Shadow_value máximo do DAC

#define ADC_RESOLUTION (SCALE/MAX_DAC_VAL)
#define DAC_RESOLUTION (MAX_DAC_VAL/SCALE)
#define ADC_TESTE 0.01221001221001221001221001221001
#define DAC_TESTE 81.92


#endif /* SHARED_VARS_H_ */
