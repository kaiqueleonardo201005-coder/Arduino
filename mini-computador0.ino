#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <math.h>
#include <esp_wifi.h>
#include <HijelHID_BLEMouse.h>
#define CODIGO_VERSAO "2.0"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

const int PIN_JOY_X = 34;
const int PIN_JOY_Y = 35;
const int PIN_JOY_SW = 32;
const int PIN_LDR = 36;



const int PIN_BATERIA = 39;
const float BATERIA_R1 = 100000.0f;
const float BATERIA_R2 = 100000.0f;
const float BATERIA_VMIN = 3.20f;
const float BATERIA_VMAX = 4.20f;

int itemSelecionado = 0;
bool emSubMenu = false;

const int TOTAL_ITENS = 8;

const char* menuItens[] = {
  "1. Tempo & Relogio",
  "2. Cotacao VGIA11",
  "3. Robo Inteligente",
  "4. Configurar Wi-Fi",
  "5. Criar Wi-Fi",
  "6. Mouse Bluetooth",
  "7. Calculadora",
  "8. Sistema / Versao"
};

unsigned long tempoUltimaAcaoJoy = 0;
bool joyEmMovimento = false;


const unsigned long TEMPO_IDLE_HACK = 30000;
unsigned long ultimaInteracaoComputador = 0;
bool modoHackIdle = false;
unsigned long inicioHackIdle = 0;

const char* BRAPI_TOKEN = "";

HijelBLEMouse mouseBLE("Mini Computer Mouse", "ESP32 Mini Computer", 100, 3, false);
unsigned long ultimaAtualizacaoBateriaBLE = 0;
unsigned long inicioPressMouse = 0;
bool mouseBotaoPressionado = false;

const int PAUSA_INICIAL_REPETICAO = 250;
const int VELOCIDADE_DESLIZE = 100;

unsigned long ultimoDebounceBotao = 0;

const int JOY_CIMA = 3500;
const int JOY_BAIXO = 600;
const int JOY_DIREITA = 3500;
const int JOY_ESQUERDA = 600;

String wifiSSID = "";
String wifiPASS = "";

const char* ntpServer = "a.st1.ntp.br";
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

bool ntpConfigurado = false;

unsigned long ultimoMovimentoOlhos = 0;
unsigned long ultimoPiscar = 0;

unsigned long tempoSusto = 0;

int olharX = 0;
int olharY = 0;

int alvoOlharX = 0;
int alvoOlharY = 0;

bool olhosFechados = false;

int expressaoAtual = 0;

bool roboDormindo = false;
bool acordandoRobo = false;

unsigned long inicioAcordar = 0;
unsigned long tempoParaAcordar = 0;

int etapaAcordar = 0;

unsigned long inicioAnimacao = 0;
int animacaoRobo = 0;

unsigned long proximoMovimentoRobo = 0;
unsigned long proximoPiscarRobo = 0;

bool joystickRoboMexendo = false;
unsigned long inicioEscuro = 0;

const unsigned long TEMPO_ESCURECIDO_PARA_DORMIR = 15000;
const bool LDR_INVERTIDO = false;
const int LDR_LIMIAR_LUZ_DORMIR = 20;
const int LDR_LIMIAR_LUZ_ACORDAR = 25;
bool ambienteEscuro = false;
unsigned long ultimoDebugRobo = 0;
unsigned long ultimoLDRValido = 0;
int ultimaLuzRobo = -1;
int comportamentoRobo = 0;
unsigned long inicioComportamentoRobo = 0;

float tempOnline = 0.0;
int codigoClima = -1;
String descricaoClima = "Buscando...";

unsigned long ultimaAtualizacaoClima = 0;

bool mostrarDetalhesClima = false;

float cotacaoVGIA11 = 0.00;

unsigned long ultimaAtualizacaoCotacao = 0;
unsigned long ultimaTentativaCotacao = 0;

const int MAX_PONTOS_GRAFICO = 78;

float aberturaVGIA11[MAX_PONTOS_GRAFICO];
float maximaVGIA11[MAX_PONTOS_GRAFICO];
float minimaVGIA11[MAX_PONTOS_GRAFICO];
float historicoVGIA11[MAX_PONTOS_GRAFICO];
time_t tempoHistoricoVGIA11[MAX_PONTOS_GRAFICO];

int quantidadeHistorico = 0;

unsigned long ultimaAtualizacaoHistorico = 0;
unsigned long ultimaTentativaHistoricoVGIA11 = 0;
volatile bool atualizandoVGIA11 = false;

int estadoWifiFlow = 0;
int totalRedesEncontradas = 0;
int redeSelecionada = 0;
int offsetListaWifi = 0;

int modoTeclado = 0;

int teclaX = 0;
int teclaY = 0;

String tempPASS = "";

const char tecladoMin[4][10] = {
  {'1','2','3','4','5','6','7','8','9','0'},
  {'q','w','e','r','t','y','u','i','o','p'},
  {'a','s','d','f','g','h','j','k','l','-'},
  {'z','x','c','v','b','n','m','>','<','O'}
};

const char tecladoNum[4][10] = {
  {'1','2','3','4','5','6','7','8','9','0'},
  {'A','B','C','D','E','F','G','H','I','J'},
  {'K','L','M','N','O','P','Q','R','S','T'},
  {'U','V','W','X','Y','Z','_','>','<','O'}
};

const char tecladoSimbol[4][10] = {
  {'!','@','#','$','%','^','&','*','(',')'},
  {'-','_','=','+','[',']','{','}',';',':'},
  {'\'','"','\\','|',',','.','/','?','~','`'},
  {'<','>',' ',' ',' ',' ',' ','>','<','O'}
};

String apSSID = "ESP32-WIFI";
String apPASS = "";

bool apAtivo = false;
bool servidorIniciado = false;

int estadoAPFlow = 0;
int apMenuSelecionado = 0;
int campoAP = 0;

String tempAPSSID = "";
String tempAPPASS = "";

const int MAX_USUARIOS_AP = 8;

struct UsuarioAP {
  bool usado;
  IPAddress ip;
  String nome;
  String senha;
  unsigned long ultimoAcesso;
};


UsuarioAP usuariosAP[MAX_USUARIOS_AP];

String calculadoraDisplay = "0";
double calculadoraValor = 0.0;
char calculadoraOperacao = 0;
bool calculadoraNovoNumero = true;
bool calculadoraTemValor = false;
bool calculadoraBloquearBotao = true;
int calculadoraX = 0;
int calculadoraY = 0;
unsigned long calculadoraUltimoMovimento = 0;
unsigned long calculadoraInicioPressao = 0;
bool calculadoraBotaoPressionado = false;
bool calculadoraLongPressExecutado = false;

void iniciarWiFiSemTravar();
void buscarCotacaoVGIA11();
void buscarHistoricoVGIA11();
void buscarTemperaturaELocalizacao();
void processarJoystick();
void desenharMenu();
void telaTempoERelogio();
void telaClimaDetalhado();
void telaVGIA11();
void telaRoboInteligente();
void telaFluxoWiFi();
void telaCriarWiFi();
void telaMouseBluetooth();
void telaSistema();
void telaCalculadora();
void processarCalculadoraJoystick();
void escanearRedesWiFi();
void atualizarRobo();
void iniciarAcordarRobo();
void desenharOlhoNormal(int x, int y, int deslocX, int deslocY);
void desenharRostoExpressao();
void iniciarAccessPoint();
void iniciarServidorWeb();
void atualizarUsuariosAP();
void desenharIconeClima(int codigo, int cx, int cy);
String obterDescricaoClima(int codigo);
int lerJoystickAnalogico(int pino);
int lerLDR();
float lerTensaoBateria();
int lerPorcentagemBateria();
void atualizarIdleHack();
void tarefaAtualizarVGIA11(void* parametro);

void setup() {
  Serial.begin(115200);

  pinMode(PIN_JOY_SW, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BATERIA, INPUT);
  Wire.begin(21, 22);

  mouseBLE.begin();
  ultimaInteracaoComputador = millis();

  ultimaInteracaoComputador = millis();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      for (;;) {}
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println("MINI COMPUTER");
  display.setCursor(35, 38);
  display.println("ESP32");
  display.display();

  delay(1000);

  preferences.begin("wifi_config", false);

  wifiSSID = preferences.getString("ssid", "");
  wifiPASS = preferences.getString("pass", "");

  apSSID = preferences.getString("apssid", "ESP32-WIFI");
  apPASS = preferences.getString("appass", "");

  preferences.end();

  iniciarWiFiSemTravar();

  randomSeed(analogRead(39));

  roboDormindo = false;
  acordandoRobo = false;
  expressaoAtual = 0;
  ultimoPiscar = millis();

  proximoMovimentoRobo = millis() + random(1500, 3500);
  proximoPiscarRobo = millis() + random(2500, 5500);



  ambienteEscuro = false;
  inicioEscuro = 0;
  joystickRoboMexendo = false;

  for (int i = 0; i < MAX_PONTOS_GRAFICO; i++) {
    aberturaVGIA11[i] = 0;
    maximaVGIA11[i] = 0;
    minimaVGIA11[i] = 0;
    historicoVGIA11[i] = 0;
    tempoHistoricoVGIA11[i] = 0;
  }

  for (int i = 0; i < MAX_USUARIOS_AP; i++) {
    usuariosAP[i].usado = false;
    usuariosAP[i].nome = "";
    usuariosAP[i].senha = "";
    usuariosAP[i].ultimoAcesso = 0;
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !ntpConfigurado) {
    configTime(
      gmtOffset_sec,
      daylightOffset_sec,
      ntpServer
    );

    ntpConfigurado = true;
  }

  if (apAtivo) {
    dnsServer.processNextRequest();
    server.handleClient();
  }

  atualizarUsuariosAP();

  atualizarIdleHack();
  processarJoystick();

  if (modoHackIdle) {
  } else if (!emSubMenu) {
    desenharMenu();
  } else {
    switch (itemSelecionado) {
      case 0:
        if (WiFi.status() == WL_CONNECTED &&
            (tempOnline == 0.0f ||
             millis() - ultimaAtualizacaoClima > 600000)) {
          buscarTemperaturaELocalizacao();
        }
        if (mostrarDetalhesClima) {
          telaClimaDetalhado();
        } else {
          telaTempoERelogio();
        }
        break;

      case 1:
        if (WiFi.status() == WL_CONNECTED && !atualizandoVGIA11) {
          unsigned long agoraVGIA11 = millis();
          bool precisaCotacao =
            cotacaoVGIA11 <= 0.0f ||
            agoraVGIA11 - ultimaAtualizacaoCotacao > 120000;
          bool precisaHistorico =
            quantidadeHistorico == 0 ||
            agoraVGIA11 - ultimaAtualizacaoHistorico > 600000;
          bool podeTentarHistorico =
            quantidadeHistorico > 0 ||
            agoraVGIA11 - ultimaTentativaHistoricoVGIA11 > 30000;

          if ((precisaCotacao || (precisaHistorico && podeTentarHistorico))) {
            atualizandoVGIA11 = true;
            ultimaTentativaHistoricoVGIA11 = agoraVGIA11;
            xTaskCreate(
              tarefaAtualizarVGIA11,
              "VGIA11",
              12288,
              NULL,
              1,
              NULL
            );
          }
        }
        telaVGIA11();
        break;

      case 2:
        telaRoboInteligente();
        break;

      case 3:
        telaFluxoWiFi();
        break;

      case 4:
        telaCriarWiFi();
        break;

      case 5:
        telaMouseBluetooth();
        break;
      case 6:
        telaCalculadora();
        break;

      case 7:
        telaSistema();
        break;
    }
  }

  delay(10);
}

int lerJoystickAnalogico(int pino) {
  long soma = 0;

  for (int i = 0; i < 8; i++) {
    soma += analogRead(pino);
  }

  return soma / 8;
}

int lerLDR() {
  long soma = 0;

  for (int i = 0; i < 8; i++) {
    soma += analogRead(PIN_LDR);
  }

  return soma / 8;
}

