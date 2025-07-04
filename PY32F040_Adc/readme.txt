================================================================================
                              Sample Description
================================================================================
Function Description:
This sample demonstrates the interrupt function of the ADC. When the value of the potentiometer connected to port A0 changes, the position of the 6 LEDs on the board updates accordingly.
================================================================================
Test Environment:
Test Board: PY32F040_AssemCorp_DevKit 
MDK Version: 5.28  
IAR Version: 9.20  
GCC Version: GNU Arm Embedded Toolchain 10.3-2021.10
================================================================================
Execution Steps:
1. Compile and download the program;
2. 6 LEDs on the board updates accordingly.
================================================================================
Notes:
Connect the PC to the STK board through a USB to TTL module. The wiring between
the STK board and the USB to TTL module should be as follows:

PA0   -->  ADC 
================================================================================
