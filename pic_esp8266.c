//Projeto - PIC com Thingspeak e ESP8266
//Autor: Pedro Bertoleti
//Data: 11/2015

#include <18F4520.h>
#include "pic_esp8266.h"
#use delay(clock=10000000) 	//Clock de 10MHz (clock útil de 2,5MHz)
#fuses HS,PUT,NOPROTECT
#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,ERRORS)
#priority INT_TIMER1, INT_EXT, INT_RDA  //ordem de prioridad das interrupções (ordem decrescente de prioridade)

//defines gerais: 
#define BREATHING_LIGHT                    PIN_A0
#define BOTAO_CONTADOR                     PIN_B0
#define TAMANHO_COMANDO_MODO_ESTACAO       sizeof(ComandoModoEstacao);
#define TAMANHO_COMANDO_CONECTA_ROTEADOR   sizeof(ComandoConectaRoteador);
#define TAMANHO_COMANDO_MULTI_CONEXOES     sizeof(ComandoMultiplasConexoes);
#define TAMANHO_COMANDO_HTTP_THINGSPEAK    sizeof(ComandoConectaHTTPThingspeak);
#define TAMANHO_COMANDO_SEND_THINGSPEAK    sizeof(ComandoSendDadoThingspeak);

//prototypes
void ConfigInterrupcaoEXT(void);
void ConfigInterrupcaoUART(void);
void ConfigTimer1(void);
void ESP8266SetaModoEstacao(void);
void ESP8266ConectaRoteador(void);
void ESP8266MultiplasConexoes(void);
void ESP8266HTTPThingspeak(void);
void ESP8266PreparaEnvioHTTP(void);
void ESP8266SendThingspeak(char valor);
void SetupESP8266(void);

//tratamento da interrupção externa
#int_EXT 
void  EXT_isr(void) 
{ 
	//debouncing
    delay_ms(10);
    if (input(BOTAO_CONTADOR) == 1)
		Contador++;    
} 

//tratamento da interrupção serial
#INT_RDA
void serial_isr()
{ 
    char ByteLido;
    ByteLido = getc();
} 

//tratamento da interrupção de timer
#INT_TIMER1
void TrataTimer1()
{
	ContadorIntTimer++;
	ContadorEnvioDadosThingspeak++;

    //contabiliza tempo de envio do dado ao thingspeak
    if (ContadorEnvioDadosThingspeak >= VALOR_MAX_CONTADOR_TEMPO_THINGSPEAK)
		DeveEnviarDadoThingspeak = SIM;

    //controla tempo para piscada do breathing light
    if (ContadorIntTimer >= 10)   //cada "tick" do timer1 tem 0,1s. Logo, 10 "ticks" equivalem a 1 segundo
    {
		//troca o estado da saída do breathing light
		output_toggle(BREATHING_LIGHT);    
    	ContadorIntTimer=0;
	}

    set_timer1(VALOR_INICIAL_TIMER1);
     
    //configura timer e religa interrupções	
    set_timer1(VALOR_INICIAL_TIMER1);
}


//função de configuração da interrupção UART
//parametros: nenhum
//saida: nenhum
void ConfigInterrupcaoUART(void)
{    
    enable_interrupts(INT_RDA);
    enable_interrupts(GLOBAL);
}

//função de configuração da interrupção externa
//parametros: nenhum
//saida: nenhum
void ConfigInterrupcaoEXT(void)
{    
    Contador = 0;
    enable_interrupts(INT_EXT);
    enable_interrupts(GLOBAL);
    ext_int_edge(L_TO_H);
}