float lerTensaoBateria() {
  uint32_t somaMv = 0;
  for (int i = 0; i < 8; i++) {
    somaMv += analogReadMilliVolts(PIN_BATERIA);
  }

  float tensaoADC = (somaMv / 8.0f) / 1000.0f;
  return tensaoADC * ((BATERIA_R1 + BATERIA_R2) / BATERIA_R2);
}

int lerPorcentagemBateria() {
  float v = lerTensaoBateria();



  int pct;
  if (v >= 4.20f) pct = 100;
  else if (v >= 4.10f) pct = 90 + (int)((v - 4.10f) * 100.0f);
  else if (v >= 4.00f) pct = 80 + (int)((v - 4.00f) * 100.0f);
  else if (v >= 3.90f) pct = 65 + (int)((v - 3.90f) * 150.0f);
  else if (v >= 3.80f) pct = 50 + (int)((v - 3.80f) * 150.0f);
  else if (v >= 3.70f) pct = 35 + (int)((v - 3.70f) * 150.0f);
  else if (v >= 3.60f) pct = 20 + (int)((v - 3.60f) * 150.0f);
  else if (v >= 3.50f) pct = 10 + (int)((v - 3.50f) * 100.0f);
  else if (v >= BATERIA_VMIN) pct = (int)((v - BATERIA_VMIN) * 100.0f / (3.50f - BATERIA_VMIN));
  else pct = 0;

  return constrain(pct, 0, 100);
}

void desenharAnimacaoIdle(int animacao, unsigned long tempo) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  int fase = tempo / 100;

  if (animacao == 0) {
    display.setCursor(0, 0);
    display.println("MINI COMPUTER");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("SYSTEM IDLE");
    display.setCursor(0, 32);
    display.print(">");
    int barras = (fase % 10);
    for (int i = 0; i < barras; i++) {
      display.print(".");
    }
    display.setCursor(0, 48);
    display.print("AGUARDANDO");

  } else if (animacao == 1) {
    display.setCursor(0, 0);
    display.println("NETWORK");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("SCANNING");
    for (int i = 0; i < 8; i++) {
      int altura = 5 + ((fase + i * 3) % 20);
      display.fillRect(4 + i * 15, 58 - altura, 10, altura, SSD1306_WHITE);
    }

  } else if (animacao == 2) {
    display.setCursor(0, 0);
    display.println("PROCESS");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("RUNNING");
    int pos = fase % 120;
    display.fillRect(4 + pos, 40, 8, 8, SSD1306_WHITE);
    display.drawRect(4, 39, 120, 10, SSD1306_WHITE);

  } else if (animacao == 3) {
    display.setCursor(0, 0);
    display.println("SIGNAL");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    int deslocamento = fase % 16;
    for (int x = 0; x < 128; x += 8) {
      int y = 36 + ((x + deslocamento) % 16);
      display.drawLine(x, 50, x + 4, y, SSD1306_WHITE);
      display.drawLine(x + 4, y, x + 8, 50, SSD1306_WHITE);
    }
    display.setCursor(0, 55);
    display.println("ANALYSIS");

  } else if (animacao == 4) {
    display.setCursor(0, 0);
    display.println("ESP32");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("STANDBY");
    int raio = 12 + ((fase / 3) % 12);
    display.drawCircle(64, 42, raio, SSD1306_WHITE);
    display.fillCircle(64, 42, 3, SSD1306_WHITE);

  } else if (animacao == 5) {
    display.setCursor(0, 0);
    display.println("WARNING");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("VIRUS DETECTED");

    int pulso = (fase / 3) % 3;
    int cx = 64;
    int cy = 42;

    display.drawCircle(cx, cy, 14 + pulso, SSD1306_WHITE);
    display.drawCircle(cx, cy, 10, SSD1306_WHITE);

    for (int i = 0; i < 8; i++) {
      float angulo = i * 0.785398f;
      int x1 = cx + (int)(14 * cos(angulo));
      int y1 = cy + (int)(14 * sin(angulo));
      int x2 = cx + (int)(20 * cos(angulo));
      int y2 = cy + (int)(20 * sin(angulo));
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }

    display.fillCircle(cx - 4, cy - 3, 2, SSD1306_WHITE);
    display.fillCircle(cx + 4, cy - 3, 2, SSD1306_WHITE);
    display.drawLine(cx - 5, cy + 5, cx, cy + 7, SSD1306_WHITE);
    display.drawLine(cx, cy + 7, cx + 5, cy + 5, SSD1306_WHITE);

  } else if (animacao == 6) {
    display.setCursor(0, 0);
    display.println("DANGER");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("SHARK DETECTED");

    int pos = 15 + (fase % 100);
    int y = 43;

    display.fillTriangle(pos - 20, y, pos + 12, y - 10, pos + 12, y + 10, SSD1306_WHITE);
    display.fillTriangle(pos + 2, y - 5, pos + 15, y - 15, pos + 15, y - 2, SSD1306_WHITE);
    display.fillTriangle(pos - 4, y - 8, pos + 3, y - 17, pos + 8, y - 7, SSD1306_WHITE);

    display.fillCircle(pos + 5, y - 3, 2, SSD1306_BLACK);
    display.drawLine(pos - 13, y + 5, pos + 2, y + 2, SSD1306_BLACK);

    for (int i = 0; i < 5; i++) {
      int bx = (pos - 30 - i * 16 + 150) % 150 - 10;
      display.drawLine(bx, 58, bx + 8, 58, SSD1306_WHITE);
    }

  } else if (animacao == 7) {
    display.setCursor(0, 0);
    display.println("SYSTEM");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 18);
    display.println("010101010101");

    const char* binario[] = {
      "101101001011",
      "010010110101",
      "110100101010",
      "001011010110",
      "101010011001"
    };

    for (int linha = 0; linha < 5; linha++) {
      int y = 29 + linha * 7;
      int desloc = (fase / 2 + linha * 5) % 16;
      display.setCursor(-desloc, y);
      display.print(binario[linha]);
      display.print("  ");
      display.print(binario[(linha + 2) % 5]);
      display.print("  10101010");
    }

  } else if (animacao == 8) {
    display.setCursor(0, 0);
    display.println("STOCK MARKET");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.println("IMAGINARY CORP +");

    int pontos[9][2] = {
      {4, 53}, {18, 50}, {30, 52}, {42, 44},
      {55, 46}, {68, 37}, {82, 39}, {96, 29}, {118, 22}
    };

    int totalPontos = 2 + (fase / 25);
    if (totalPontos > 9) totalPontos = 9;

    for (int i = 0; i < totalPontos - 1; i++) {
      display.drawLine(
        pontos[i][0], pontos[i][1],
        pontos[i + 1][0], pontos[i + 1][1],
        SSD1306_WHITE
      );
    }

    int ultimo = totalPontos - 1;
    display.fillCircle(pontos[ultimo][0], pontos[ultimo][1], 2, SSD1306_WHITE);

    if (tempo > 26000) {
      display.setCursor(103, 14);
      display.print("UP");
      display.drawCircle(112, 49, 9, SSD1306_WHITE);
      display.fillCircle(109, 46, 1, SSD1306_WHITE);
      display.fillCircle(115, 46, 1, SSD1306_WHITE);
      display.drawLine(108, 51, 112, 54, SSD1306_WHITE);
      display.drawLine(112, 54, 116, 51, SSD1306_WHITE);
    }

  } else {
    display.setCursor(0, 0);
    display.println("STOCK MARKET");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.println("IMAGINARY CORP -");

    int pontos[9][2] = {
      {4, 25}, {18, 29}, {30, 27}, {42, 35},
      {55, 33}, {68, 43}, {82, 41}, {96, 51}, {118, 57}
    };

    int totalPontos = 2 + (fase / 25);
    if (totalPontos > 9) totalPontos = 9;

    for (int i = 0; i < totalPontos - 1; i++) {
      display.drawLine(
        pontos[i][0], pontos[i][1],
        pontos[i + 1][0], pontos[i + 1][1],
        SSD1306_WHITE
      );
    }

    int ultimo = totalPontos - 1;
    display.fillCircle(pontos[ultimo][0], pontos[ultimo][1], 2, SSD1306_WHITE);

    if (tempo > 26000) {
      display.setCursor(103, 14);
      display.print("DOWN");
      display.drawCircle(112, 42, 9, SSD1306_WHITE);
      display.fillCircle(109, 39, 1, SSD1306_WHITE);
      display.fillCircle(115, 39, 1, SSD1306_WHITE);
      display.drawLine(108, 49, 112, 46, SSD1306_WHITE);
      display.drawLine(112, 46, 116, 49, SSD1306_WHITE);
    }
  }

  display.display();
}
void atualizarIdleHack() {
  if (emSubMenu && itemSelecionado == 2) {
    modoHackIdle = false;
    return;
  }

  unsigned long agora = millis();

  if (!modoHackIdle && agora - ultimaInteracaoComputador >= TEMPO_IDLE_HACK) {
    modoHackIdle = true;
    inicioHackIdle = agora;
  }

  if (modoHackIdle) {
    unsigned long tempoIdle = agora - inicioHackIdle;

    if (tempoIdle >= 300000UL) {
      inicioHackIdle = agora;
      tempoIdle = 0;
    }

    int animacao = tempoIdle / 30000UL;
    desenharAnimacaoIdle(animacao, tempoIdle % 30000UL);
  }
}

void processarJoystick() {
  int valorX = lerJoystickAnalogico(PIN_JOY_X);
  int valorY = lerJoystickAnalogico(PIN_JOY_Y);
  int btnClique = digitalRead(PIN_JOY_SW);

  bool movCima = valorX > JOY_CIMA;
  bool movBaixo = valorX < JOY_BAIXO;
  bool movDireita = valorY > JOY_DIREITA;
  bool movEsquerda = valorY < JOY_ESQUERDA;

  bool algumDirecionamento =
    movCima || movBaixo || movDireita || movEsquerda;

  if (!algumDirecionamento) {
    joyEmMovimento = false;
  }


  if (algumDirecionamento || btnClique == LOW) {
    ultimaInteracaoComputador = millis();
    modoHackIdle = false;
  }

  if (emSubMenu && itemSelecionado == 5) {
    if (mouseBLE.isPaired()) {
      int mx = 0;
      int my = 0;
      if (movEsquerda) mx = -8;
      else if (movDireita) mx = 8;
      if (movCima) my = -8;
      else if (movBaixo) my = 8;
      if (mx != 0 || my != 0) mouseBLE.moveTo(mx, my);
    }

    if (btnClique == LOW) {
      if (!mouseBotaoPressionado) {
        mouseBotaoPressionado = true;
        inicioPressMouse = millis();
      }
    } else if (mouseBotaoPressionado) {
      unsigned long duracaoPress = millis() - inicioPressMouse;
      mouseBotaoPressionado = false;

      if (duracaoPress >= 900) {
        emSubMenu = false;
        joyEmMovimento = false;
      } else if (mouseBLE.isPaired() && duracaoPress >= 40 && millis() - ultimoDebounceBotao > 200) {
        ultimoDebounceBotao = millis();
        mouseBLE.click(MouseButton::Left);
      }
    }
    return;
  }

  if (emSubMenu && itemSelecionado == 4) {
    return;
  }

  if (emSubMenu && itemSelecionado == 6) {
    processarCalculadoraJoystick();
    return;
  }

  if (!emSubMenu) {
    if (movCima || movBaixo) {
      unsigned long tempoAtual = millis();
      int intervaloRequerido = joyEmMovimento ? VELOCIDADE_DESLIZE : PAUSA_INICIAL_REPETICAO;

      if (!joyEmMovimento || tempoAtual - tempoUltimaAcaoJoy >= intervaloRequerido) {
        if (movCima) {
          itemSelecionado = (itemSelecionado - 1 + TOTAL_ITENS) % TOTAL_ITENS;
        } else if (movBaixo) {
          itemSelecionado = (itemSelecionado + 1) % TOTAL_ITENS;
        }
        tempoUltimaAcaoJoy = tempoAtual;
        joyEmMovimento = true;
      }
    }

    if (btnClique == LOW && millis() - ultimoDebounceBotao > 200) {
      ultimoDebounceBotao = millis();
      emSubMenu = true;
      joyEmMovimento = false;

      if (itemSelecionado == 0) {
        mostrarDetalhesClima = false;
      }

      if (itemSelecionado == 3) {
        estadoWifiFlow = 0;
        escanearRedesWiFi();
      }

      if (itemSelecionado == 4) {
        estadoAPFlow = 0;
        apMenuSelecionado = 0;
        campoAP = 0;
        tempAPSSID = apSSID;
        tempAPPASS = apPASS;
      }

      if (itemSelecionado == 6) {
        calculadoraDisplay = "0";
        calculadoraValor = 0.0;
        calculadoraOperacao = 0;
        calculadoraNovoNumero = true;
        calculadoraTemValor = false;
        calculadoraBloquearBotao = true;
        calculadoraX = 0;
        calculadoraY = 0;
        calculadoraUltimoMovimento = millis();
      }
    }
  } else {
    if (itemSelecionado == 0 && btnClique == LOW && millis() - ultimoDebounceBotao > 300) {
      ultimoDebounceBotao = millis();
      if (!mostrarDetalhesClima) {
        mostrarDetalhesClima = true;
      } else {
        mostrarDetalhesClima = false;
        emSubMenu = false;
      }
      joyEmMovimento = false;
    } else if (itemSelecionado == 2 && btnClique == LOW && millis() - ultimoDebounceBotao > 300) {
      ultimoDebounceBotao = millis();
      tempoSusto = millis();
      expressaoAtual = 5;
    } else if (
      btnClique == LOW &&
      itemSelecionado != 3 &&
      itemSelecionado != 0 &&
      itemSelecionado != 4 &&
      itemSelecionado != 5 &&
      millis() - ultimoDebounceBotao > 200
    ) {
      ultimoDebounceBotao = millis();
      emSubMenu = false;
      joyEmMovimento = false;
    }
  }
}

void desenharMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("MINI COMPUTER");
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);



  const int linhasVisiveis = 5;
  const int yInicial = 12;
  const int alturaLinha = 10;

  int inicio = itemSelecionado - (linhasVisiveis - 1);
  if (inicio < 0) inicio = 0;
  int maxInicio = TOTAL_ITENS - linhasVisiveis;
  if (maxInicio < 0) maxInicio = 0;
  if (inicio > maxInicio) inicio = maxInicio;

  for (int linha = 0; linha < linhasVisiveis; linha++) {
    int i = inicio + linha;
    if (i >= TOTAL_ITENS) break;

    int y = yInicial + linha * alturaLinha;
    if (i == itemSelecionado) {
      display.fillRect(0, y - 1, 128, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(1, y);
      display.print("> ");
      display.println(menuItens[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(1, y);
      display.print("  ");
      display.println(menuItens[i]);
    }
  }


  int bateriaPct = lerPorcentagemBateria();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(78, 0);
  display.print("BAT ");
  display.print(bateriaPct);
  display.print("%");

  display.display();
}

void iniciarWiFiSemTravar() {
  if (wifiSSID.length() > 0) {
    WiFi.mode(WIFI_STA);

    WiFi.setAutoReconnect(true);

    WiFi.begin(
      wifiSSID.c_str(),
      wifiPASS.c_str()
    );
  }
}

void buscarCotacaoVGIA11() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[VGIA11] Sem Wi-Fi.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(3000);

  float precoEncontrado = 0.0f;

  String urlBrapi = "https://brapi.dev/api/quote/VGIA11";
  if (http.begin(client, urlBrapi)) {
    http.setUserAgent("ESP32 Mini Computer/1.9");
    if (strlen(BRAPI_TOKEN) > 0) {
      http.addHeader("Authorization", String("Bearer ") + BRAPI_TOKEN);
    }
    int code = http.GET();

    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(8192);

      if (!deserializeJson(doc, payload)) {
        precoEncontrado = doc["results"][0]["regularMarketPrice"] | 0.0f;

        if (precoEncontrado <= 0) {
          precoEncontrado = doc["results"][0]["data"]["regularMarketPrice"] | 0.0f;
        }
      }
    }

    http.end();
  }

  if (precoEncontrado <= 0) {
    String yahooURL =
      "https://query1.finance.yahoo.com/v8/finance/chart/"
      "VGIA11.SA?range=1d&interval=1d";

    if (http.begin(client, yahooURL)) {
      http.setUserAgent("Mozilla/5.0 ESP32 Mini Computer/1.9");
      int codeYahoo = http.GET();

      if (codeYahoo == HTTP_CODE_OK) {
        String payloadYahoo = http.getString();
        DynamicJsonDocument docY(8192);

        if (!deserializeJson(docY, payloadYahoo)) {
          precoEncontrado =
            docY["chart"]["result"][0]["meta"]["regularMarketPrice"] | 0.0f;

          if (precoEncontrado <= 0) {
            JsonArray closes =
              docY["chart"]["result"][0]["indicators"]["quote"][0]["close"];

            if (!closes.isNull()) {
              for (int i = (int)closes.size() - 1; i >= 0; i--) {
                if (closes[i].isNull()) continue;
                float p = closes[i] | 0.0f;
                if (p > 0) {
                  precoEncontrado = p;
                  break;
                }
              }
            }
          }
        }
      }

      http.end();
    }
  }

  if (precoEncontrado > 0) {
    cotacaoVGIA11 = precoEncontrado;
    ultimaAtualizacaoCotacao = millis();

    Serial.print("[VGIA11] PRECO: R$ ");
    Serial.println(cotacaoVGIA11, 2);
  } else {
    Serial.println("[VGIA11] Nao foi possivel obter o preco.");
  }
}

void tarefaAtualizarVGIA11(void* parametro) {
  buscarCotacaoVGIA11();
  buscarHistoricoVGIA11();
  atualizandoVGIA11 = false;
  vTaskDelete(NULL);
}

void buscarHistoricoVGIA11() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[VGIA11] Sem Wi-Fi para buscar historico.");
    return;
  }

  float novoAbertura[MAX_PONTOS_GRAFICO];
  float novoMaxima[MAX_PONTOS_GRAFICO];
  float novoMinima[MAX_PONTOS_GRAFICO];
  float novoFechamento[MAX_PONTOS_GRAFICO];
  time_t novoTempo[MAX_PONTOS_GRAFICO];
  int novaQuantidade = 0;
  bool carregou = false;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(5000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.println("[VGIA11] Tentando historico intraday...");

  String urlsBrapi[] = {
    "https://brapi.dev/api/quote/VGIA11?range=1d&interval=5m",
    "https://brapi.dev/api/quote/VGIA11?range=5d&interval=5m"
  };

  for (int tentativa = 0; tentativa < 2 && !carregou; tentativa++) {
    novaQuantidade = 0;

    if (http.begin(client, urlsBrapi[tentativa])) {
      int code = http.GET();

      if (code == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(60000);
        DeserializationError erro = deserializeJson(doc, payload);

        if (!erro) {
          JsonArray resultados = doc["results"].as<JsonArray>();

          if (!resultados.isNull() && resultados.size() > 0) {
            JsonArray historico = resultados[0]["historicalDataPrice"].as<JsonArray>();

            for (JsonObject candle : historico) {
              float fechamento = candle["close"] | 0.0f;
              float abertura = candle["open"] | fechamento;
              float maxima = candle["high"] | fechamento;
              float minima = candle["low"] | fechamento;
              time_t tempo = candle["date"] | (time_t)0;

              if (fechamento > 0 && maxima > 0 && minima > 0 && novaQuantidade < MAX_PONTOS_GRAFICO) {
                novoAbertura[novaQuantidade] = abertura;
                novoMaxima[novaQuantidade] = maxima;
                novoMinima[novaQuantidade] = minima;
                novoFechamento[novaQuantidade] = fechamento;
                novoTempo[novaQuantidade] = tempo;
                novaQuantidade++;
              }
            }
          }
        }
      }

      http.end();
    }

    if (novaQuantidade > 1) {
      carregou = true;
      Serial.print("[VGIA11] Intraday carregado: ");
      Serial.print(novaQuantidade);
      Serial.println(" candles.");
    }
  }

  if (!carregou) {
    Serial.println("[VGIA11] Intraday sem dados. Tentando historico diario...");

    String urlsDiarias[] = {
      "https://brapi.dev/api/quote/VGIA11?range=1mo&interval=1d",
      "https://query1.finance.yahoo.com/v8/finance/chart/VGIA11.SA?range=1mo&interval=1d&events=history"
    };

    for (int tentativa = 0; tentativa < 2 && !carregou; tentativa++) {
      novaQuantidade = 0;

      if (http.begin(client, urlsDiarias[tentativa])) {
        int code = http.GET();

        if (code == HTTP_CODE_OK) {
          String payload = http.getString();
          DynamicJsonDocument doc(60000);
          DeserializationError erro = deserializeJson(doc, payload);

          if (!erro) {
            if (tentativa == 0) {
              JsonArray resultados = doc["results"].as<JsonArray>();

              if (!resultados.isNull() && resultados.size() > 0) {
                JsonArray historico = resultados[0]["historicalDataPrice"].as<JsonArray>();

                for (JsonObject candle : historico) {
                  float fechamento = candle["close"] | 0.0f;
                  float abertura = candle["open"] | fechamento;
                  float maxima = candle["high"] | fechamento;
                  float minima = candle["low"] | fechamento;
                  time_t tempo = candle["date"] | (time_t)0;

                  if (fechamento > 0 && maxima > 0 && minima > 0 && novaQuantidade < MAX_PONTOS_GRAFICO) {
                    novoAbertura[novaQuantidade] = abertura;
                    novoMaxima[novaQuantidade] = maxima;
                    novoMinima[novaQuantidade] = minima;
                    novoFechamento[novaQuantidade] = fechamento;
                    novoTempo[novaQuantidade] = tempo;
                    novaQuantidade++;
                  }
                }
              }
            } else {
              JsonArray resultados = doc["chart"]["result"].as<JsonArray>();

              if (!resultados.isNull() && resultados.size() > 0) {
                JsonArray timestamps = resultados[0]["timestamp"].as<JsonArray>();
                JsonArray opens = resultados[0]["indicators"]["quote"][0]["open"].as<JsonArray>();
                JsonArray highs = resultados[0]["indicators"]["quote"][0]["high"].as<JsonArray>();
                JsonArray lows = resultados[0]["indicators"]["quote"][0]["low"].as<JsonArray>();
                JsonArray closes = resultados[0]["indicators"]["quote"][0]["close"].as<JsonArray>();

                int total = closes.size();

                for (int i = 0; i < total && novaQuantidade < MAX_PONTOS_GRAFICO; i++) {
                  if (closes[i].isNull() || timestamps[i].isNull()) continue;

                  float fechamento = closes[i].as<float>();
                  float abertura = fechamento;
                  float maxima = fechamento;
                  float minima = fechamento;

                  if (i < opens.size() && !opens[i].isNull()) abertura = opens[i].as<float>();
                  if (i < highs.size() && !highs[i].isNull()) maxima = highs[i].as<float>();
                  if (i < lows.size() && !lows[i].isNull()) minima = lows[i].as<float>();

                  if (fechamento > 0) {
                    novoAbertura[novaQuantidade] = abertura;
                    novoMaxima[novaQuantidade] = maxima;
                    novoMinima[novaQuantidade] = minima;
                    novoFechamento[novaQuantidade] = fechamento;
                    novoTempo[novaQuantidade] = timestamps[i].as<time_t>();
                    novaQuantidade++;
                  }
                }
              }
            }
          }
        }

        http.end();
      }

      if (novaQuantidade > 1) {
        carregou = true;
        Serial.print("[VGIA11] Historico diario carregado: ");
        Serial.print(novaQuantidade);
        Serial.println(" candles.");
      }
    }
  }

  if (carregou) {
    quantidadeHistorico = novaQuantidade;

    for (int i = 0; i < novaQuantidade; i++) {
      aberturaVGIA11[i] = novoAbertura[i];
      maximaVGIA11[i] = novoMaxima[i];
      minimaVGIA11[i] = novoMinima[i];
      historicoVGIA11[i] = novoFechamento[i];
      tempoHistoricoVGIA11[i] = novoTempo[i];
    }

    cotacaoVGIA11 = historicoVGIA11[quantidadeHistorico - 1];
    ultimaAtualizacaoHistorico = millis();

    Serial.print("[VGIA11] Grafico pronto com ");
    Serial.print(quantidadeHistorico);
    Serial.println(" pontos reais.");
  } else {
    Serial.println("[VGIA11] Nao foi possivel carregar dados reais.");
  }
}

