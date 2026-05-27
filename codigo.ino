/*Declarando a porta de cada componente*/
#define LED_VERDE 2
#define LED_AMARELO 3
#define LED_VERMELHO 4
#define LED_AZUL 5

#define BOTAO_VERDE 8
#define BOTAO_AMARELO 9
#define BOTAO_VERMELHO 10
#define BOTAO_AZUL 11

#define BUZZER 13

/*Declarando constantes de tempo*/
#define UM_SEGUNDO 1000
#define MEIO_SEGUNDO 500

/*Declarando variáveis do jogo*/
#define INDEFINIDO -1 //quando o jogador ainda não tomou nenhuma ação

#define TAMANHO_SEQUENCIA 20 //quantidade de acertos consecutivos para vencer o jogo

int sequenciaLuzes[TAMANHO_SEQUENCIA]; //guarda a ordem em que os leds acendem
int rodada = 0;
int ledsRespondidos = 0; //quantidade de leds que o jogador acionou

/*Listando os estados do jogo*/
enum Estados {
  PRONTO_PARA_PROXIMA_RODADA,
  USUARIO_RESPONDENDO,
  JOGO_FINALIZADO_SUCESSO,
  JOGO_FINALIZADO_FALHA
};


void setup() {
  Serial.begin(9600); //para acompanhar no monitor serial
  iniciaPortas(); //configuração dos pins mode
  iniciaJogo(); //configuração inicial do jogo
}


