
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "scicomm.h"
#include "shared_vars.h"

/*
#pragma DATA_SECTION(vo,"CpuToCla1MsgRAM");
float vo;
#pragma DATA_SECTION(d,"Cla1ToCpuMsgRAM");
float d;
*/

int teste = 0;

volatile uint16_t dac;

//
// Defini��es de Constantes
//
#define F_PWM                  200000.0f     // Frequ�ncia de chaveamento (Hz)
#define T_PWM                  (1.0f / F_PWM) // Per�odo de chaveamento (s)
#define DT_SIM                 0.000005f    // Passo de simula��o (5 �s)
#define N_STEPS_PER_CYCLE      (uint32_t)(T_PWM / DT_SIM) // Passos por ciclo PWM

// Par�metros do Conversor Buck
#define VIN                    12.0f       // Tens�o de entrada (V)
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
volatile uint32_t g_step_counter = 0;        // Contador de passos dentro do ciclo PWM
volatile bool g_switch_on = false;           // Estado da chave (true = ligada)
volatile bool g_new_step_ready = false;
float32_t v_l, i_c;

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
            // Executa apenas se a ISR indicar que � hora de simular
            if (g_new_step_ready)
             {

              g_new_step_ready = false;

              // Tens�o no indutor
              v_l = g_switch_on ? (VIN - g_vout_sim) : (-g_vout_sim);

              // Corrente do capacitor
              i_c = g_il_sim - (g_vout_sim * INV_R_LOAD);

              // Atualiza��o via m�todo de Euler
              g_il_sim += INV_L * v_l;
              g_vout_sim += INV_C * i_c;

              if(g_vout_sim > SCALE)
                  dac = MAX_DAC_VAL;
              else
                  dac = g_vout_sim * DAC_RESOLUTION;

              DAC_setShadowValue(DAC0_BASE, (uint16_t) (dac));

             }
        }

}

// CLA

__interrupt void cla1Isr1 ()
{
    Interrupt_clearACKGroup(INT_myCLA01_INTERRUPT_ACK_GROUP);
}

// Detecta Borda de Subida e Descida

__interrupt void INT_GPIO_PWM_XINT_ISR(void)
{
    g_switch_on = GPIO_readPin(GPIO_PWM);

    Interrupt_clearACKGroup(INT_GPIO_PWM_XINT_INTERRUPT_ACK_GROUP);
}

// Passo da Simula��o

__interrupt void INT_myCPUTIMER0_ISR(void)
{
    g_new_step_ready = true;

    Interrupt_clearACKGroup(INT_myCPUTIMER0_INTERRUPT_ACK_GROUP);
}

