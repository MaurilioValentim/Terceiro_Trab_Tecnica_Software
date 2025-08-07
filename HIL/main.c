
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "scicomm.h"
#include "shared_vars.h"

//
// Definições de Constantes
//
#define F_PWM                  10000.0f     // Frequência de chaveamento (Hz)
#define T_PWM                  (1.0f / F_PWM) // Período de chaveamento (s)
#define DT_SIM                 0.000003f    // Passo de simulação (5 µs)
#define N_STEPS_PER_CYCLE      (uint32_t)(T_PWM / DT_SIM) // Passos por ciclo PWM

// Parâmetros do Conversor Buck
#define VIN                    12.0f       // Tensão de entrada (V)
#define L                      0.001f      // Indutância (H)
#define C                      0.00001f    // Capacitância (F)
#define R_LOAD                 10.0f       // Carga resistiva (Ohm)

// Constantes auxiliares (evita divisões repetidas no loop)
#define INV_L                  (DT_SIM / L)
#define INV_C                  (DT_SIM / C)
#define INV_R_LOAD             (1.0f / R_LOAD)

//
// Variáveis Globais da Simulação
//
volatile float32_t g_vout_sim = 0.0f;        // Tensão de saída simulada
volatile float32_t g_il_sim = 0.0f;          // Corrente no indutor simulada
volatile bool g_switch_on = false;           // Estado da chave (true = ligada)
volatile bool g_new_step_ready = false;      // Flag para novo passo de simulação

float32_t v_l, i_c;                          // Variaveis para calculos
volatile uint16_t dac_v;
volatile uint16_t CMPSS_teste = 0;           // Variavel para ativar o CMPSS
volatile uint16_t dac_i;

void main(void)
{
        Device_init();

        Interrupt_initModule();

        Interrupt_initVectorTable();

        Board_init();
        EINT;
        ERTM;


        for(;;)
        {

            if (g_new_step_ready)
             {
              // Desativa o passo de simulação
              g_new_step_ready = false;

              // Tensão no indutor
              v_l = g_switch_on ? (VIN) : (VIN - g_vout_sim);

              // Corrente do capacitor
              i_c = g_switch_on ?  (- g_vout_sim * INV_R_LOAD) : ( g_il_sim - (g_vout_sim * INV_R_LOAD) );

              // Atualização via método de Euler
              g_il_sim += INV_L * v_l;
              g_vout_sim += INV_C * i_c;

              // Verifica se a tensão é maior do que a escala
              if(g_vout_sim > SCALE_V)
                  dac_v = MAX_DAC_VAL;
              else if (g_vout_sim <= 0)
                  dac_v = 0;
              else
                  dac_v = (float)g_vout_sim * (float)DAC_TESTE;

              // Verifica se a Corrente é maior do que a escala
              if (g_il_sim > SCALE_I)
                   dac_i = MAX_DAC_VAL;
              else if (g_il_sim <= 0)
                   dac_i = 0;
              else
                   dac_i = (float) g_il_sim * DAC_RESOLUTION_DA_I;

              // Teste do CMPSS
              if(CMPSS_teste == 1){ // Força um valor acima do CMPSS, ou seja, ativa o CMPSS para parar o codigo
                  DAC_setShadowValue(DAC1_BASE, 4000);
              }
              else if(CMPSS_teste == 2){ // Caso Seja One-Shot isso faz com que reseta o EPWM
                  // Clear trip flags
                  DAC_setShadowValue(DAC1_BASE, (uint16_t) (dac_i));
                  EPWM_clearTripZoneFlag(myEPWM0_BASE, EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST | EPWM_TZ_FLAG_DCAEVT1);
              }
              else{
                  // Envia o valor da corrente
                  DAC_setShadowValue(DAC1_BASE, (uint16_t) (dac_i));
              }

              // Envia o valor de Vo para o ADC, ou seja, envia para o CLA para o controle do pwm
              DAC_setShadowValue(DAC0_BASE, (uint16_t) (dac_v));

             }
        }

}


// Detecta Borda de Subida e Descida para ter a leitura da chave

__interrupt void INT_GPIO_PWM_XINT_ISR(void)
{
    // Leitura da chave ( ON or OFF )
    g_switch_on = GPIO_readPin(GPIO_PWM);

    Interrupt_clearACKGroup(INT_GPIO_PWM_XINT_INTERRUPT_ACK_GROUP);
}

// Passo da Simulação, ativada pelo timer da propria CPU

__interrupt void INT_myCPUTIMER0_ISR(void)
{
    // Ativa o passo da simulação
    g_new_step_ready = true;

    Interrupt_clearACKGroup(INT_myCPUTIMER0_INTERRUPT_ACK_GROUP);
}