void iniciaPortas() {
  /*Configurando os leds*/
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  /*Configurando os botões com resistor interno*/
  pinMode(BOTAO_VERDE, INPUT_PULLUP);
  pinMode(BOTAO_AMARELO, INPUT_PULLUP);
  pinMode(BOTAO_VERMELHO, INPUT_PULLUP);
  pinMode(BOTAO_AZUL, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
}

void iniciaJogo() {
  int jogo = analogRead(0); //lê a porta analógica A0
  randomSeed(jogo); //gera uma nova partida de jogo aleatória entre 0 e 1023
  //ou seja, o jogo consegue gerar até 1023 partidas diferentes

  /*Usando a função sorteiaCor para incluir uma cor de led aleatória na sequência*/
  for (int i = 0; i < TAMANHO_SEQUENCIA; i++) {
    sequenciaLuzes[i] = sorteiaCor();
  }
}

int sorteiaCor() {
  return random(LED_VERDE, LED_AZUL + 1); //sorteia um número aleatório entre 2 (porta led verde) e 5 (porta led azul)
  //soma-se 1 ao número 5, pois senão ele não estaria incluso nas opções de escolha randômica
}


/*Caracterizando os estados do jogo*/
int estadoAtual() {
  if (rodada <= TAMANHO_SEQUENCIA) { //número de rodadas não ultrapassou o limite do jogo
    if (ledsRespondidos == rodada) { //a qtd de leds acionados pelo jogador é igual ao nº de rodadas
      return PRONTO_PARA_PROXIMA_RODADA;  //vai para a próxima rodada e adiciona uma cor no fim da sequência
    } else {
      return USUARIO_RESPONDENDO; //jogador ainda não acionou todos os leds da sequência
    }
  } else if (rodada == TAMANHO_SEQUENCIA + 1) { //número de rodadas ultrapassou o limite do jogo
    return JOGO_FINALIZADO_SUCESSO; //vitória
  } else { //qualquer outra situação que não seja as anteriormente especificadas
    return JOGO_FINALIZADO_FALHA; //game over
  }
}

void loop() {
  /*Identificando qual o estado atual do jogo e chamando a respectiva função*/
  switch (estadoAtual()) {
    case PRONTO_PARA_PROXIMA_RODADA:
      Serial.println("Pronto para a proxima rodada");
      preparaNovaRodada();
      break;
    case USUARIO_RESPONDENDO:
      Serial.println("Usuario respondendo");
      processaRespostaUsuario();
      break;
    case JOGO_FINALIZADO_SUCESSO:
      Serial.println("Jogo finalizado com sucesso");
      jogoFinalizadoSucesso();
      break;
    case JOGO_FINALIZADO_FALHA:
      Serial.println("Jogo finalizado com falha");
      jogoFinalizadoFalha();
      break;
  }
  delay(MEIO_SEGUNDO);
}


/*Função do estado de jogo: PRONTO_PARA_PROXIMA_RODADA*/
void preparaNovaRodada() {
  rodada++; //adiciona 1 no número de rodadas
  Serial.println(rodada);
  ledsRespondidos = 0; //zera a quantidade de leds acionados (pois uma nova rodada se iniciou)
  if (rodada <= TAMANHO_SEQUENCIA) { //número de rodadas não ultrapassou o limite do jogo
    tocaLedsRodada();
  }
}

void tocaLedsRodada() {
  for (int i = 0; i < rodada; i++) {
    piscaLed(sequenciaLuzes[i]);  //acende os leds conforme sequência
  }
}

int piscaLed(int portaLed) {
  verificaSomDoLed(portaLed); //chama a função que toca uma frequência de acordo com a cor do led

  digitalWrite(portaLed, HIGH); //acende o led
  delay(500);
  digitalWrite(portaLed, LOW); //apaga o led
  delay(300);

  return portaLed; //retorna o valor da variável portaLed para ser usado em outras funções
}


/*Função do estado de jogo: USUARIO_RESPONDENDO*/
void processaRespostaUsuario() {
  int resposta = checaRespostaJogador();

  if (resposta == INDEFINIDO) { //jogador ainda não tomou uma ação
    return;
  }

  if (resposta == sequenciaLuzes[ledsRespondidos]) { //jogador acertou a sequência
    ledsRespondidos++; //acrescenta um no número de leds acionados
  } else { //jogador errou a sequência
    Serial.println("Sequencia errada");
    rodada = TAMANHO_SEQUENCIA + 2; //soma-se 2 para cair no estado de jogo game over
    //se somasse 1 cairia no estado de jogo vitória (vide função estadoAtual)
  }
}

int checaRespostaJogador() {
  //acende o led correspondente ao botão que foi acionado
  if (digitalRead(BOTAO_VERDE) == LOW) {
    return piscaLed(LED_VERDE);
  }
  if (digitalRead(BOTAO_AMARELO) == LOW) {
    return piscaLed(LED_AMARELO);
  }
  if (digitalRead(BOTAO_VERMELHO) == LOW) {
    return piscaLed(LED_VERMELHO);
  }
  if (digitalRead(BOTAO_AZUL) == LOW) {
    return piscaLed(LED_AZUL);
  }

  return INDEFINIDO; //jogador não acionou nenhum botão
}


/*Função do estado de jogo: JOGO_FINALIZADO_SUCESSO*/
void jogoFinalizadoSucesso() {
  //pisca um led de cada vez
  piscaLed(LED_VERDE);
  piscaLed(LED_AMARELO);
  piscaLed(LED_VERMELHO);
  piscaLed(LED_AZUL);
  delay(MEIO_SEGUNDO);
}


/*Função do estado de jogo: JOGO_FINALIZADO_FALHA*/
void jogoFinalizadoFalha() { //toca uma frequência e pisca todos os leds de uma vez
  tocaSom(297);
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AMARELO, HIGH);
  digitalWrite(LED_VERMELHO, HIGH);
  digitalWrite(LED_AZUL, HIGH);
  delay(UM_SEGUNDO);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_AZUL, LOW);
  delay(MEIO_SEGUNDO);
}

/*Criando função para acionar buzzer passando apenas a frequência como parâmetro*/
void tocaSom(int frequencia) {
  tone(BUZZER, frequencia, 100);
}

void verificaSomDoLed(int portaLed) {
  //determina uma frequência diferente para cada cor de led
  switch (portaLed) {
    case LED_VERDE:
      tocaSom(495);
      break;
    case LED_AMARELO:
      tocaSom(440);
      break;
    case LED_VERMELHO:
      tocaSom(330);
      break;
    case LED_AZUL:
      tocaSom(352);
      break;
  }
}
