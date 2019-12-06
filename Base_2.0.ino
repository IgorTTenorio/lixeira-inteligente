/*
---------------------------------------------------------------------------------------------------
    VERSÃO 2.0 - MÓDULO BASE LIXEIRA INTELIGENTE
    
    Elaborador: Igor Tenório Teixeira                                        Data: 26/05/19
    
    Conteúdo: Nesta versão do módulo base será testado a coleta de dados e informações do protótipo
    incrementado da lixeira. São esses dados de nível e peso do lixo. Os valores coletados serão 
    enviados para o Gerente_2.0 que definirá o número de medidas a serem realizadas.
---------------------------------------------------------------------------------------------------
*/
// Inclusão das bibliotecas
#include <HX711.h>                          // Biblioteca HX711

// Declaração de Variáveis e Valores

// Sensor Ultrassônico HC-SR04
int pinTrig = 2;                            // Pino usado para disparar os pulsos do sensor
int pinEcho = 3;                            // Pino usado para ler a saída do sensor
float TimeEcho = 0;                         // Variável de tempo entre emissão e recepção
float Dist_C = 0;                           // Variável de distância em centímetros
float Dist_M = 0;                           // Variável de distância em metros
const float VelSound_mpers = 340;           // Em metros por segundo 
const float VelSound_mperus = 0.000340;     // Em metros por microsegundo

// Sensor de Peso HX711
#define DOUT  5                             // HX711 DATA OUT = pino 5 do Arduino 
#define CLK  4                              // HX711 SCK IN = pino 4 do Arduino 
HX711 balanca;                              // Instancia a Balança HX711
float calibration_factor = -23570;          // Fator de calibração aferido na calibraçao 
float Peso;                                 // Variável de peso em quilos

// TX e RX entre Base e Gerente
byte PacoteRX[52];                          // Pacote de 52 bytes que será recebido pelo nó sensor
byte PacoteTX[52];                          // Pacote de 52 bytes que será transmitido pelo nó sensor
int ID;                                     // Identificação do sensor

void setup() { 
Serial.begin(9600);                          // Inicia a porta serial
pinMode(pinTrig,OUTPUT);                     // Configura o pino Trig como saída
digitalWrite(pinTrig,LOW);                   // Inicia o pino em nível baixo
pinMode(pinEcho,INPUT);                      // Configura o pino Echo como entrada
balanca.begin(DOUT, CLK);                    // Inicia o módulo HX711
balanca.set_scale(calibration_factor);       // Ajusta fator de calibração
balanca.tare();                              // Zera a Balança

ID = 1;                                      // Identificação da Base
delay(100);                                  // Aguarda 100 ms
}

// Função que envia o pulso de trigger
void DisparaUltrassonico () {
  digitalWrite(pinTrig, HIGH);                // Nível alto no pino Trig
  delayMicroseconds(10);                      // Aguardar pelo menos 10 us para SR04
  digitalWrite(pinTrig, LOW);                 // Nível baixo no pino Trig 
}

// Função que calcula distancia em metros
float CalculaDistancia (float tempo_us) {
  return ((tempo_us*VelSound_mperus)/2);      // Fórmula do cálculo
}

float StrToFloat(String str){
char carray[str.length() + 1]; 
str.toCharArray(carray, sizeof(carray));
return atof(carray);
}

void loop() {

// Medida HC-SR04
DisparaUltrassonico();                        // Dispara trigger
TimeEcho = pulseIn(pinEcho,HIGH);             // Mede o tempo de duração do sinal no pino de leitura (us)
Dist_M = CalculaDistancia(TimeEcho);          // Calcula distância em metros
Dist_C = CalculaDistancia(TimeEcho*100);      // Calcula distância em centímetros

// Medida HX711
Peso = StrToFloat(String (balanca.get_units(), 2));
//Serial.println(Peso);


// Quando chega o pacote pela USB
if (Serial.available() ==52)                  // Recebe pacote que entrou pela USB
{
  
// A transmissão através do pacote é feita por bytes. Para isto deve ser convertido para um número inteiro. 
Dist_M = Dist_M*100;
Dist_C = Dist_C*100;
Peso = Peso*100;
                 
// Leitura do buffer da serial e coloca no PacoteRX[] e zera pacote de transmissão PacoteTX[]
for (int i = 0; i < 52; i++)                  // PacoteTX[#] é preenchido com zero e PacoteRX[#] recebe os bytes do buffer
{
  PacoteTX[i] = 0;                            // Zera o pacote de transmissão
  PacoteRX[i] = Serial.read();                // Faz a leitura dos bytes que estão no buffer da serial
  delay(1);
}
  
// Verifica se o pacote que chegou é para este sensor, que no caso é 1, pois foi definido no void setup ID = 1
if (PacoteRX[8] == ID){

  // Escreve no pacote as informações do ADC 0
  PacoteTX[17] = (byte) ((int)(Dist_M)/256);  // Valor inteiro no byte 17
  PacoteTX[18] = (byte) ((int)(Dist_M)%256);  // Resto da divisão no byte 18

  // Escreve no pacote as informações do ADC 1
  PacoteTX[20] = (byte) ((int)(Dist_C)/256);  // Valor inteiro no byte 20
  PacoteTX[21] = (byte) ((int)(Dist_C)%256);  // Resto da divisão no byte 21
  
  // Escreve no pacote as informações do ADC 2
  PacoteTX[23] = (byte) ((int)(Peso)/256);  // Valor inteiro no byte 23
  PacoteTX[24] = (byte) ((int)(Peso)%256);  // Resto da divisão no byte 24     
 
  // Escreve no pacote as informações do ADC 3
  //PacoteTX[26] 
  //PacoteTX[27]

  // Escreve no pacote as informações do ADC 4
  //PacoteTX[29] 
  //PacoteTX[30] 

// Transmite o pacote
for (int i = 0; i < 52; i++)
{
  Serial.write(PacoteTX[i]);
}
              
}
}     
}
