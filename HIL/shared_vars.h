/*
 * shared_vars.h
 *
 *  Created on: 4 de jul de 2025
 *      Author: Prof. Guilherme Márcio Soares
 */

#ifndef SHARED_VARS_H_
#define SHARED_VARS_H_

#define SCALE_V           50U      // Fundo de escala da tensão (V)
#define SCALE_I           10U    // Fundo de escala da corrente (A)
#define MAX_DAC_VAL     4095U    // Shadow_value máximo do DAC

#define ADC_TESTE ((float)SCALE_V/(float)MAX_DAC_VAL)
#define DAC_TESTE ((float)MAX_DAC_VAL/(float)SCALE_V)
#define DAC_RESOLUTION_DA_I ((float)MAX_DAC_VAL/(float)SCALE_I)

#endif /* SHARED_VARS_H_ */