//função de configuração do Timer1
//parametros: nenhum
//saida: nenhum
void ConfigTimer1(void)
{
    // - Frequencia do oscilador interno (10000000/4)=2,5Mhz (por default, o PIC funciona a 1/4 da frequencia de clock estabelecida)
	// - Se o Timer1 tem 16 bits, seu valor máximo de contagem é 0xFFFF (65535)	
	// - Com 2,5MHz de frequencia util, temos que cada ciclo de máquina terá, em segundos: 1 / 2,5MHz = 0,0000004 (0,4us)
    // - Utilizando o prescaler do microcontrolador em 4 (ou seja, a frequencia util do timer1 é 1/4 da frequencia util do pic), temos:
    //   Periodo minimo "contável" pelo Timer1 =  (1 / (2,5MHz/4))   = 0,0000016 (1,6us)
    // - Logo, a cada 16 bits contados, teremos: 65536 * 1,6us =  0,1048576s
    // - visando maior precisão, sera feito um timer de 0,1s. Logo:   
    //             0,1048576s   ---  65536
    //                 0,10s     ---     x        x = 62500
    // Logo, o valor a ser setao no timer1 é: 65536 - 62500 = 3036
    ContadorIntTimer = 0;
    setup_timer_1(T1_INTERNAL| T1_DIV_BY_4);
    set_timer1(VALOR_INICIAL_TIMER1);
    enable_interrupts(INT_TIMER1);      
    enable_interrupts(GLOBAL);
}

//função que configura o ESP8266 para operar em modo estação (se conecta em um roteador)
//parametros: nenhum
//saida: nenhum
void ESP8266SetaModoEstacao(void)
{
	puts(ComandoModoEstacao);
	delay_ms(500);
}

//função que conecta o ESP8266 em um roteador
//parametros: nenhum
//saida: nenhum
void ESP8266ConectaRoteador(void)
{
	puts(ComandoConectaRoteador);
	delay_ms(7000);
}

//função que configura o ESP8266 para operar com multiplas conexoes
//parametros: nenhum
//saida: nenhum
void ESP8266MultiplasConexoes(void)
{
	puts(ComandoMultiplasConexoes);
	delay_ms(500);
}

//função que conecta o ESP8266 via HTTP ao Thingspeak
//parametros: nenhum
//saida: nenhum
void ESP8266HTTPThingspeak(void)
{
	puts(ComandoConectaHTTPThingspeak);
	delay_ms(5000);
}

//função que prepara o ESP8266 para enviar dados via HTTP ao Thingspeak
//parametros: nenhum
//saida: nenhum
void ESP8266PreparaEnvioHTTP(void)
{
	puts(ComandoPreparaEnvioThingspeak);
	delay_ms(500);	
}

//função que envia um dado ao Thingspeak
//parametros: dado (número de 1 byte)
//saida: nenhum
void ESP8266SendThingspeak(char valor)
{
	char ComandoEnvioDadoHTTP[50];

    memset(ComandoEnvioDadoHTTP,0,sizeof(ComandoEnvioDadoHTTP));
	sprintf(ComandoEnvioDadoHTTP,"%s%03d\r\n\0",ComandoSendDadoThingspeak,valor);
	puts(ComandoEnvioDadoHTTP);
	delay_ms(5000);
}

//função que inicializa, configura e conecta ESP8266 ao roteador
//parametros: nenhum
//saida: nenhum
void SetupESP8266(void)
{
	ESP8266SetaModoEstacao();
	ESP8266ConectaRoteador();
	ESP8266MultiplasConexoes();
}

//programa principal
void main(void)
{
    //inicializa maquina de controle da ESP8266
	EstadoMaquinaESP8266 = ESTADO_MODO_ESTACAO;

	//configura Timer1 
    ConfigTimer1();

    //inicializa contador de tempo de envio de dados para o Thingspeak
	ContadorEnvioDadosThingspeak = 0;
	DeveEnviarDadoThingspeak = NAO;

    //configuração das interrupções (UART e interrupção externa)
 	ConfigInterrupcaoUART();
	ConfigInterrupcaoEXT();

    //inicializa, configura e conecta ESP8266 ao roteador
    SetupESP8266();

	while(1)
	{
		if (DeveEnviarDadoThingspeak == SIM)
		{
			ESP8266HTTPThingspeak();
			ESP8266PreparaEnvioHTTP();
			ESP8266SendThingspeak(Contador);			
			ContadorEnvioDadosThingspeak = 0;		
			DeveEnviarDadoThingspeak = NAO;			
		}

	}
}