void buscarTemperaturaELocalizacao() {
  HTTPClient http;

  http.setTimeout(4000);

  http.begin(
    "http://ip-api.com/json/?fields=lat,lon"
  );

  int codeGeo =
    http.GET();

  float lat = 0.0;
  float lon = 0.0;

  if (codeGeo == HTTP_CODE_OK) {
    String payloadGeo =
      http.getString();

    DynamicJsonDocument docGeo(1024);

    if (!deserializeJson(docGeo, payloadGeo)) {
      lat = docGeo["lat"];
      lon = docGeo["lon"];
    }
  }

  http.end();

  if (lat != 0.0 && lon != 0.0) {
    String urlClima =
      "http://api.open-meteo.com/v1/forecast"
      "?latitude=" +
      String(lat, 4) +
      "&longitude=" +
      String(lon, 4) +
      "&current_weather=true";

    http.begin(urlClima);

    int codeClima =
      http.GET();

    if (codeClima == HTTP_CODE_OK) {
      String payloadClima =
        http.getString();

      DynamicJsonDocument docClima(2048);

      if (!deserializeJson(docClima, payloadClima)) {
        tempOnline =
          docClima["current_weather"]
                  ["temperature"];

        codigoClima =
          docClima["current_weather"]
                  ["weathercode"] | -1;

        descricaoClima =
          obterDescricaoClima(codigoClima);
      }
    }

    http.end();
  }

  ultimaAtualizacaoClima =
    millis();
}

String obterDescricaoClima(int codigo) {
  switch (codigo) {
    case 0:
      return "Ensolarado";

    case 1:
      return "Poucas nuvens";

    case 2:
      return "Parcialmente nublado";

    case 3:
      return "Nublado";

    case 45:
    case 48:
      return "Nevoeiro";

    case 51:
    case 53:
    case 55:
      return "Chuvisco";

    case 56:
    case 57:
      return "Chuvisco gelado";

    case 61:
    case 63:
    case 65:
      return "Chuva";

    case 66:
    case 67:
      return "Chuva gelada";

    case 71:
    case 73:
    case 75:
      return "Neve";

    case 77:
      return "Granizo";

    case 80:
    case 81:
    case 82:
      return "Aguaceiro";

    case 85:
    case 86:
      return "Neve forte";

    case 95:
      return "Tempestade";

    case 96:
    case 99:
      return "Tempestade forte";

    default:
      return "Clima desconhecido";
  }
}

void telaTempoERelogio() {
  int luzLDR = lerLDR();

  int luzPorcentagem =
    map(
      luzLDR,
      4095,
      0,
      0,
      100
    );

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);

  display.println(
    "--- RELOGIO & CLIMA ---"
  );

  struct tm timeinfo;

  bool horaOk =
    getLocalTime(
      &timeinfo,
      50
    );

  if (horaOk) {
    char strHora[10];
    char strData[12];

    strftime(
      strHora,
      sizeof(strHora),
      "%H:%M:%S",
      &timeinfo
    );

    strftime(
      strData,
      sizeof(strData),
      "%d/%m/%Y",
      &timeinfo
    );

    display.setCursor(15, 14);

    display.setTextSize(2);

    display.println(strHora);

    display.setTextSize(1);

    display.setCursor(30, 32);

    display.println(strData);
  } else {
    display.setCursor(0, 20);

    display.println(
      "Sincronizando Hora..."
    );
  }

  display.setCursor(0, 46);

  if (tempOnline != 0.0) {
    display.print("Temp: ");

    display.print(
      tempOnline,
      1
    );

    display.print(" C ");
  } else {
    display.print(
      "Temp: --.- C "
    );
  }

  display.print("Luz: ");

  display.print(
    luzPorcentagem
  );

  display.println("%");

  display.setCursor(0, 56);

  display.print("Wi-Fi: ");

  display.println(
    WiFi.status() ==
    WL_CONNECTED
    ? "Conectado"
    : "Buscando..."
  );

  display.display();
}

void desenharIconeClima(
  int codigo,
  int cx,
  int cy
) {
  if (codigo == 0) {
    display.drawCircle(
      cx,
      cy,
      8,
      SSD1306_WHITE
    );

    for (int i = 0; i < 8; i++) {
      float ang =
        i * 0.785398;

      int x1 =
        cx +
        cos(ang) * 11;

      int y1 =
        cy +
        sin(ang) * 11;

      int x2 =
        cx +
        cos(ang) * 15;

      int y2 =
        cy +
        sin(ang) * 15;

      display.drawLine(
        x1,
        y1,
        x2,
        y2,
        SSD1306_WHITE
      );
    }

    return;
  }

  if (
    codigo == 1 ||
    codigo == 2 ||
    codigo == 3
  ) {
    display.fillCircle(
      cx - 7,
      cy,
      7,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 3,
      cy - 5,
      9,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 10,
      cy + 1,
      6,
      SSD1306_WHITE
    );

    display.fillRect(
      cx - 12,
      cy,
      28,
      8,
      SSD1306_WHITE
    );

    return;
  }

  if (
    codigo >= 45 &&
    codigo <= 48
  ) {
    display.drawFastHLine(
      cx - 15,
      cy - 7,
      30,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      cx - 12,
      cy,
      24,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      cx - 15,
      cy + 7,
      30,
      SSD1306_WHITE
    );

    return;
  }

  if (
    (codigo >= 51 &&
     codigo <= 67) ||
    (codigo >= 80 &&
     codigo <= 82)
  ) {
    display.fillCircle(
      cx - 7,
      cy,
      7,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 3,
      cy - 5,
      9,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 10,
      cy + 1,
      6,
      SSD1306_WHITE
    );

    display.fillRect(
      cx - 12,
      cy,
      28,
      8,
      SSD1306_WHITE
    );

    display.drawLine(
      cx - 8,
      cy + 12,
      cx - 11,
      cy + 17,
      SSD1306_WHITE
    );

    display.drawLine(
      cx,
      cy + 12,
      cx - 3,
      cy + 17,
      SSD1306_WHITE
    );

    display.drawLine(
      cx + 8,
      cy + 12,
      cx + 5,
      cy + 17,
      SSD1306_WHITE
    );

    return;
  }

  if (
    codigo == 95 ||
    codigo == 96 ||
    codigo == 99
  ) {
    display.fillCircle(
      cx - 7,
      cy,
      7,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 3,
      cy - 5,
      9,
      SSD1306_WHITE
    );

    display.fillCircle(
      cx + 10,
      cy + 1,
      6,
      SSD1306_WHITE
    );

    display.fillRect(
      cx - 12,
      cy,
      28,
      8,
      SSD1306_WHITE
    );

    display.drawLine(
      cx + 1,
      cy + 8,
      cx - 4,
      cy + 17,
      SSD1306_WHITE
    );

    display.drawLine(
      cx - 4,
      cy + 17,
      cx + 3,
      cy + 17,
      SSD1306_WHITE
    );

    return;
  }

  display.drawCircle(
    cx,
    cy,
    12,
    SSD1306_WHITE
  );
}

void telaClimaDetalhado() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);

  display.println(
    "--- CLIMA ATUAL ---"
  );

  display.drawFastHLine(
    0,
    9,
    128,
    SSD1306_WHITE
  );

  desenharIconeClima(
    codigoClima,
    22,
    34
  );

  String clima =
    descricaoClima;

  display.setCursor(45, 17);

  if (clima.length() <= 13) {
    display.println(clima);
  } else {
    display.println(
      clima.substring(0, 13)
    );

    display.setCursor(45, 27);

    display.println(
      clima.substring(13, 26)
    );
  }

  display.setCursor(45, 40);

  display.print("Temp: ");

  if (tempOnline != 0.0) {
    display.print(
      tempOnline,
      1
    );

    display.println(" C");
  } else {
    display.println("--.- C");
  }

  display.setCursor(0, 55);

  display.println(
    "Clique: voltar menu"
  );

  display.display();
}

void telaVGIA11() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("VGIA11 R$");
  if (cotacaoVGIA11 > 0) display.print(cotacaoVGIA11, 2); else display.print("--.--");

  if (quantidadeHistorico > 1) {
    float menor = minimaVGIA11[0];
    float maior = maximaVGIA11[0];

    for (int i = 1; i < quantidadeHistorico; i++) {
      if (minimaVGIA11[i] < menor) menor = minimaVGIA11[i];
      if (maximaVGIA11[i] > maior) maior = maximaVGIA11[i];
    }

    float faixa = maior - menor;
    if (faixa < 0.05f) faixa = 0.05f;

    const int x0 = 22;
    const int x1 = 127;
    const int y0 = 14;
    const int y1 = 50;
    const int w = x1 - x0;
    const int h = y1 - y0;

    for (int g = 1; g < 4; g++) {
      int gy = y0 + (g * h / 4);
      for (int x = x0; x <= x1; x += 4) display.drawPixel(x, gy, SSD1306_WHITE);
    }

    display.drawFastVLine(x0, y0, h + 1, SSD1306_WHITE);
    display.drawFastHLine(x0, y1, w + 1, SSD1306_WHITE);

    display.setCursor(0, 12);
    display.print(maior, 1);
    display.setCursor(0, 29);
    display.print((maior + menor) * 0.5f, 1);
    display.setCursor(0, 47);
    display.print(menor, 1);

    for (int i = 0; i < quantidadeHistorico; i++) {
      int x = x0 + ((long)i * w) / (quantidadeHistorico - 1);
      int yMax = y1 - (int)(((maximaVGIA11[i] - menor) / faixa) * h);
      int yMin = y1 - (int)(((minimaVGIA11[i] - menor) / faixa) * h);
      int yAbertura = y1 - (int)(((aberturaVGIA11[i] - menor) / faixa) * h);
      int yFechamento = y1 - (int)(((historicoVGIA11[i] - menor) / faixa) * h);

      yMax = constrain(yMax, y0, y1);
      yMin = constrain(yMin, y0, y1);
      yAbertura = constrain(yAbertura, y0, y1);
      yFechamento = constrain(yFechamento, y0, y1);

      display.drawLine(x, yMax, x, yMin, SSD1306_WHITE);

      int corpoTopo = min(yAbertura, yFechamento);
      int corpoBase = max(yAbertura, yFechamento);

      if (corpoTopo == corpoBase) {
        display.drawFastHLine(x - 1, corpoTopo, 3, SSD1306_WHITE);
      } else {
        display.fillRect(x, corpoTopo, 2, corpoBase - corpoTopo + 1, SSD1306_WHITE);
      }
    }

    char horaInicio[6] = "--:--";
    char horaMeio[6] = "--:--";
    char horaFim[6] = "--:--";

    if (tempoHistoricoVGIA11[0] > 0) {
      struct tm tmInicio, tmMeio, tmFim;
      localtime_r(&tempoHistoricoVGIA11[0], &tmInicio);
      localtime_r(&tempoHistoricoVGIA11[quantidadeHistorico / 2], &tmMeio);
      localtime_r(&tempoHistoricoVGIA11[quantidadeHistorico - 1], &tmFim);
      bool historicoDiario = tmInicio.tm_yday != tmFim.tm_yday || tmInicio.tm_year != tmFim.tm_year;

      if (historicoDiario) {
        strftime(horaInicio, sizeof(horaInicio), "%d/%m", &tmInicio);
        strftime(horaMeio, sizeof(horaMeio), "%d/%m", &tmMeio);
        strftime(horaFim, sizeof(horaFim), "%d/%m", &tmFim);
      } else {
        strftime(horaInicio, sizeof(horaInicio), "%H:%M", &tmInicio);
        strftime(horaMeio, sizeof(horaMeio), "%H:%M", &tmMeio);
        strftime(horaFim, sizeof(horaFim), "%H:%M", &tmFim);
      }
    }

    display.setCursor(22, 56);
    display.print(horaInicio);

    display.setCursor(61, 56);
    display.print(horaMeio);

    display.setCursor(100, 56);
    display.print(horaFim);
  } else {
    display.setCursor(18, 34);
    display.print("Carregando dados...");
  }

  display.display();
}

