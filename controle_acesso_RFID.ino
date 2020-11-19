#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#define RST_PIN 9
#define SS_PIN 10
#define EletroIma A0
#define Tx A4
#define Rx A5
#define SerialControleBi 3 // Controla o envio e o recebimento de dados pela serial
#define transmitir HIGH // Habilita a transmissão
#define receber LOW // Habilita o recebimento
#define LedEstadia A3
bool mensagem_Inicial_Exibida = true; // verifica se a função mensagem_Inicial ja rodo
int esperaDados = 0;
SoftwareSerial RS485Serial(Rx, Tx); // (RX, TX) Cria o objeto Serial
MFRC522 mfrc522(SS_PIN, RST_PIN); // Cria objeto da classe MFRC522
LiquidCrystal lcd(8, 7, 5, 4, A2, A1); // Cria o objeto para manipulação do lcd
void setup()
{
	// ===================================================================================
	SPI.begin(); // inicialização da interface periferica serial
	mfrc522.PCD_Init(); // inicialização da placa mc522
	lcd.begin(16, 2); // SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY. EM SUMA: UMA MATRIZ DE 16 COLUNAS E 2 LINHAS
	mensagem_Inicial();
	// ===================================================================================
	pinMode(EletroIma, OUTPUT);
	digitalWrite(EletroIma, HIGH);
	pinMode ( LedEstadia, OUTPUT);
	digitalWrite(LedEstadia, LOW);
	pinMode(6, OUTPUT); // DECLARA QUE O PINO 6 É UMA SAÍDA (CONTRASTE DO LCD)
	analogWrite(6, 99); // ESCREVE O VALOR 99 NA PORTA 6 (CONTROLE DO CONTRASTE VIA PWM)
	pinMode(SerialControleBi, OUTPUT);
	pinMode(SerialControleBi, transmitir); // inicializa envio de dados da IHM para serial
	RS485Serial.begin(115600);
}

// ===================================================================================
void loop()
{
	inicio: // inicia o loop após sair so switch

	esperaDados = 0; // zera a variavel apos o loop

	if(mensagem_Inicial_Exibida == true)
	{
		mensagem_Inicial();
		mensagem_Inicial_Exibida = false;
	}

	if(!mfrc522.PICC_IsNewCardPresent())
	{
		while(!mfrc522.PICC_IsNewCardPresent())
		{
			Led_Estadia(); // Liga ou desliga o led de sinalização de colaborador no porão

		}
	}

	// Se nada for lido, não executa o restante das instruções.
	if (!mfrc522.PICC_ReadCardSerial())
	{
		return;
	}

	pinMode(SerialControleBi, receber); // inicializa recebimento da tag

	RS485Serial.println(Exibir_Tag()); // exibe a tag 

	pinMode(SerialControleBi, transmitir); // muda estado da transmissão de dados. IHM para serial

	// espera o Supervisorio responda a solicitação
	while (esperaDados != 30000)
	{
			// Libera ou não, com basse o retorno dado pelo sepervisorio
		switch (RS485Serial.read())
		{
			case 'E':
					mensagem_Entrada_Liberado();
					digitalWrite(EletroIma, LOW);
					delay(3000);
					digitalWrite(EletroIma, HIGH);
					mensagem_Inicial_Exibida = true; // zera a variavel de exibição da mensagem inicial
					Led_Estadia();
					Serial.flush(); // Limpa buffer
					goto inicio;
					break;
			case 'S':
					mensagem_Saida_Liberado();
					digitalWrite(EletroIma, LOW);
					delay(3000);
					digitalWrite(EletroIma, HIGH);
					mensagem_Inicial_Exibida = true; // zera a variavel de exibição da mensagem inicial
					Led_Estadia();
					Serial.flush();// Limpa  buffer
					goto inicio;
					break;
			case 'B':
					mensagem_Acesso_Negado();
					delay(2000);
					mensagem_Inicial_Exibida = true; // zera a variavel de exibição da mensagem inicial
					Serial.flush();// Limpa buffer
					goto inicio;
					break;
		}

	esperaDados++; // incrementa a cada iteração
	}
	Serial.flush(); // Limpa buffer
}

// ===================================================================================
// Faz a desfragmentação do valor lido pelo leitor rfid
String Exibir_Tag()
{
	String tag = "";
	// -----------------------------------------------------------------------------------
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		tag.concat(String(mfrc522.uid.uidByte[i], HEX));
	}
	return(tag);
}

// ===================================================================================
void mensagem_Inicial()
{
	lcd.clear();
	lcd.print(" APROXIME O SEU");
	lcd.setCursor(0, 1);
	lcd.print("CARTAO DO LEITOR");
}

// ===================================================================================
void mensagem_Entrada_Liberado()
{
	lcd.clear();
	lcd.print("ENTRADA LIBERADA");
}

// ===================================================================================
void mensagem_Saida_Liberado()
{
	lcd.clear();
	lcd.print("SAIDA LIBERADA");
}

// ===================================================================================
void mensagem_Acesso_Negado()
{
	lcd.clear();
	lcd.print(" ACESSO NEGADO");
	lcd.setCursor(0, 1);
	lcd.print(" PROCURE ADM's");
	delay(3000);
}

// ===================================================================================
void Led_Estadia() // Liga ou desliga sinalizador de colaborador acessando o porão
{
  switch (RS485Serial.read()) {
      case 'L':
        digitalWrite(LedEstadia, HIGH);
        break;
      case 'D':
        digitalWrite(LedEstadia, LOW);
        break;
  }
}