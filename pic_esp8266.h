//Header geral - PIC com Thingspeak e ESP8266
//Autor: Pedro Bertoleti
//Data: 11/2015

//defines gerais:
#define VALOR_INICIAL_TIMER1    3036

#define ESTADO_MODO_ESTACAO     0
#define ESTADO_CONECTA_ROTEADOR 1
#define ESTADO_MULTI_CONEXOES   2
#define ESTADO_HTTP_THINGSPEAK  3
#define ESTADO_SEND_THINGSPEAK  4

#define TEMPO_SEND_THINGSPEAK                 30    //envio para o thingspeak é feito em um período de 30s
#define VALOR_MAX_CONTADOR_TEMPO_THINGSPEAK   TEMPO_SEND_THINGSPEAK*10 

#define SIM     1
#define NAO     0

//variáveis globais:

char ComandoModoEstacao[]="AT+CWMODE=1\r\n\0";
char ComandoConectaRoteador[]="AT+CWJAP=\"rrrr\",\"ssss\"\r\n\0"; //substituir rrrr e ssss pelo ssid/nome da rede e senha, respectivamente
char ComandoMultiplasConexoes[]="AT+CIPMUX=1\r\n\0";
char ComandoConectaHTTPThingspeak[]="AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80\r\n\0";
char ComandoPreparaEnvioThingspeak[]="AT+CIPSEND=4,46\r\n\0";   //46 é o tamanho do comando get (incluindo o \r\n\0)
char ComandoSendDadoThingspeak[]="GET /update?key=kkkkkkkkkkkkkkkk&field1=";  //substituir kkkkkkkkkkkkkkkk pelo token/identificador de canal do thingspeak
volatile char Contador;
char ContadorIntTimer;
long ContadorEnvioDadosThingspeak;
volatile char EstadoMaquinaESP8266;
char DeveEnviarDadoThingspeak;