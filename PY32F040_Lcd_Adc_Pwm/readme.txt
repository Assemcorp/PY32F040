================================================================================
                              Sample Description
================================================================================
Function Description:
This sample demonstrates the use of LCD, ADC, and PWM. The ADC is connected to pin B1, and a potentiometer is attached to this pin. As you turn the potentiometer, the PWM duty cycle output from pin B3 changes accordingly.
================================================================================
Test Environment:
Test Board: PY32F040_AssemCorp_DevKit 
MDK Version: 5.28  
IAR Version: 9.20  
GCC Version: GNU Arm Embedded Toolchain 10.3-2021.10
================================================================================
Execution Steps:
1. Compile and download the program to the board.
2. Turn the potentiometer connected to B1 and observe the change in PWM duty cycle on pin B3.
================================================================================

B1   -->  ADC input (potentiometer)
B3   -->  PWM output