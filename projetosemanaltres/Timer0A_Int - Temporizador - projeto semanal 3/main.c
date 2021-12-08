 //TERCEIRO PROJETO SEMANAL - LUIZA TEDESCHI COSTA GOMES E MARIA LUIZA MACHIOSKI SILVANO

//Projeto de temporizador aplicado na máquina de estados

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
  Program 7.5, example 7.6

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

//includes com as bibliotecas e códigos externos usados
//para desenvolver o projeto
#include "PLL.h" 						//inicia o temporizador em 80 MHz
#include "Timer0.h"					
#include "tm4c123gh6pm.h"		//possui os defines já pre-estabelecidos
#include <stdint.h>

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF0       (*((volatile uint32_t *)0x40025001))
#define PD3       (*((volatile uint32_t *)0x40007020))
//define do registrador DATA que habilita apenas a mudança de um bit escolhido
//nesse caso PF1 autoriza apenas mudar o bit 1 do DATA do portF
//desse modo, é possível modificar os pinos individualmente

static uint32_t i = 0, j=0;		//variável i do exercício
//controla qual estado está sendo executado

//protótipos das funções do prof Valvano e das criadas para o código
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode
void PortF_Init(void);  			//protótipo da função de inic.
void Delay25 (void);
void Delaypadrao(float t);

//inicialização do portF
//PF1,2,3 saídas
//PF4,0 entradas pull up
void PortF_Init(void){
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;    // F clock
  delay = SYSCTL_RCGC2_R;          // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;          // allow changes to PF4-0 
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PORTF
  GPIO_PORTF_PCTL_R = 0x00000000;  // configure PORTF as GPIO
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF0, PF4 input, PF1-PF3 output
  GPIO_PORTF_AFSEL_R &= ~0x1F;     // disable alt funct 
  GPIO_PORTF_PUR_R = 0x11;         // enable weak pull-up on PF4 and PF0
  GPIO_PORTF_DEN_R |= 0x1F;        // enable digital I/O
}

//inicialização do portD
//PD3 como saída
void PortD_Init(void){    
	SYSCTL_RCGCGPIO_R |= 0x00000008; // (a) activate clock for port D          
  GPIO_PORTD_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port D
  GPIO_PORTD_CR_R = 0x08;           // allow changes to PD3
  GPIO_PORTD_DIR_R |=  0x08;    // output on PD3
  GPIO_PORTD_AFSEL_R &= ~0x1F;  //     disable alt funct on PD
  GPIO_PORTD_DEN_R |= 0x1F;     //     enable digital I/O on PD3
  GPIO_PORTD_PCTL_R &= ~0x000FFFFF; // configure PD as GPIO
  GPIO_PORTD_AMSEL_R = 0;       //     disable analog functionality on PD
}

//função chamada quando ocorre interrupção devido ao Timer
//realiza a mudança de estados
//variável "i" aumenta uma unidade a cada interrupção
//mas não em todos os estados, em alguns, existe uma condição
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;	// acknowledge TIMER0A timeout
//	TIMER0_ICR_R = 0x00000001;    		// clear TIMER0A timeout flag
											
	if(i==3) //se estiver no estado 3, volta para o estado 1
	{
		i=0; //zera pq a soma de i acontece depois
	}
	//se no estado 2 PF0 estiver pressionada
	//vai para o estado 4
	if(i==2&&PF0==0x00)
	{
		i=3; //i=3 porque será somado 1 depois dessa análise
	}
	//repete o estado 5 2x
	//se for a primeira vez, j=0, e repete
	if(i==5&&j==0){
		i=4;
		j=1;
	}
		
	i=i+1;	//soma a variável
		
	//reinicia o loop
	if(i==7)
	{
		i=0;
		j=0;
	}
}

//rotina principal: inicializações das portas e e temporizador
//rotina da máquina de estados
int main(void){ volatile unsigned long delay;

  PLL_Init();                      // bus clock at 80 MHz
	PortF_Init();										 // initialize PortF
	PortD_Init();
  Timer0_Init(800000000); 				// initialize timer1 (0,1 Hz)
	//10 segundos para cada estado

  EnableInterrupts();
  while(1){
	switch(i)
    {
				//ESTADO 0
        case 0:
						//colocando as saídas PF1, PF2 e PF3 desligadas
            PF1=0x00;
						PF2=0x00;
						PF3=0x00;
						//aplicando nível lógico 0 na saída PD3 (só ela foi autorizada a ter mudanças)
						PD3=0x00;
            break;
				
				//ESTADO 1
        case 1:
					//pisca led azul com 2,5 Hz
						PF2=0x04;
						Delay25();
						PF2=0x00;
						Delay25();
						//colocando as saídas PF1 e PF3 desligadas
            PF1=0x00;
						PF3=0x00;
						//aplicando nível lógico 0 na saída PD3
						PD3=0x00;
            break;
						
				//ESTADO 2
				case 2:	
					//espera 10s, se PF0 não for pressionado, manda para estado 3
					//se pressionado vai para estado 4
            break;
						
				//ESTADO 3
        case 3:
				//pisca 5x o led verde e quando o estado termina, próx é o 1
						PF3=0x08;
						Delaypadrao(1.05);							
						//delay de 1,05s para que durante 10 s
						//seja repetido 5x
						PF3=0x00;
						Delaypadrao(1.05);
            PF1=0x00;
						PF2=0x00;
            break;
						
				//ESTADO 4
        case 4:
					//pisca o led vermelho 2x
	            PF1 = 0x02;	//vermelho é PF1
							Delaypadrao(2.55);
							//delay de 2,55s para que durante 10 s
							//seja repetido 2x
							PF1 = 0x00;				
							Delaypadrao(2.55);
            break;	
						
				//ESTADO 5				
        case 5:		
					//pisca o led vermelho em 2,5 Hz
					//esse estado é chamado duas vezes
	            PF1 = 0x02;						
							Delay25();
							PF1 = 0x00;				
							Delay25();
						break;
				
				//ESTADO 6		
        case 6:		
					//pisca em sequência os 3 LEDS
					//repete 4x essa ação
							PD3=0x08;
	            PF1 = 0x02;			//led vermelho			
							PF3 = 0x00;
							Delaypadrao(0.834);
							//delay de 0,834s para que durante 10 s
							//seja repetido 4x
							PF1 = 0x00;	
							PF2 = 0x04;			//led azul			
							Delaypadrao(0.834);
							PF3 = 0x08;	  //led verde	
							PF2 = 0x00;							
							Delaypadrao(0.834);
            break;
				return 0;
		}
  }
}

// Delay de 0,2 seg para atingir freq. de 2,5 Hz. 
//Usado o padrão dos exemplos
//apenas usei regra de três para chegar no tempo desejado
void Delay25(void){unsigned long volatile time;
  time = 0.9092934335*727240*200/91;  // 0.2 sec
  while(time){
		time--;
  }
}
//função que volta o tempo colocado na função
//se t=1, gera um delay de 1s
void Delaypadrao(float t){unsigned long volatile time;
  time = t*4.546467168*727240*200/91;  // 1 sec
  while(time){
		time--;
  }
}
