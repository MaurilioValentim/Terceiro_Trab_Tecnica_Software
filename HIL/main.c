
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "scicomm.h"
#include "shared_vars.h"

//
// Defini��es de Constantes
//
#define F_PWM                  10000.0f     // Frequ�ncia de chaveamento (Hz)
#define T_PWM                  (1.0f / F_PWM) // Per�odo de chaveamento (s)
#define DT_SIM                 0.000001f    // Passo de simula��o (5 �s)
#define N_STEPS_PER_CYCLE      (uint32_t)(T_PWM / DT_SIM) // Passos por ciclo PWM

// Par�metros do Conversor Buck
#define VIN                    10.0f       // Tens�o de entrada (V)
#define L                      0.001f      // Indut�ncia (H)
#define C                      0.00001f    // Capacit�ncia (F)
#define R_LOAD                 10.0f       // Carga resistiva (Ohm)

// Constantes auxiliares (evita divis�es repetidas no loop)
#define INV_L                  (DT_SIM / L)
#define INV_C                  (DT_SIM / C)
#define INV_R_LOAD             (1.0f / R_LOAD)

//
// Vari�veis Globais da Simula��o
//
volatile float32_t g_vout_sim = 0.0f;        // Tens�o de sa�da simulada
volatile float32_t g_il_sim = 0.0f;          // Corrente no indutor simulada
volatile bool g_switch_on = false;           // Estado da chave (true = ligada)
volatile bool g_new_step_ready = false;      // Flag para novo passo de simula��o

float32_t v_l, i_c;                          // Variaveis para calculos
volatile uint16_t dac;
volatile uint16_t CMPSS_teste = 0;

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
              // Desativa o passo de simula��o
              g_new_step_ready = false;

              // Tens�o no indutor
              //v_l = g_switch_on ? (VIN - g_vout_sim) : (-g_vout_sim);
              v_l = g_switch_on ? (VIN) : (VIN - g_vout_sim);
              // Corrente do capacitor
              i_c = g_il_sim - (g_vout_sim * INV_R_LOAD);

              // Atualiza��o via m�todo de Euler
              g_il_sim += INV_L * v_l;
              g_vout_sim += INV_C * i_c;

              // Verifica se a tens�o � maior do que a escala
              if(g_vout_sim > SCALE)
                  dac = MAX_DAC_VAL;
              else
                  dac = (float)g_vout_sim * (float)DAC_TESTE;


              // Teste do CMPSS
              if(CMPSS_teste == 1){
                  DAC_setShadowValue(DAC1_BASE, 2000);
              }

              // Envia o valor de Vo para o ADC, ou seja, envia para o CLA para o controle do pwm
              DAC_setShadowValue(DAC0_BASE, (uint16_t) (dac));

             }
        }

}

// CLA para o calculo do controle, ativada com o pwm

__interrupt void cla1Isr1 ()
{
    Interrupt_clearACKGroup(INT_myCLA01_INTERRUPT_ACK_GROUP);
}

// Detecta Borda de Subida e Descida para ter a leitura da chave

__interrupt void INT_GPIO_PWM_XINT_ISR(void)
{
    // Leitura da chave ( ON or OFF )
    g_switch_on = GPIO_readPin(GPIO_PWM);

    Interrupt_clearACKGroup(INT_GPIO_PWM_XINT_INTERRUPT_ACK_GROUP);
}

// Passo da Simula��o, ativada pelo timer da propria CPU

__interrupt void INT_myCPUTIMER0_ISR(void)
{
    // Ativa o passo da simula��o
    g_new_step_ready = true;

    Interrupt_clearACKGroup(INT_myCPUTIMER0_INTERRUPT_ACK_GROUP);
}