void iniciarAcordarRobo() {
  if (!roboDormindo || acordandoRobo) return;
  roboDormindo = false;
  acordandoRobo = true;
  inicioAcordar = millis();
  tempoParaAcordar = 900;
  etapaAcordar = 0;
  olharX = 0;
  olharY = 0;
  alvoOlharX = 0;
  alvoOlharY = 0;
  olhosFechados = true;
  expressaoAtual = 4;
}

void atualizarRobo() {
  int luzLDR = lerLDR();
  int luminosidadePct = map(constrain(luzLDR, 0, 4095), 4095, 0, 0, 100);
  if (LDR_INVERTIDO) luminosidadePct = 100 - luminosidadePct;
  luminosidadePct = constrain(luminosidadePct, 0, 100);
  ultimaLuzRobo = luminosidadePct;
  ultimoLDRValido = millis();

  if (millis() - ultimoDebugRobo >= 1000) {
    ultimoDebugRobo = millis();
    Serial.print("[ROBO] LDR=");
    Serial.print(luzLDR);
    Serial.print(" luz=");
    Serial.print(luminosidadePct);
    Serial.print("% escuro=");
    Serial.print(ambienteEscuro ? "SIM" : "NAO");
    Serial.print(" joystick=");
    Serial.println(joystickRoboMexendo ? "MEXENDO" : "PARADO");
  }

  bool escuroAgora = luminosidadePct < LDR_LIMIAR_LUZ_DORMIR;
  bool claroAgora = luminosidadePct >= LDR_LIMIAR_LUZ_ACORDAR;

  if (escuroAgora) {
    if (!ambienteEscuro) {
      ambienteEscuro = true;
      inicioEscuro = millis();
    }
  } else if (claroAgora) {
    ambienteEscuro = false;
    inicioEscuro = 0;
  }

  if (joystickRoboMexendo) {
    inicioEscuro = 0;
    ambienteEscuro = false;
    if (roboDormindo || acordandoRobo) {
      roboDormindo = false;
      acordandoRobo = false;
      olhosFechados = false;
      expressaoAtual = 3;
      alvoOlharX = olharX;
      alvoOlharY = olharY;
      inicioComportamentoRobo = millis();
    }
    return;
  }

  if (ambienteEscuro) {
    if (inicioEscuro == 0) inicioEscuro = millis();
    if (!roboDormindo && !acordandoRobo && millis() - inicioEscuro >= TEMPO_ESCURECIDO_PARA_DORMIR) {
      roboDormindo = true;
      acordandoRobo = false;
      olhosFechados = true;
      olharX = 0;
      olharY = 0;
      alvoOlharX = 0;
      alvoOlharY = 0;
      expressaoAtual = 4;
      comportamentoRobo = 0;
      return;
    }
    if (roboDormindo) return;
  } else {
    inicioEscuro = 0;
  }

  if (roboDormindo && !acordandoRobo) {
    iniciarAcordarRobo();
    return;
  }

  if (acordandoRobo) {
    unsigned long t = millis() - inicioAcordar;
    if (t < 350) {
      olhosFechados = true;
      expressaoAtual = 4;
      return;
    }
    if (t < 650) {
      olhosFechados = false;
      olharX = -3;
      alvoOlharX = -3;
      expressaoAtual = 0;
      return;
    }
    if (t < 950) {
      olharX = 3;
      alvoOlharX = 3;
      expressaoAtual = 0;
      return;
    }
    acordandoRobo = false;
    olhosFechados = false;
    olharX = 0;
    olharY = 0;
    alvoOlharX = 0;
    alvoOlharY = 0;
    expressaoAtual = 0;
    comportamentoRobo = 0;
    inicioComportamentoRobo = millis();
    proximoMovimentoRobo = millis() + 1800;
    proximoPiscarRobo = millis() + 3000;
    return;
  }

  unsigned long agora = millis();

  if (agora >= proximoPiscarRobo && !olhosFechados) {
    olhosFechados = true;
    inicioAnimacao = agora;
  }

  if (olhosFechados && agora - inicioAnimacao >= 140) {
    olhosFechados = false;
    proximoPiscarRobo = agora + random(3000, 6500);
  }

  if (agora >= proximoMovimentoRobo) {
    comportamentoRobo = random(0, 7);
    inicioComportamentoRobo = agora;
    proximoMovimentoRobo = agora + random(1800, 4200);

    if (comportamentoRobo == 0) {
      alvoOlharX = random(-5, 6);
      alvoOlharY = 0;
      expressaoAtual = 0;
    } else if (comportamentoRobo == 1) {
      alvoOlharX = -6;
      alvoOlharY = 0;
      expressaoAtual = 0;
    } else if (comportamentoRobo == 2) {
      alvoOlharX = 6;
      alvoOlharY = 0;
      expressaoAtual = 0;
    } else if (comportamentoRobo == 3) {
      alvoOlharX = 0;
      alvoOlharY = -4;
      expressaoAtual = 0;
    } else if (comportamentoRobo == 4) {
      alvoOlharX = 0;
      alvoOlharY = 2;
      expressaoAtual = 1;
    } else if (comportamentoRobo == 5) {
      alvoOlharX = random(-3, 4);
      alvoOlharY = random(-2, 3);
      expressaoAtual = 3;
    } else {
      alvoOlharX = 0;
      alvoOlharY = 0;
      expressaoAtual = 0;
    }
  }

  if (agora - inicioComportamentoRobo > 900) {
    if (comportamentoRobo == 1 || comportamentoRobo == 2 || comportamentoRobo == 3) {
      alvoOlharX = 0;
      alvoOlharY = 0;
    }
    if (comportamentoRobo == 4 || comportamentoRobo == 5) {
      expressaoAtual = 0;
    }
  }

  if (olharX < alvoOlharX) olharX++;
  else if (olharX > alvoOlharX) olharX--;
  if (olharY < alvoOlharY) olharY++;
  else if (olharY > alvoOlharY) olharY--;
}

void animarRoboAutonomo() {
  atualizarRobo();
}

void desenharOlhoNormal(
  int x,
  int y,
  int deslocX,
  int deslocY
) {
  display.drawRoundRect(
    x - 16,
    y - 16,
    32,
    32,
    6,
    SSD1306_WHITE
  );

  display.fillCircle(
    x + deslocX,
    y + deslocY,
    6,
    SSD1306_WHITE
  );
}

