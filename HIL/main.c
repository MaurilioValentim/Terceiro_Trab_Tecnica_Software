
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "scicomm.h"

#pragma DATA_SECTION(vo,"CpuToCla1MsgRAM");
float vo;
#pragma DATA_SECTION(d,"Cla1ToCpuMsgRAM");
float d;

int teste = 0;

volatile uint16_t dac;
volatile uint16_t adc;

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
            NOP;
        }

}

__interrupt void cla1Isr1 ()
{

    Interrupt_clearACKGroup(INT_myCLA01_INTERRUPT_ACK_GROUP);
}

__interrupt void INT_ADC0_1_ISR(void)
{
    adc = ADC_readResult(ADC0_RESULT_BASE, ADC0_SOC0);
    vo = adc;
    CLA_forceTasks(myCLA0_BASE,CLA_TASKFLAG_1);
    ADC_clearInterruptStatus(ADC0_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INT_ADC0_1_INTERRUPT_ACK_GROUP);

}

__interrupt void INT_GPIO0_XINT_ISR(void)
{

    DAC_setShadowValue(DAC0_BASE, (uint16_t) (dac));
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

