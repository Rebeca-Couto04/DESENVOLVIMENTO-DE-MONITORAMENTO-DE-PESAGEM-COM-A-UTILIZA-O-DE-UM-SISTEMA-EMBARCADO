// Rebeca Maria Couto Mota
// Objetivo: Coletar dado da Balança de Borracha

///////////////////////////////////// INCLUSÃO DAS BIBLIOTECAS //

 #include <SPI.h>                      // 
 #include <Wire.h>                     //
 #include <LiquidCrystal_I2C.h>        // Display e o módulo I2C
 #include <SD.h>                       // Módulo Cartão de Memória
 #include "RTClib.h"                   // Módulo RTC (Real Time Clock)


///////////////////////////////////// DEFINIÇÕES DA VARIAVEIS //
 File data_geral;

 String arquivo_geral;
 String peso_convert;

 int b_press = 0;
 int b_envio = 0;
 int b_arquivo = 0;
 int b_lider = -1;

 int erro_SD = 2;
 int pacote_ok = 0;

 RTC_DS1307 rtc;                       // Inicializa o objeto para o RTC DS1307

 LiquidCrystal_I2C lcd(0x27, 16, 2);   // Endereço 0x27 para o LCD 16x2

 #define LIDER     A0                  // PRETO
 #define RESET     A1                  // VERMELHO
 #define ENVIAR    A2                  // BRANCO
 #define B_AZU     A3                  // AZUL
 #define B_AMA     A4                  // AMARELO
 #define B_VED     A5                  // VERDE
 #define CS_PIN    53

///////////////////////////////////// LEITURA DA SERIAL //
 //#######TIPOS LOCAIS####################
  typedef enum
  {
    S0_ESPERA_INICIO,    //Espera receber o caractere de inicio de pacote;
    S1_RECEBENDO_PACOTE, //Enquanto estiver recebendo o pacote;
    S2_TRATA_PACOTE,     //Tratar o pacore recebido
    S3_ENVIA_PACOTE,     //Espera reiniciar a contagem
  }   estados;

  //#######################################
  char pacote[11] = {'0','0','0','0','0','0','0','0','0','0','0',};
  byte byteRecebido = 0;
  byte posicao = 0;
  bool pacoteCorreto = false;

  //Maquina de estados
  estados estadoAtual = S0_ESPERA_INICIO;    //cria uma variavel estadoAtual do tipo estados inicializa com o estado inicial S0
  estados estadoFuturo = estadoAtual; //cria uma variavel estadoFuturo do tipo `estados' e inicializa com o estado inicial S0

  estados funcEstadoS0(byte inByte); //Espera receber o caractere de inicio de pacote;
  estados funcEstadoS1(byte inByte); //Enquanto estiver recebendo o pacote;
  estados funcEstadoS2(); //Tratar o pacore recebido
  estados funcEstadoS3(); //Espera reiniciar a contagem
  byte inverte_byte(byte inByte); //Inverte os bits de um byte e rotaciona ele

  ////########Funcoes locais##############
  estados funcEstadoS0(byte inByte)
  {
    if(inByte == 0xFF)
    {
      pacote[0] = inByte;
      posicao = 1;
      Serial.println(pacote[0]);
      Serial.println("Iniciado");
      return(S1_RECEBENDO_PACOTE);
    }
    else
    {
    //  Serial.println("Esperando Inicio");
      return(S0_ESPERA_INICIO);    
    }
  }
  //Recebendo o pacote
  estados funcEstadoS1(byte inByte)
  {
    if(posicao > 10) //inByte == 0x02 || 
    {
      pacote[10] = inByte;
      Serial.println("Pacote completo sem tratamento");
      return(S2_TRATA_PACOTE); 
    }
    else
    {
      pacote[posicao] = inByte;
      posicao++;
    // Serial.println("Recebendo Pacote");
      return(S1_RECEBENDO_PACOTE);    
    }
  }//FIM

  //Tratando o pacote
  estados funcEstadoS2()
  {

    pacoteCorreto = true; 
    if(pacote[0] != 0xFF)
    {
      pacoteCorreto = false; 
    }
    if(pacote[11] != 0x02)
    {
      pacoteCorreto = false; 
    }
    
    if(pacote[2] == 0x02)
    {
      pacote[1] = '0';
      pacote[2] = '0';
      pacote[3] = (pacote[3]>>3) + '0';
    }
    else if(pacote[2] == 0x0A)
    {
      pacote[1] = '0';
      pacote[2] = '1';
    }
    else
    {
      pacoteCorreto = false; 
    }


    return(S3_ENVIA_PACOTE);
  }//FIM

  estados funcEstadoS3()
  {
    if(pacoteCorreto)
    {
      /*
      for(int i = 0; i < 11; i++)
      {
        Serial.print(pacote[i]);
      }
      */
      pacote_ok = 1;
    }
    else
    {
      Serial.println("Pacote com erro!");
      pacote_ok = 1;                          /////////////////////////////// ALTERAR PARA pacote_ok = 0
      /*
      for(int i = 0; i < 11; i++)
      {
        Serial.print(pacote[i]);
      }
      */
    }
    return(S0_ESPERA_INICIO);
  }

  //Inverte os bits de um byte e rotaciona o byte
  byte inverte_byte(byte inByte)
  {
    byte outByte = inByte ^ 0xFF;
    for(int i = 0; i < 7; i++)
    {
      outByte = (outByte >> 7) | (outByte << 1);
    }
    return(outByte);
  }