void desenharRostoExpressao() {
  if (expressaoAtual == 0) {
    desenharOlhoNormal(
      36,
      26,
      olharX,
      olharY
    );

    desenharOlhoNormal(
      92,
      26,
      olharX,
      olharY
    );

    display.drawFastHLine(
      54,
      50,
      20,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 1) {
    desenharOlhoNormal(
      36,
      26,
      olharX,
      olharY
    );

    desenharOlhoNormal(
      92,
      26,
      olharX,
      olharY
    );

    display.drawLine(
      52,
      48,
      58,
      52,
      SSD1306_WHITE
    );

    display.drawLine(
      58,
      52,
      64,
      48,
      SSD1306_WHITE
    );

    display.drawLine(
      64,
      48,
      70,
      52,
      SSD1306_WHITE
    );

    display.drawLine(
      70,
      52,
      76,
      48,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 2) {
    desenharOlhoNormal(
      36,
      26,
      olharX,
      olharY
    );

    display.drawFastHLine(
      76,
      26,
      32,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      54,
      50,
      20,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 3) {
    display.drawRoundRect(
      20,
      10,
      32,
      32,
      8,
      SSD1306_WHITE
    );

    display.drawRoundRect(
      76,
      10,
      32,
      32,
      8,
      SSD1306_WHITE
    );

    display.fillCircle(
      36 + olharX,
      26 + olharY,
      6,
      SSD1306_WHITE
    );

    display.fillCircle(
      92 + olharX,
      26 + olharY,
      6,
      SSD1306_WHITE
    );

    display.drawCircle(
      64,
      51,
      6,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 4) {
    display.drawFastHLine(
      20,
      25,
      32,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      76,
      25,
      32,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      54,
      50,
      20,
      SSD1306_WHITE
    );

    display.setTextSize(1);
    display.setCursor(100, 8);
    display.print("Zz");
  }

  if (expressaoAtual == 5) {
    display.drawCircle(
      36,
      26,
      13,
      SSD1306_WHITE
    );

    display.drawCircle(
      92,
      26,
      13,
      SSD1306_WHITE
    );

    display.fillCircle(
      36 + olharX,
      26 + olharY,
      5,
      SSD1306_WHITE
    );

    display.fillCircle(
      92 + olharX,
      26 + olharY,
      5,
      SSD1306_WHITE
    );

    display.drawCircle(
      64,
      50,
      7,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 6) {
    display.drawLine(
      20,
      15,
      50,
      22,
      SSD1306_WHITE
    );

    display.drawLine(
      78,
      22,
      108,
      15,
      SSD1306_WHITE
    );

    desenharOlhoNormal(
      36,
      28,
      olharX,
      olharY
    );

    desenharOlhoNormal(
      92,
      28,
      olharX,
      olharY
    );

    display.drawFastHLine(
      54,
      50,
      20,
      SSD1306_WHITE
    );
  }

  if (expressaoAtual == 7) {
    display.drawLine(
      30,
      21,
      36,
      15,
      SSD1306_WHITE
    );

    display.drawLine(
      36,
      15,
      42,
      21,
      SSD1306_WHITE
    );

    display.drawLine(
      42,
      21,
      36,
      30,
      SSD1306_WHITE
    );

    display.drawLine(
      36,
      30,
      30,
      21,
      SSD1306_WHITE
    );

    display.drawLine(
      86,
      21,
      92,
      15,
      SSD1306_WHITE
    );

    display.drawLine(
      92,
      15,
      98,
      21,
      SSD1306_WHITE
    );

    display.drawLine(
      98,
      21,
      92,
      30,
      SSD1306_WHITE
    );

    display.drawLine(
      92,
      30,
      86,
      21,
      SSD1306_WHITE
    );

    display.drawLine(
      52,
      48,
      58,
      52,
      SSD1306_WHITE
    );

    display.drawLine(
      58,
      52,
      64,
      48,
      SSD1306_WHITE
    );

    display.drawLine(
      64,
      48,
      70,
      52,
      SSD1306_WHITE
    );

    display.drawLine(
      70,
      52,
      76,
      48,
      SSD1306_WHITE
    );
  }
}

void telaRoboInteligente() {
  int valX = lerJoystickAnalogico(PIN_JOY_X);
  int valY = lerJoystickAnalogico(PIN_JOY_Y);

  joystickRoboMexendo =
    valX > JOY_CIMA ||
    valX < JOY_BAIXO ||
    valY > JOY_DIREITA ||
    valY < JOY_ESQUERDA;

  display.clearDisplay();
  atualizarRobo();

  if (roboDormindo) {
    display.drawFastHLine(20, 25, 32, SSD1306_WHITE);
    display.drawFastHLine(76, 25, 32, SSD1306_WHITE);
    display.drawFastHLine(54, 50, 20, SSD1306_WHITE);

    int fase = (millis() / 650) % 3;
    if (fase == 0) {
      display.setTextSize(2);
      display.setCursor(106, 18);
      display.print("Z");
    } else if (fase == 1) {
      display.setTextSize(1);
      display.setCursor(105, 22);
      display.print("Zz");
    } else {
      display.setTextSize(1);
      display.setCursor(109, 24);
      display.print("z");
    }

    display.setTextSize(1);
    display.setCursor(0, 58);
    display.print("Dormindo");
    display.display();
    return;
  }

  if (acordandoRobo) {
    unsigned long t = millis() - inicioAcordar;
    if (t < 350) {
      display.drawFastHLine(20, 25, 32, SSD1306_WHITE);
      display.drawFastHLine(76, 25, 32, SSD1306_WHITE);
      display.drawFastHLine(54, 50, 20, SSD1306_WHITE);
      display.setCursor(100, 14);
      display.print("Zz");
    } else {
      desenharRostoExpressao();
    }
    display.display();
    return;
  }

  if (joystickRoboMexendo) {
    if (valX > JOY_CIMA) alvoOlharY = -6;
    else if (valX < JOY_BAIXO) alvoOlharY = 6;
    else alvoOlharY = 0;

    if (valY > JOY_DIREITA) alvoOlharX = 6;
    else if (valY < JOY_ESQUERDA) alvoOlharX = -6;
    else alvoOlharX = 0;

    if (valX < JOY_BAIXO) expressaoAtual = 7;
    else expressaoAtual = 3;
  }

  if (millis() - tempoSusto < 800) {
    expressaoAtual = 5;
  }

  if (olhosFechados) {
    display.drawFastHLine(20, 25, 32, SSD1306_WHITE);
    display.drawFastHLine(76, 25, 32, SSD1306_WHITE);
    display.drawFastHLine(55, 49, 18, SSD1306_WHITE);
  } else {
    desenharRostoExpressao();
  }

  display.setTextSize(1);
  display.setCursor(0, 58);
  display.print("Luz ");
  display.print(ultimaLuzRobo);
  display.print("%");
  display.display();
}

void telaMouseBluetooth() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("--- MOUSE BLUETOOTH ---");
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print("Status: ");
  display.println(mouseBLE.isPaired() ? "CONECTADO" : "AGUARDANDO");

  display.setCursor(0, 28);
  display.println("Joystick = mover");
  display.setCursor(0, 39);
  display.println("Clique = esquerdo");
  display.setCursor(0, 59);
  display.println("Clique longo = sair");

  display.display();
}

void iniciarAccessPoint() {
  if (
    tempAPSSID.length() == 0
  ) {
    tempAPSSID =
      "ESP32-WIFI";
  }

  if (
    tempAPSSID.length() > 32
  ) {
    tempAPSSID =
      tempAPSSID.substring(
        0,
        32
      );
  }

  apSSID =
    tempAPSSID;

  apPASS =
    tempAPPASS;

  if (
    apPASS.length() > 0 &&
    apPASS.length() < 8
  ) {
    apPASS = "";
  }

  preferences.begin(
    "wifi_config",
    false
  );

  preferences.putString(
    "apssid",
    apSSID
  );

  preferences.putString(
    "appass",
    apPASS
  );

  preferences.end();

  WiFi.mode(
    WIFI_AP_STA
  );



  if (apAtivo) {
    WiFi.softAPdisconnect(true);
    delay(100);
  }

  if (
    apPASS.length() >= 8
  ) {
    WiFi.softAP(
      apSSID.c_str(),
      apPASS.c_str()
    );
  } else {
    WiFi.softAP(
      apSSID.c_str()
    );
  }

  apAtivo = true;



  dnsServer.start(53, "*", WiFi.softAPIP());

  iniciarServidorWeb();

  for (
    int i = 0;
    i < MAX_USUARIOS_AP;
    i++
  ) {
    usuariosAP[i].usado = false;
    usuariosAP[i].nome = "";
    usuariosAP[i].senha = "";
    usuariosAP[i].ultimoAcesso = 0;
  }
}

void iniciarServidorWeb() {
  if (servidorIniciado) {
    return;
  }

  auto redirecionarPortal = []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "Portal ESP32");
  };

  server.on("/generate_204", HTTP_GET, redirecionarPortal);
  server.on("/hotspot-detect.html", HTTP_GET, redirecionarPortal);
  server.on("/connecttest.txt", HTTP_GET, redirecionarPortal);
  server.on("/ncsi.txt", HTTP_GET, redirecionarPortal);
  server.on("/fwlink", HTTP_GET, redirecionarPortal);

  server.on(
    "/",
    HTTP_GET,
    []() {
      String html;

      html +=
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32 Wi-Fi</title>"
        "<style>"
        "body{font-family:Arial;text-align:center;background:#111;color:white;padding:30px}"
        "input{padding:14px;width:85%;font-size:18px;margin:8px;border-radius:8px;border:0}"
        "button{padding:14px 25px;font-size:18px;border:0;border-radius:8px}"
        "</style>"
        "</head>"
        "<body>";

      html +=
        "<h1>ESP32 Wi-Fi</h1>";

      html +=
        "<p>Identificacao de usuario.</p>";

      html +=
        "<form action='/login' method='POST'>";

      html +=
        "<label>E-mail</label><br>";

      html +=
        "<input name='email' maxlength='40' placeholder='Email@' required>";

      html +=
        "<label>Senha</label><br>";

      html +=
        "<input name='senha' type='password' maxlength='12' value='432' required>";

      html +=
        "<p style='font-size:13px;color:#aaa'>Este e um acesso ficticio local da ESP32. Nao e login do Google.</p>";

      html +=
        "<br><button type='submit'>Entrar</button>";

      html +=
        "</form>";

      html +=
        "</body></html>";

      server.send(
        200,
        "text/html",
        html
      );
    }
  );

  server.on(
    "/login",
    HTTP_POST,
    []() {
      String nome =
        server.arg("email");

      String senha =
        server.arg("senha");

      nome.trim();

      if (
        nome.length() == 0
      ) {
        server.send(
          400,
          "text/plain",
          "E-mail invalido"
        );

        return;
      }

      if (nome.length() > 40) {
        nome = nome.substring(0, 40);
      }

      IPAddress ip =
        server.client().remoteIP();

      int encontrado = -1;

      for (
        int i = 0;
        i < MAX_USUARIOS_AP;
        i++
      ) {
        if (
          usuariosAP[i].usado &&
          usuariosAP[i].ip == ip
        ) {
          encontrado = i;
          break;
        }
      }

      if (encontrado == -1) {
        for (
          int i = 0;
          i < MAX_USUARIOS_AP;
          i++
        ) {
          if (
            !usuariosAP[i].usado
          ) {
            encontrado = i;
            break;
          }
        }
      }

      if (encontrado != -1) {
        usuariosAP[encontrado].usado = true;
        usuariosAP[encontrado].ip = ip;
        usuariosAP[encontrado].nome = nome;
        usuariosAP[encontrado].senha = senha;
        usuariosAP[encontrado].ultimoAcesso = millis();
      }

      String html;

      html +=
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Conectado</title>"
        "<style>"
        "body{font-family:Arial;text-align:center;background:#111;color:white;padding:30px}"
        "</style>"
        "</head>"
        "<body>";

      html +=
        "<h1>Conectado!</h1>";

      html +=
        "<h2>Usuario: " +
        nome +
        "</h2>";

      html +=
        "<p>Acesso local ficticio autorizado.</p>";

      html +=
        "<p>IP: " +
        ip.toString() +
        "</p>";

      html +=
        "<meta http-equiv='refresh' content='5'>";

      html +=
        "</body></html>";

      server.send(
        200,
        "text/html",
        html
      );
    }
  );

  server.on(
    "/usuarios",
    HTTP_GET,
    []() {
      String html;

      html +=
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Usuarios</title>"
        "<meta http-equiv='refresh' content='3'>"
        "<style>"
        "body{font-family:Arial;background:#111;color:white;padding:20px}"
        "li{padding:10px}"
        "</style>"
        "</head>"
        "<body>";

      html +=
        "<h1>Usuarios conectados</h1>";

      html +=
        "<ul>";

      for (
        int i = 0;
        i < MAX_USUARIOS_AP;
        i++
      ) {
        if (
          usuariosAP[i].usado
        ) {
          html +=
            String("<li style='margin-bottom:18px'>") +
            "<div style='font-size:20px;font-weight:bold'>" +
            usuariosAP[i].nome +
            "</div>" +
            "<div style='font-size:16px;color:#aaa'>Senha: " +
            usuariosAP[i].senha +
            "</div>" +
            "</li>";
        }
      }

      html +=
        "</ul>";

      html +=
        "</body></html>";

      server.send(
        200,
        "text/html",
        html
      );
    }
  );

  server.begin();

  servidorIniciado = true;
}

void atualizarUsuariosAP() {
  if (!apAtivo) {
    return;
  }

  int quantidade =
    WiFi.softAPgetStationNum();

  for (
    int i = 0;
    i < MAX_USUARIOS_AP;
    i++
  ) {
    if (
      usuariosAP[i].usado &&
      millis() -
      usuariosAP[i].ultimoAcesso >
      3600000
    ) {
      usuariosAP[i].usado = false;
      usuariosAP[i].nome = "";
      }
  }

  if (
    quantidade == 0
  ) {
    for (
      int i = 0;
      i < MAX_USUARIOS_AP;
      i++
    ) {
      if (
        usuariosAP[i].usado &&
        millis() -
        usuariosAP[i].ultimoAcesso >
        60000
      ) {
        usuariosAP[i].usado = false;
        usuariosAP[i].nome = "";
        }
    }
  }
}

void telaCriarWiFi() {
  int joyX = lerJoystickAnalogico(PIN_JOY_X);
  int joyY = lerJoystickAnalogico(PIN_JOY_Y);
  int btn = digitalRead(PIN_JOY_SW);

  bool movCima = joyX > JOY_CIMA;
  bool movBaixo = joyX < JOY_BAIXO;
  bool movDireita = joyY > JOY_DIREITA;
  bool movEsquerda = joyY < JOY_ESQUERDA;
  bool movendo = movCima || movBaixo || movDireita || movEsquerda;

  if (!movendo) {
    joyEmMovimento = false;
  }







  if (estadoAPFlow == 0) {
    if (movCima || movBaixo) {
      unsigned long agora = millis();
      int intervalo = joyEmMovimento ? VELOCIDADE_DESLIZE : PAUSA_INICIAL_REPETICAO;

      if (!joyEmMovimento || agora - tempoUltimaAcaoJoy >= intervalo) {
        if (movCima && apMenuSelecionado > 0) apMenuSelecionado--;
        if (movBaixo && apMenuSelecionado < 2) apMenuSelecionado++;
        tempoUltimaAcaoJoy = agora;
        joyEmMovimento = true;
      }
    }

    if (btn == LOW && millis() - ultimoDebounceBotao > 250) {
      ultimoDebounceBotao = millis();
      joyEmMovimento = false;

      if (apMenuSelecionado == 0) {
        estadoAPFlow = 2;
      } else if (apMenuSelecionado == 1) {
        estadoAPFlow = 1;
        campoAP = 0;
        teclaX = 0;
        teclaY = 0;
        modoTeclado = 0;
        tempAPSSID = apSSID;
        tempAPPASS = apPASS;
      } else {
        estadoAPFlow = 0;
        emSubMenu = false;
      }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("CRIAR WI-FI");
      display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    const char* opcoes[3] = {
      "Usuarios conectados",
      "Configurar Wi-Fi",
      "Voltar"
    };

    for (int i = 0; i < 3; i++) {
      int y = 15 + i * 15;
      if (i == apMenuSelecionado) {
        display.fillRect(0, y - 1, 128, 13, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(3, y + 1);
      display.print(i == apMenuSelecionado ? "> " : "  ");
      display.println(opcoes[i]);
    }

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 55);
    display.print("Rede: ");
    display.print(apSSID.substring(0, 14));
    display.setCursor(0, 63);
    display.print("Senha: ");
    if (apPASS.length() == 0) display.print("ABERTA");
    else display.print(apPASS.substring(0, 12));
    display.display();
    return;
  }




  if (estadoAPFlow == 2) {
    if (movCima || movBaixo || movEsquerda || movDireita) {

    }

    if (btn == LOW && millis() - ultimoDebounceBotao > 250) {
      ultimoDebounceBotao = millis();
      estadoAPFlow = 0;
      apMenuSelecionado = 0;
      joyEmMovimento = false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("USUARIOS / E-MAIL");
      display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    int linha = 13;
    int total = 0;
    for (int i = 0; i < MAX_USUARIOS_AP; i++) {
      if (!usuariosAP[i].usado) continue;
      total++;
      if (linha <= 48) {
        String nome = usuariosAP[i].nome;
        String senha = usuariosAP[i].senha;
        if (nome.length() > 18) nome = nome.substring(0, 18);
        if (senha.length() > 18) senha = senha.substring(0, 18);
        display.setCursor(0, linha);
        display.print("> ");
        display.println(nome);
        linha += 8;
        if (linha <= 48) {
          display.setCursor(4, linha);
          display.print("S:");
          display.println(senha);
          linha += 9;
        }
      }
    }

    if (total == 0) {
      display.setCursor(5, 30);
      display.println("Nenhum usuario");
    }

    display.setCursor(0, 58);
    display.print("Clique para voltar");
    display.display();
    return;
  }




  if (estadoAPFlow == 1) {
    if (movendo) {
      unsigned long agora = millis();
      int intervalo = joyEmMovimento ? VELOCIDADE_DESLIZE : PAUSA_INICIAL_REPETICAO;

      if (!joyEmMovimento || agora - tempoUltimaAcaoJoy >= intervalo) {
        if (movDireita && teclaX < 9) teclaX++;
        if (movEsquerda && teclaX > 0) teclaX--;
        if (movBaixo && teclaY < 3) teclaY++;
        if (movCima && teclaY > 0) teclaY--;
        tempoUltimaAcaoJoy = agora;
        joyEmMovimento = true;
      }
    }

    if (btn == LOW && millis() - ultimoDebounceBotao > 250) {
      ultimoDebounceBotao = millis();

      char tecla = ' ';
      if (modoTeclado == 0) tecla = tecladoMin[teclaY][teclaX];
      else if (modoTeclado == 1) tecla = tecladoNum[teclaY][teclaX];
      else tecla = tecladoSimbol[teclaY][teclaX];


      if (teclaX == 7 && teclaY == 3) {
        modoTeclado = (modoTeclado + 1) % 3;
      }

      else if (teclaX == 8 && teclaY == 3) {
        if (campoAP == 0) {
          if (tempAPSSID.length() > 0) tempAPSSID.remove(tempAPSSID.length() - 1);
        } else {
          if (tempAPPASS.length() > 0) tempAPPASS.remove(tempAPPASS.length() - 1);
        }
      }

      else if (teclaX == 9 && teclaY == 3) {
        if (campoAP == 0) {
          if (tempAPSSID.length() == 0) tempAPSSID = "ESP32-WIFI";
          campoAP = 1;
          teclaX = 0;
          teclaY = 0;
          modoTeclado = 0;
        } else {


          if (tempAPPASS.length() > 0 && tempAPPASS.length() < 8) {
            tempAPPASS = "";
          }

          iniciarAccessPoint();
          estadoAPFlow = 0;
          apMenuSelecionado = 0;
          emSubMenu = true;
          joyEmMovimento = false;
          return;
        }
      }

      else {
        if (campoAP == 0) {
          if (tempAPSSID.length() < 32) tempAPSSID += tecla;
        } else {
          if (tempAPPASS.length() < 18) tempAPPASS += tecla;
        }
      }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (campoAP == 0) {
      display.print("NOME WI-FI:");
      display.setCursor(0, 9);
      String nome = tempAPSSID;
      if (nome.length() > 20) nome = nome.substring(nome.length() - 20);
      display.println(nome);
    } else {
      display.print("SENHA WI-FI:");
      display.setCursor(0, 9);
      if (tempAPPASS.length() == 0) {
        display.println("[VAZIA]");
      } else {

        for (unsigned int i = 0; i < tempAPPASS.length() && i < 20; i++) display.print('*');
        display.println();
      }
    }

    display.drawFastHLine(0, 18, 128, SSD1306_WHITE);

    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 10; x++) {
        int posX = x * 13;
        int posY = 21 + y * 11;

        if (x == teclaX && y == teclaY) {
          display.fillRect(posX, posY, 12, 10, SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(posX + 2, posY + 1);
        char c = ' ';
        if (modoTeclado == 0) c = tecladoMin[y][x];
        else if (modoTeclado == 1) c = tecladoNum[y][x];
        else c = tecladoSimbol[y][x];

        if (x == 7 && y == 3) display.print("#");
        else if (x == 8 && y == 3) display.print("<");
        else if (x == 9 && y == 3) display.print("O");
        else display.print(c);
      }
    }

    display.setTextColor(SSD1306_WHITE);
      display.display();
    return;
  }
}



void escanearRedesWiFi() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(
    0,
    20
  );

  display.println(
    " Escaneando redes..."
  );

  display.display();

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
  }

  totalRedesEncontradas =
    WiFi.scanNetworks();

  redeSelecionada = 0;
  offsetListaWifi = 0;

  joyEmMovimento = false;
}


void telaFluxoWiFi() {
  int joyX =
    lerJoystickAnalogico(
      PIN_JOY_X
    );

  int joyY =
    lerJoystickAnalogico(
      PIN_JOY_Y
    );

  int btn =
    digitalRead(
      PIN_JOY_SW
    );

  bool movCima =
    joyX > JOY_CIMA;

  bool movBaixo =
    joyX < JOY_BAIXO;

  bool movDireita =
    joyY > JOY_DIREITA;

  bool movEsquerda =
    joyY < JOY_ESQUERDA;

  bool movendo =
    movCima ||
    movBaixo ||
    movDireita ||
    movEsquerda;

  if (!movendo) {
    joyEmMovimento = false;
  }

  if (estadoWifiFlow == 0) {
    int totalOpcoes =
      totalRedesEncontradas + 1;

    if (movCima || movBaixo) {
      unsigned long tempoAtual =
        millis();

      int intervaloRequerido =
        joyEmMovimento
        ? VELOCIDADE_DESLIZE
        : PAUSA_INICIAL_REPETICAO;

      if (
        !joyEmMovimento ||
        tempoAtual - tempoUltimaAcaoJoy >=
        intervaloRequerido
      ) {
        if (
          movCima &&
          redeSelecionada > 0
        ) {
          redeSelecionada--;

          if (
            redeSelecionada <
            offsetListaWifi
          ) {
            offsetListaWifi--;
          }
        }

        if (
          movBaixo &&
          redeSelecionada <
          totalOpcoes - 1
        ) {
          redeSelecionada++;

          if (
            redeSelecionada >=
            offsetListaWifi + 2
          ) {
            offsetListaWifi++;
          }
        }

        tempoUltimaAcaoJoy =
          tempoAtual;

        joyEmMovimento = true;
      }
    }

    if (
      btn == LOW &&
      millis() - ultimoDebounceBotao > 200
    ) {
      ultimoDebounceBotao =
        millis();

      if (
        redeSelecionada ==
        totalRedesEncontradas
      ) {
        emSubMenu = false;
        joyEmMovimento = false;

        return;
      } else if (
        totalRedesEncontradas > 0
      ) {
        wifiSSID =
          WiFi.SSID(
            redeSelecionada
          );

        tempPASS = "";

        teclaX = 0;
        teclaY = 0;

        estadoWifiFlow = 1;

        joyEmMovimento = false;
      }
    }

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(
      0,
      0
    );

    display.print(
      "REDES ("
    );

    display.print(
      totalRedesEncontradas
    );

    display.println(
      ")"
    );

    display.drawFastHLine(
      0,
      9,
      128,
      SSD1306_WHITE
    );

    for (int i = 0; i < 2; i++) {
      int idx =
        offsetListaWifi + i;

      if (
        idx >= totalOpcoes
      ) {
        break;
      }

      int posY =
        12 + (i * 26);

      if (
        idx ==
        redeSelecionada
      ) {
        display.fillRect(
          0,
          posY,
          128,
          24,
          SSD1306_WHITE
        );

        display.setTextColor(
          SSD1306_BLACK
        );
      } else {
        display.setTextColor(
          SSD1306_WHITE
        );
      }

      if (
        idx ==
        totalRedesEncontradas
      ) {
        display.setCursor(
          2,
          posY + 8
        );

        display.print(
          "< [ VOLTAR AO MENU ]"
        );
      } else {
        display.setCursor(
          2,
          posY + 2
        );

        String nomeRede =
          WiFi.SSID(idx);

        if (
          nomeRede.length() > 20
        ) {
          nomeRede =
            nomeRede.substring(
              0,
              20
            );
        }

        display.print(
          nomeRede
        );

        int rssi =
          WiFi.RSSI(idx);

        int qualidade =
          map(
            constrain(
              rssi,
              -100,
              -50
            ),
            -100,
            -50,
            0,
            100
          );

        display.setCursor(
          2,
          posY + 13
        );

        display.print(
          "Sinal: "
        );

        display.print(
          qualidade
        );

        display.print(
          "%"
        );
      }
    }

    display.setTextColor(
      SSD1306_WHITE
    );
    display.display();
  } else if (
    estadoWifiFlow == 1
  ) {
    if (movendo) {
      unsigned long tempoAtual =
        millis();

      int intervaloRequerido =
        joyEmMovimento
        ? VELOCIDADE_DESLIZE
        : PAUSA_INICIAL_REPETICAO;

      if (
        !joyEmMovimento ||
        tempoAtual - tempoUltimaAcaoJoy >=
        intervaloRequerido
      ) {
        if (
          movDireita &&
          teclaX < 9
        ) {
          teclaX++;
        }

        if (
          movEsquerda &&
          teclaX > 0
        ) {
          teclaX--;
        }

        if (
          movBaixo &&
          teclaY < 3
        ) {
          teclaY++;
        }

        if (
          movCima &&
          teclaY > 0
        ) {
          teclaY--;
        }

        tempoUltimaAcaoJoy =
          tempoAtual;

        joyEmMovimento = true;
      }
    } else {
      joyEmMovimento = false;
    }

    if (
      btn == LOW &&
      millis() - ultimoDebounceBotao > 200
    ) {
      ultimoDebounceBotao =
        millis();

      char tecla = ' ';

      if (modoTeclado == 0) {
        tecla =
          tecladoMin[
            teclaY
          ][
            teclaX
          ];
      } else if (
        modoTeclado == 1
      ) {
        tecla =
          tecladoNum[
            teclaY
          ][
            teclaX
          ];
      } else {
        tecla =
          tecladoSimbol[
            teclaY
          ][
            teclaX
          ];
      }

      if (
        teclaX == 7 &&
        teclaY == 3
      ) {
        modoTeclado =
          (modoTeclado + 1) %
          3;
      } else if (
        teclaX == 8 &&
        teclaY == 3
      ) {
        if (
          tempPASS.length() > 0
        ) {
          tempPASS.remove(
            tempPASS.length() - 1
          );
        }
      } else if (
        teclaX == 9 &&
        teclaY == 3
      ) {
        wifiPASS =
          tempPASS;

        preferences.begin(
          "wifi_config",
          false
        );

        preferences.putString(
          "ssid",
          wifiSSID
        );

        preferences.putString(
          "pass",
          wifiPASS
        );

        preferences.end();

        iniciarWiFiSemTravar();

        emSubMenu = false;

        joyEmMovimento = false;

        return;
      } else {
        if (
          tempPASS.length() < 18
        ) {
          tempPASS += tecla;
        }
      }
    }

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(
      SSD1306_WHITE
    );

    display.setCursor(
      0,
      0
    );

    display.print(
      "P:"
    );

    display.println(
      tempPASS
    );

    display.drawFastHLine(
      0,
      10,
      128,
      SSD1306_WHITE
    );

    for (
      int y = 0;
      y < 4;
      y++
    ) {
      for (
        int x = 0;
        x < 10;
        x++
      ) {
        int posX =
          x * 13;

        int posY =
          13 + (y * 12);

        if (
          x == teclaX &&
          y == teclaY
        ) {
          display.fillRect(
            posX,
            posY,
            12,
            11,
            SSD1306_WHITE
          );

          display.setTextColor(
            SSD1306_BLACK
          );
        } else {
          display.setTextColor(
            SSD1306_WHITE
          );
        }

        display.setCursor(
          posX + 2,
          posY + 2
        );

        char c = ' ';

        if (modoTeclado == 0) {
          c =
            tecladoMin[y][x];
        } else if (
          modoTeclado == 1
        ) {
          c =
            tecladoNum[y][x];
        } else {
          c =
            tecladoSimbol[y][x];
        }

        if (
          x == 7 &&
          y == 3
        ) {
          display.print("#");
        } else if (
          x == 8 &&
          y == 3
        ) {
          display.print("<");
        } else if (
          x == 9 &&
          y == 3
        ) {
          display.print("O");
        } else {
          display.print(c);
        }
      }
    }

    display.setTextColor(
      SSD1306_WHITE
    );
    display.display();
  }
}



void calcularResultadoCalculadora() {
  double segundoValor = calculadoraDisplay.toDouble();

  if (!calculadoraTemValor || calculadoraOperacao == 0) {
    calculadoraValor = segundoValor;
    return;
  }

  if (calculadoraOperacao == '+') {
    calculadoraValor += segundoValor;
  } else if (calculadoraOperacao == '-') {
    calculadoraValor -= segundoValor;
  } else if (calculadoraOperacao == '*') {
    calculadoraValor *= segundoValor;
  } else if (calculadoraOperacao == '/') {
    if (segundoValor == 0.0) {
      calculadoraDisplay = "Erro";
      calculadoraValor = 0.0;
      calculadoraOperacao = 0;
      calculadoraNovoNumero = true;
      calculadoraTemValor = false;
      return;
    }
    calculadoraValor /= segundoValor;
  }

  String resultado = String(calculadoraValor, 8);
  while (resultado.endsWith("0")) resultado.remove(resultado.length() - 1);
  if (resultado.endsWith(".")) resultado.remove(resultado.length() - 1);
  if (resultado == "-0") resultado = "0";

  calculadoraDisplay = resultado;
}

void selecionarTeclaCalculadora(char tecla) {
  if (tecla >= '0' && tecla <= '9') {
    if (calculadoraDisplay == "Erro") {
      calculadoraDisplay = "0";
      calculadoraOperacao = 0;
      calculadoraTemValor = false;
    }

    if (calculadoraNovoNumero || calculadoraDisplay == "0") {
      calculadoraDisplay = String(tecla);
      calculadoraNovoNumero = false;
    } else if (calculadoraDisplay.length() < 10) {
      calculadoraDisplay += tecla;
    }
    return;
  }

  if (tecla == '.') {
    if (calculadoraDisplay == "Erro") {
      calculadoraDisplay = "0";
      calculadoraOperacao = 0;
      calculadoraTemValor = false;
      calculadoraNovoNumero = false;
    }

    if (calculadoraNovoNumero) {
      calculadoraDisplay = "0.";
      calculadoraNovoNumero = false;
    } else if (calculadoraDisplay.indexOf('.') < 0 && calculadoraDisplay.length() < 10) {
      calculadoraDisplay += ".";
    }
    return;
  }

  if (tecla == 'C') {
    calculadoraDisplay = "0";
    calculadoraValor = 0.0;
    calculadoraOperacao = 0;
    calculadoraNovoNumero = true;
    calculadoraTemValor = false;
    return;
  }

  if (tecla == '<') {
    if (!calculadoraNovoNumero && calculadoraDisplay != "Erro") {
      if (calculadoraDisplay.length() > 1) {
        calculadoraDisplay.remove(calculadoraDisplay.length() - 1);
      } else {
        calculadoraDisplay = "0";
      }
    }
    return;
  }

  if (tecla == '%') {
    if (calculadoraDisplay != "Erro") {
      double valor = calculadoraDisplay.toDouble() / 100.0;
      calculadoraDisplay = String(valor, 8);
      while (calculadoraDisplay.endsWith("0")) calculadoraDisplay.remove(calculadoraDisplay.length() - 1);
      if (calculadoraDisplay.endsWith(".")) calculadoraDisplay.remove(calculadoraDisplay.length() - 1);
      if (calculadoraDisplay == "") calculadoraDisplay = "0";
      calculadoraNovoNumero = false;
    }
    return;
  }

  if (tecla == '+' || tecla == '-' || tecla == '*' || tecla == '/') {
    if (calculadoraDisplay == "Erro") return;

    if (calculadoraTemValor && calculadoraOperacao != 0 && !calculadoraNovoNumero) {
      calcularResultadoCalculadora();
    } else {
      calculadoraValor = calculadoraDisplay.toDouble();
    }

    calculadoraOperacao = tecla;
    calculadoraTemValor = true;
    calculadoraNovoNumero = true;
    return;
  }

  if (tecla == '=') {
    if (calculadoraDisplay == "Erro") return;

    if (calculadoraTemValor && calculadoraOperacao != 0) {
      calcularResultadoCalculadora();
      calculadoraOperacao = 0;
      calculadoraTemValor = false;
      calculadoraNovoNumero = true;
    }
  }
}

void processarCalculadoraJoystick() {
  int valorX = lerJoystickAnalogico(PIN_JOY_X);
  int valorY = lerJoystickAnalogico(PIN_JOY_Y);
  int btnClique = digitalRead(PIN_JOY_SW);

  bool movCima = valorX > JOY_CIMA;
  bool movBaixo = valorX < JOY_BAIXO;
  bool movDireita = valorY > JOY_DIREITA;
  bool movEsquerda = valorY < JOY_ESQUERDA;
  bool movendo = movCima || movBaixo || movDireita || movEsquerda;

  if (calculadoraBloquearBotao) {
    if (btnClique == HIGH) {
      calculadoraBloquearBotao = false;
    }
    return;
  }

  if (!movendo) {
    joyEmMovimento = false;
  }

  if (movendo) {
    unsigned long agora = millis();
    int intervalo = joyEmMovimento ? VELOCIDADE_DESLIZE : PAUSA_INICIAL_REPETICAO;

    if (!joyEmMovimento || agora - calculadoraUltimoMovimento >= intervalo) {
      if (movCima && calculadoraY > 0) calculadoraY--;
      else if (movBaixo && calculadoraY < 4) calculadoraY++;
      else if (movEsquerda && calculadoraX > 0) calculadoraX--;
      else if (movDireita && calculadoraX < 3) calculadoraX++;

      calculadoraUltimoMovimento = agora;
      joyEmMovimento = true;
    }
  }

  if (btnClique == LOW) {
    if (!calculadoraBotaoPressionado) {
      calculadoraBotaoPressionado = true;
      calculadoraInicioPressao = millis();
      calculadoraLongPressExecutado = false;
    }

    if (!calculadoraLongPressExecutado &&
        millis() - calculadoraInicioPressao >= 900) {
      calculadoraLongPressExecutado = true;
      calculadoraBotaoPressionado = false;
      emSubMenu = false;
      calculadoraBloquearBotao = true;
      joyEmMovimento = false;
      display.clearDisplay();
      display.display();
      return;
    }
  } else {
    if (calculadoraBotaoPressionado) {
      unsigned long duracaoPressao = millis() - calculadoraInicioPressao;

      if (!calculadoraLongPressExecutado &&
          duracaoPressao < 900 &&
          millis() - ultimoDebounceBotao > 200) {
        ultimoDebounceBotao = millis();

        const char teclas[5][4] = {
          {'C', '<', '%', '/'},
          {'7', '8', '9', '*'},
          {'4', '5', '6', '-'},
          {'1', '2', '3', '+'},
          {'0', '.', '=', 'S'}
        };

        char tecla = teclas[calculadoraY][calculadoraX];

        if (tecla != ' ') {
          if (tecla == 'S') {
            emSubMenu = false;
            calculadoraBloquearBotao = true;
            joyEmMovimento = false;
            display.clearDisplay();
            display.display();
          } else {
            selecionarTeclaCalculadora(tecla);
            joyEmMovimento = false;
          }
        }
      }

      calculadoraBotaoPressionado = false;
      calculadoraLongPressExecutado = false;
    }

    calculadoraBloquearBotao = false;
  }
}

void telaCalculadora() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("CALCULADORA");

  display.setCursor(76, 0);
  if (calculadoraOperacao != 0) {
    display.print(calculadoraOperacao);
  } else {
    display.print("=");
  }

  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

  String valor = calculadoraDisplay;
  if (valor.length() > 18) {
    valor = valor.substring(valor.length() - 18);
  }

  display.setTextSize(2);
  int largura = valor.length() * 12;
  int x = 124 - largura;
  if (x < 0) x = 0;
  display.setCursor(x, 12);
  display.print(valor);

  display.setTextSize(1);

  const char teclas[5][4] = {
    {'C', '<', '%', '/'},
    {'7', '8', '9', '*'},
    {'4', '5', '6', '-'},
    {'1', '2', '3', '+'},
    {'0', '.', '=', 'S'}
  };

  const int larguraTecla = 32;
  const int alturaTecla = 8;
  const int inicioY = 31;

  for (int y = 0; y < 5; y++) {
    for (int xTecla = 0; xTecla < 4; xTecla++) {
      char tecla = teclas[y][xTecla];
      if (tecla == ' ') continue;

      int px = xTecla * larguraTecla;
      int py = inicioY + y * alturaTecla;

      if (xTecla == calculadoraX && y == calculadoraY) {
        display.fillRect(px, py, larguraTecla - 1, alturaTecla - 1, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.drawRect(px, py, larguraTecla - 1, alturaTecla - 1, SSD1306_WHITE);
        display.setTextColor(SSD1306_WHITE);
      }

      if (tecla == 'S') {
        display.setCursor(px + 4, py + 1);
        display.print("SAIR");
      } else {
        display.setCursor(px + 14, py + 1);
        display.print(tecla);
      }
      display.setTextColor(SSD1306_WHITE);
    }
  }

  display.setCursor(0, 59);
  display.print("Clique=OK");

  display.display();
}

void telaSistema() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  uint32_t sketch = ESP.getSketchSize();
  uint32_t freeSketch = ESP.getFreeSketchSpace();
  uint32_t heapTotal = ESP.getHeapSize();
  uint32_t heapFree = ESP.getFreeHeap();
  uint32_t flashTotal = ESP.getFlashChipSize();
  uint32_t ramUsada = heapTotal > heapFree ? heapTotal - heapFree : 0;

  int flashUsadaPct = flashTotal > 0 ? (int)((uint64_t)sketch * 100ULL / flashTotal) : 0;
  int ramUsadaPct = heapTotal > 0 ? (int)((uint64_t)ramUsada * 100ULL / heapTotal) : 0;
  int ramLivrePct = heapTotal > 0 ? (int)((uint64_t)heapFree * 100ULL / heapTotal) : 0;
  int flashLivrePct = flashTotal > 0 ? (int)((uint64_t)freeSketch * 100ULL / flashTotal) : 0;

  display.setCursor(0, 0);
  display.print("MEMORIA ");
  display.print(CODIGO_VERSAO);
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

  display.setCursor(0, 12);
  display.print("FLASH ");
  display.print(sketch / 1024.0f, 0);
  display.print("K ");
  display.print(flashUsadaPct);
  display.print("%");

  display.setCursor(0, 23);
  display.print("F LIVRE ");
  display.print(freeSketch / 1024.0f, 0);
  display.print("K ");
  display.print(flashLivrePct);
  display.print("%");

  display.setCursor(0, 34);
  display.print("RAM ");
  display.print(ramUsada / 1024.0f, 0);
  display.print("/");
  display.print(heapTotal / 1024.0f, 0);
  display.print("K ");
  display.print(ramUsadaPct);
  display.print("%");

  display.setCursor(0, 45);
  display.print("RAM LIVRE ");
  display.print(heapFree / 1024.0f, 0);
  display.print("K ");
  display.print(ramLivrePct);
  display.print("%");

  display.setCursor(0, 56);
  display.print("CHIP ");
  display.print(flashTotal / (1024.0f * 1024.0f), 0);
  display.print("MB");

  display.display();
}
