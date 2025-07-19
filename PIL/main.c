
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
    protocolSendData(SCI0_BASE, &d,sizeof(float));
    Interrupt_clearACKGroup(INT_myCLA01_INTERRUPT_ACK_GROUP);
    Interrupt_clearACKGroup(INT_SCI0_RX_INTERRUPT_ACK_GROUP);
    SCI_clearInterruptStatus(SCI0_BASE,SCI_INT_RXFF);
}

__interrupt void INT_SCI0_RX_ISR ()
{
    protocolReceiveData(SCI0_BASE,&vo,sizeof(float));

    CLA_forceTasks(myCLA0_BASE,CLA_TASKFLAG_1);
}