///////////////////////////////////// FUNÇÃO DE CHECAGEM DO RTC //
 void checagem_RTC()
 {
  if (! rtc.begin())
  {
    lcd.setCursor(0, 0);
    lcd.println("RTC INDISPONIVEL");
    Serial.flush();
    lcd.setCursor(0, 1);
    lcd.println("OLHE AS CONEXOES");
    while (1) delay(10000);
  }
  if (! rtc.isrunning()) {
    lcd.setCursor(0, 0);
    Serial.println("RTC INTERROMPIDO");
    lcd.setCursor(0, 1);
    Serial.println("AJUSTE PARAMETRO");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(10000);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 }
///////////////////////////////////// FUNÇÃO DE CHECAGEM DO CARTÃO DE MEMÓRIA //
 void checagem_cartaoSD()
 {
  lcd.setCursor(0, 1);
  lcd.print("ATIVANDO CARTAO ");
  delay(2000);

  if (!SD.begin(53)) 
  {
    lcd.setCursor(0, 1);
    lcd.println("ERRO COM CARTAO!");
    erro_SD = 1;
  }
  else 
  {
    lcd.setCursor(0, 1);
    lcd.println("   CARTAO OK!   ");
  }
 }
///////////////////////////////////// FUNÇÃO PARA ATUALIZAÇÃO NO DISPLAY A DATA E HORA ATUAIS //
 void atualizarLCD_RTC()
 {
  DateTime now = rtc.now();           // Ativação da função da biblioteca do RTC

  lcd.setCursor(0, 0);                // Posiciona o cursor do LCD onde será colocado a informação
  lcd.print(now.hour(), DEC);         // Imprime no display a hora atual

  lcd.setCursor(2, 0);
  lcd.print(":");                     //  Imprime no display o caractere 

  lcd.setCursor(3, 0);
  lcd.print(now.minute(),DEC);       // Imprime no display o minuto atual

  lcd.setCursor(6, 0);
  lcd.print(now.day(), DEC);          // Imprime no display o dia atual

  lcd.setCursor(8, 0);
  lcd.print("/");                     // Imprime no display o caractere

  lcd.setCursor(9, 0);
  lcd.print(now.month(), DEC);        // Imprime no display o mês

  lcd.setCursor(11, 0);
  lcd.print("/");                     // Imprime no display o caractere

  lcd.setCursor(12, 0);
  lcd.print(now.year(), DEC);         //Imprime no display o ano atual
 }
///////////////////////////////////// FUNÇÃO PARA ATUALIZAR VALOR DA PESAGEM NO DISPLAY //
 String atualizarLCD_PESO()
 {
  char peso[8] = {'0','0','0','0','0','0','0',' ',};
  int j = 0;

    for(int i = 1; i < 8; i++)
  {
    peso[j] = pacote[i];
    j++;
  } 

  return (String(peso));
 }
///////////////////////////////////// FUNÇÃO PARA ESCREVER DADOS NO ARQUIVO //
 void escrever_SD(String nome_arq, String material , String peso, String data, String hora)
 {
  File arquivo = SD.open(nome_arq, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (arquivo) {
    arquivo.println(material + "-" + data + "-" + hora + "-" + peso);
    // close the file:
    arquivo.close();
    lcd.setCursor(0, 1);
    lcd.print("PESAGEM ENVIADA!");
    delay(5000);
  }
  else
  {
    // if the file didn't open, print an error:
    lcd.setCursor(0, 1);
    lcd.print("ERRO COM ARQUIVO");
  }
  
 }

///////////////////////////////////// FUNÇÃO PARA ENVIAR DADOS PARA ARQUIVO //
 void enviar_dados(String peso)
 {
   DateTime envio = rtc.now();
   String data_envio;
   String hora_envio;

   data_envio = (String) envio.day() + "/" + envio.month() + "/" + envio.year();
   hora_envio = (String) envio.hour() + ":" + envio.minute();

   switch(b_press)
   {
     case 1:
       //escrever no arquivo 1
       escrever_SD(arquivo_geral, "B15" , peso, data_envio, hora_envio);
       break;
     case 2:
       //escrever no arquivo 2
       escrever_SD(arquivo_geral,"B30" , peso, data_envio, hora_envio);
       break;
     case 3:
       //escrever no arquivo 3
       escrever_SD(arquivo_geral, "RESINA" , peso, data_envio, hora_envio);
       break;
     default:
       lcd.setCursor(0, 1);
       lcd.print("PESO NAO ENVIADO");
       delay(5000);
   }
   b_envio = 0;
 }

void setup()
{
 //////////////////////////////////// INICIALIZAÇÃO DA SERIAL //
  Serial.begin(9600);
  while (!Serial)
  { ; }
  Serial1.begin(9600); // serial da balança

 //////////////////////////////////// INICIALIZAÇÃO DOS BOTÕES // 
  pinMode(B_VED, INPUT_PULLUP);
  pinMode(B_AMA, INPUT_PULLUP);
  pinMode(B_AZU, INPUT_PULLUP);
  pinMode(RESET, INPUT_PULLUP); // VERMELHO
  pinMode(LIDER, INPUT_PULLUP); // PRETO
  pinMode(ENVIAR, INPUT_PULLUP); // BRANCO

 //////////////////////////////////// INICIALIZAÇÃO DO DISPLAY //
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();                        // Limpa tudo que está escrito no display
  
 //////////////////////////////////// INICIALIZAÇÃO DO RTC //
  checagem_RTC();
  atualizarLCD_RTC();

 //////////////////////////////////// INICIALIZAÇÃO DO CARTÃO DE MEMÓRIA // 
  checagem_cartaoSD();

}
void loop()
{
 //////////////////////////////////// ATUALIZAÇÃO DE DATA E HORA NO LCD //
  DateTime s = rtc.now();
  if (s.second()==0)
  {
  atualizarLCD_RTC();
  }

 //////////////////////////////////// ACIONAMENTO DOS BOTÕES //
  if (digitalRead(LIDER) == HIGH)
  {
    b_arquivo = 1;
  }
  if (erro_SD == 0)
  {
    if (digitalRead(ENVIAR) == HIGH)
    {
      b_envio = 1;
    }
  
    if (digitalRead(B_VED) == HIGH)
    {
      b_press = 1;
    }
    else if (digitalRead(B_AMA) == HIGH)
    {
      b_press = 2;
    }
    else if (digitalRead(B_AZU) == HIGH)
    {
      b_press = 3;
    }
  }


 //////////////////////////////////// ACIONAMENTO DO BOTÃO DE CRIAÇÃO DOS ARQUIVOS//
  if (b_arquivo == 1 )
  {
    b_arquivo = 0;
    if (!SD.begin(53)) 
    {
    lcd.setCursor(0, 1);
    lcd.println("ERRO COM CARTAO!");
    erro_SD = 1;
    b_press = 0;
    }
    else 
    {
      if(erro_SD == 1)
      {
        checagem_cartaoSD();
        erro_SD = 0;
        b_lider = -1;
      }

      b_lider=-b_lider;

      if (b_lider == -1)
      {
        data_geral.close();

        do
        {
          lcd.setCursor(0, 1);
          lcd.print("RETIRE CARTAO SD");
        } while(SD.begin(53));
          {
            do
            {
              lcd.setCursor(0, 1);
              lcd.print("CARTAO RETIRADO!");
            } while(!SD.begin(53));
              {
                lcd.setCursor(0, 1);
                lcd.print(" CARTAO INSERIDO ");
                delay(2000);
                erro_SD = 1;
                b_press = 0;
              }
          }
      
      }

      if (b_lider == 1)
      {
        DateTime arquivo = rtc.now();

        arquivo_geral = (String) arquivo.day() + "-" + arquivo.month() + ".txt"; // ve se o prof sabe o pq desse bug
         
    
        lcd.setCursor(0, 1);
        lcd.print("PRONTO PARA USO ");
        erro_SD = 0;
      }
      delay(1000);
    }
  }

 //////////////////////////////////// LEITURA DO PESO //
  if(Serial1.available())
  {
    byteRecebido = Serial1.read();
    byteRecebido = inverte_byte(byteRecebido);
    switch(estadoAtual)
    {
    case S0_ESPERA_INICIO: //Espera o inicio do pacote
      estadoFuturo = funcEstadoS0(byteRecebido);
      break;
    case S1_RECEBENDO_PACOTE: //Recebe todo o pacote
      estadoFuturo = funcEstadoS1(byteRecebido);
      break;
    case S2_TRATA_PACOTE: //Conta MM:SS
      estadoFuturo = funcEstadoS2( );
      break;
    case S3_ENVIA_PACOTE: //Trava esperando reiniciar o contador
      estadoFuturo = funcEstadoS3( );
      //////////////////////////////////// ACIONAMENTO DOS BOTÕES DOS MATERIAIS // 
      if (pacote_ok == 1)
      {
        switch(b_press)
        {
        case 1:
          lcd.setCursor( 0, 1);
          lcd.print("  B015:");
          lcd.setCursor(7, 1);
          peso_convert = atualizarLCD_PESO();
          lcd.print(peso_convert);
          break;
        case 2:
          lcd.setCursor(0, 1);
          lcd.print("  B030:");
          lcd.setCursor(7, 1);
          peso_convert = atualizarLCD_PESO();
          lcd.print(peso_convert);
          break;
        case 3:
          lcd.setCursor(0, 1);
          lcd.print(" RESINA:");
          lcd.setCursor(8, 1);
          peso_convert = atualizarLCD_PESO();
          lcd.print(peso_convert);
          break;
        }
        if(b_envio == 1)
        {
          enviar_dados(peso_convert);
        }
      } 
      break;
    default:
      estadoFuturo = S0_ESPERA_INICIO;
      break;
    } 
  estadoAtual = estadoFuturo;       //atualiza o estado atual
  }

}
