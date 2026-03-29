#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <EEPROM.h>

#define BTN_DIR A3
#define BTN_MENU A2

#define MAZE_W 16
#define MAZE_H 8
#define CELL_W 8
#define CELL_H 8

#define SSD1306_SCL PB2
#define SSD1306_SDA PB0
#define SSD1306_ADDR 0x78

#define DIGITAL_WRITE_HIGH(PORT) PORTB |= (1 << PORT)
#define DIGITAL_WRITE_LOW(PORT)  PORTB &= ~(1 << PORT)

#define EEPROM_MAGIC_ADDR 0
#define EEPROM_LEVEL_ADDR 1
#define EEPROM_MAGIC 66

#define STATE_TITLE 0
#define STATE_MENU  1
#define STATE_GAME  2
#define STATE_WIN   3
#define STATE_OVER  4

#define BTN_YELLOW 1
#define BTN_GREEN  2
#define BTN_RED    3
#define BTN_BLUE   4

#define DIR_UP     1
#define DIR_RIGHT  2
#define DIR_DOWN   3
#define DIR_LEFT   4

#define ENEMY_DELAY_MS 5000
#define ENEMY_STEP_MS 120
#define MAX_HISTORY 100

const uint8_t level1[MAZE_H][MAZE_W] PROGMEM = {
  {0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
  {1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1},
  {1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
  {1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0},
  {1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,2}
};

const uint8_t level2[MAZE_H][MAZE_W] PROGMEM = {
  {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0},
  {0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0},
  {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0},
  {1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0},
  {0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0},
  {0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0},
  {0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0},
  {1,1,0,1,1,1,0,1,1,1,1,1,1,0,0,2}
};

uint8_t gameState = STATE_TITLE;
uint8_t currentLevel = 1;
uint8_t px = 0;
uint8_t py = 0;
uint8_t menuIndex = 0;
unsigned long lastInputTime = 0;

uint8_t ex = 0;
uint8_t ey = 0;
uint8_t moveHistory[MAX_HISTORY];
uint8_t historyLen = 0;
uint8_t enemyReadIndex = 0;
bool enemyActive = false;
bool enemyWarningShown = false;
unsigned long levelStartTime = 0;
unsigned long lastEnemyStep = 0;

const uint8_t fontA[5] = {0x7E,0x09,0x09,0x09,0x7E};
const uint8_t fontB[5] = {0x7F,0x49,0x49,0x49,0x36};
const uint8_t fontC[5] = {0x3E,0x41,0x41,0x41,0x22};
const uint8_t fontD[5] = {0x7F,0x41,0x41,0x22,0x1C};
const uint8_t fontE[5] = {0x7F,0x49,0x49,0x49,0x41};
const uint8_t fontF[5] = {0x7F,0x09,0x09,0x09,0x01};
const uint8_t fontG[5] = {0x3E,0x41,0x49,0x49,0x7A};
const uint8_t fontI[5] = {0x00,0x41,0x7F,0x41,0x00};
const uint8_t fontJ[5] = {0x20,0x40,0x41,0x3F,0x01};
const uint8_t fontK[5] = {0x7F,0x08,0x14,0x22,0x41};
const uint8_t fontL[5] = {0x7F,0x40,0x40,0x40,0x40};
const uint8_t fontM[5] = {0x7F,0x02,0x04,0x02,0x7F};
const uint8_t fontN[5] = {0x7F,0x02,0x04,0x08,0x7F};
const uint8_t fontO[5] = {0x3E,0x41,0x41,0x41,0x3E};
const uint8_t fontP[5] = {0x7F,0x09,0x09,0x09,0x06};
const uint8_t fontR[5] = {0x7F,0x09,0x19,0x29,0x46};
const uint8_t fontS[5] = {0x46,0x49,0x49,0x49,0x31};
const uint8_t fontT[5] = {0x01,0x01,0x7F,0x01,0x01};
const uint8_t fontU[5] = {0x3F,0x40,0x40,0x40,0x3F};
const uint8_t fontV[5] = {0x1F,0x20,0x40,0x20,0x1F};
const uint8_t fontW[5] = {0x7F,0x20,0x18,0x20,0x7F};
const uint8_t fontY[5] = {0x03,0x04,0x78,0x04,0x03};
const uint8_t font1[5] = {0x00,0x42,0x7F,0x40,0x00};
const uint8_t font2[5] = {0x62,0x51,0x49,0x49,0x46};
const uint8_t fontGT[5] = {0x00,0x41,0x22,0x14,0x08};
const uint8_t fontSpace[5] = {0,0,0,0,0};

void ssd1306_xfer_start(void) {
  DIGITAL_WRITE_HIGH(SSD1306_SCL);
  DIGITAL_WRITE_HIGH(SSD1306_SDA);
  DIGITAL_WRITE_LOW(SSD1306_SDA);
  DIGITAL_WRITE_LOW(SSD1306_SCL);
}

void ssd1306_xfer_stop(void) {
  DIGITAL_WRITE_LOW(SSD1306_SCL);
  DIGITAL_WRITE_LOW(SSD1306_SDA);
  DIGITAL_WRITE_HIGH(SSD1306_SCL);
  DIGITAL_WRITE_HIGH(SSD1306_SDA);
}

void ssd1306_send_byte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    if ((b << i) & 0x80) DIGITAL_WRITE_HIGH(SSD1306_SDA);
    else DIGITAL_WRITE_LOW(SSD1306_SDA);
    DIGITAL_WRITE_HIGH(SSD1306_SCL);
    DIGITAL_WRITE_LOW(SSD1306_SCL);
  }
  DIGITAL_WRITE_HIGH(SSD1306_SDA);
  DIGITAL_WRITE_HIGH(SSD1306_SCL);
  DIGITAL_WRITE_LOW(SSD1306_SCL);
}

void ssd1306_send_command(uint8_t cmd) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_ADDR);
  ssd1306_send_byte(0x00);
  ssd1306_send_byte(cmd);
  ssd1306_xfer_stop();
}

void ssd1306_send_data_start(void) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_ADDR);
  ssd1306_send_byte(0x40);
}

void ssd1306_send_data_stop(void) {
  ssd1306_xfer_stop();
}

void ssd1306_setpos(uint8_t x, uint8_t page) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_ADDR);
  ssd1306_send_byte(0x00);
  ssd1306_send_byte(0xB0 + page);
  ssd1306_send_byte(((x & 0xF0) >> 4) | 0x10);
  ssd1306_send_byte((x & 0x0F) | 0x01);
  ssd1306_xfer_stop();
}

void ssd1306_init(void) {
  DDRB |= (1 << SSD1306_SDA);
  DDRB |= (1 << SSD1306_SCL);

  ssd1306_send_command(0xAE);
  ssd1306_send_command(0xD5); ssd1306_send_command(0x80);
  ssd1306_send_command(0xA8); ssd1306_send_command(0x3F);
  ssd1306_send_command(0xD3); ssd1306_send_command(0x00);
  ssd1306_send_command(0x40);
  ssd1306_send_command(0x8D); ssd1306_send_command(0x14);
  ssd1306_send_command(0x20); ssd1306_send_command(0x00);
  ssd1306_send_command(0xA1);
  ssd1306_send_command(0xC8);
  ssd1306_send_command(0xDA); ssd1306_send_command(0x12);
  ssd1306_send_command(0x81); ssd1306_send_command(0xCF);
  ssd1306_send_command(0xD9); ssd1306_send_command(0xF1);
  ssd1306_send_command(0xDB); ssd1306_send_command(0x40);
  ssd1306_send_command(0xA4);
  ssd1306_send_command(0xA6);
  ssd1306_send_command(0xAF);
}

void fillScreen(uint8_t value) {
  for (uint8_t page = 0; page < 8; page++) {
    ssd1306_setpos(0, page);
    ssd1306_send_data_start();
    for (uint8_t x = 0; x < 128; x++) ssd1306_send_byte(value);
    ssd1306_send_data_stop();
  }
}

const uint8_t* getGlyph(char c) {
  switch (c) {
    case 'A': return fontA;
    case 'B': return fontB;
    case 'C': return fontC;
    case 'D': return fontD;
    case 'E': return fontE;
    case 'F': return fontF;
    case 'G': return fontG;
    case 'I': return fontI;
    case 'J': return fontJ;
    case 'K': return fontK;
    case 'L': return fontL;
    case 'M': return fontM;
    case 'N': return fontN;
    case 'O': return fontO;
    case 'P': return fontP;
    case 'R': return fontR;
    case 'S': return fontS;
    case 'T': return fontT;
    case 'U': return fontU;
    case 'V': return fontV;
    case 'W': return fontW;
    case 'Y': return fontY;
    case '1': return font1;
    case '2': return font2;
    case '>': return fontGT;
    default:  return fontSpace;
  }
}

void drawChar5x7(char c, uint8_t x, uint8_t page) {
  const uint8_t *g = getGlyph(c);
  ssd1306_setpos(x, page);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < 5; i++) ssd1306_send_byte(g[i]);
  ssd1306_send_byte(0x00);
  ssd1306_send_data_stop();
}

void drawText(const char* txt, uint8_t x, uint8_t page) {
  while (*txt) {
    drawChar5x7(*txt, x, page);
    x += 6;
    txt++;
  }
}

void saveLevel(uint8_t lvl) {
  EEPROM.update(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  EEPROM.update(EEPROM_LEVEL_ADDR, lvl);
}

bool hasSave() {
  return EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC;
}

uint8_t getSavedLevel() {
  if (!hasSave()) return 1;
  uint8_t lvl = EEPROM.read(EEPROM_LEVEL_ADDR);
  if (lvl < 1 || lvl > 2) lvl = 1;
  return lvl;
}

void clearSave() {
  EEPROM.update(EEPROM_MAGIC_ADDR, 0);
  EEPROM.update(EEPROM_LEVEL_ADDR, 1);
}

uint8_t readMenuButton() {
  int val = analogRead(BTN_MENU);
  if (val > 490 && val < 540) return BTN_YELLOW;
  if (val > 590 && val < 640) return BTN_GREEN;
  if (val > 660 && val < 700) return BTN_RED;
  if (val > 710 && val < 750) return BTN_BLUE;
  return 0;
}

uint8_t readDir() {
  int val = analogRead(BTN_DIR);
  if (val > 490 && val < 540) return DIR_UP;
  if (val > 590 && val < 640) return DIR_RIGHT;
  if (val > 650 && val < 710) return DIR_DOWN;
  if (val > 710 && val < 760) return DIR_LEFT;
  return 0;
}

bool inputReady(unsigned long d) {
  if (millis() - lastInputTime < d) return false;
  lastInputTime = millis();
  return true;
}

uint8_t getCell(uint8_t x, uint8_t y) {
  if (currentLevel == 1) return pgm_read_byte(&level1[y][x]);
  return pgm_read_byte(&level2[y][x]);
}

void resetEnemy() {
  ex = 0;
  ey = 0;
  historyLen = 0;
  enemyReadIndex = 0;
  enemyActive = false;
  enemyWarningShown = false;
  levelStartTime = millis();
  lastEnemyStep = millis();
}

void loadLevel(uint8_t lvl) {
  currentLevel = lvl;
  px = 0;
  py = 0;
  saveLevel(lvl);
  resetEnemy();
}

void startNewGame() {
  loadLevel(1);
  gameState = STATE_GAME;
}

void resumeGame() {
  loadLevel(getSavedLevel());
  gameState = STATE_GAME;
}

void showLevelDone(uint8_t lvl) {
  fillScreen(0x00);
  drawText("LEVEL", 34, 2);
  if (lvl == 1) drawText("1", 70, 2);
  else drawText("2", 70, 2);
  drawText("OK", 46, 4);
  delay(1200);
}

void showLevelGo(uint8_t lvl) {
  fillScreen(0x00);
  drawText("LEVEL", 34, 2);
  if (lvl == 1) drawText("1", 70, 2);
  else drawText("2", 70, 2);
  drawText("GO", 46, 4);
  delay(1200);
}

void drawTitleScreen() {
  fillScreen(0x00);
  drawText("LABTTINY", 40, 2);
  drawText("BLEU", 40, 5);
  drawText("POUR", 34, 6);
  drawText("CONTINUER", 20, 7);
}

void drawMenuScreen() {
  fillScreen(0x00);
  drawText("MENU", 44, 1);
  if (menuIndex == 0) drawText(">", 18, 3);
  drawText("JOUER", 34, 3);
  if (menuIndex == 1) drawText(">", 18, 5);
  drawText("REPRENDRE", 34, 5);
}

void drawWinScreen() {
  fillScreen(0x00);
  drawText("BRAVO", 34, 2);
  drawText("FIN", 46, 4);
}

void showEnemyWarning() {
  fillScreen(0x00);
  drawText("ENNEMI", 28, 2);
  drawText("SORTI", 34, 4);
  delay(900);
}

void drawGameOverScreen() {
  fillScreen(0x00);
  drawText("PERDU", 34, 3);
}

void nextLevel() {
  if (currentLevel == 1) {
    showLevelDone(1);
    showLevelGo(2);
    loadLevel(2);
  } else {
    showLevelDone(2);
    clearSave();
    gameState = STATE_WIN;
    drawWinScreen();
  }
}

void triggerGameOver() {
  gameState = STATE_OVER;
  drawGameOverScreen();
  delay(1200);
  gameState = STATE_MENU;
}

void recordMove(uint8_t dir) {
  if (historyLen < MAX_HISTORY) {
    moveHistory[historyLen] = dir;
    historyLen++;
  }
}

void moveEnemyOneStep(uint8_t dir) {
  if (dir == DIR_UP && ey > 0) ey--;
  if (dir == DIR_RIGHT && ex < MAZE_W - 1) ex++;
  if (dir == DIR_DOWN && ey < MAZE_H - 1) ey++;
  if (dir == DIR_LEFT && ex > 0) ex--;
}

void updateEnemy() {
  if (!enemyActive) {
    if (millis() - levelStartTime >= ENEMY_DELAY_MS) {
      enemyActive = true;
      if (!enemyWarningShown) {
        enemyWarningShown = true;
        showEnemyWarning();
      }
    } else {
      return;
    }
  }

  if (enemyReadIndex >= historyLen) return;
  if (millis() - lastEnemyStep < ENEMY_STEP_MS) return;

  lastEnemyStep = millis();
  moveEnemyOneStep(moveHistory[enemyReadIndex]);
  enemyReadIndex++;

  if (ex == px && ey == py) {
    triggerGameOver();
  }
}

void movePlayer(int8_t dx, int8_t dy, uint8_t dirCode) {
  int8_t nx = (int8_t)px + dx;
  int8_t ny = (int8_t)py + dy;
  if (nx < 0 || nx >= MAZE_W || ny < 0 || ny >= MAZE_H) return;

  uint8_t cell = getCell((uint8_t)nx, (uint8_t)ny);

  if (cell != 1) {
    px = (uint8_t)nx;
    py = (uint8_t)ny;
    recordMove(dirCode);
  }

  if (enemyActive && ex == px && ey == py) {
    triggerGameOver();
    return;
  }

  if (cell == 2) {
    nextLevel();
  }
}

uint8_t getPageByte(uint8_t x, uint8_t page) {
  uint8_t byteOut = 0x00;

  for (uint8_t bit = 0; bit < 8; bit++) {
    uint8_t y = page * 8 + bit;
    uint8_t cellX = x / CELL_W;
    uint8_t cellY = y / CELL_H;
    uint8_t pixelOn = 1;
    uint8_t cell = getCell(cellX, cellY);

    if (cell == 1) pixelOn = 0;

    if (cell == 2) {
      uint8_t localX = x % CELL_W;
      uint8_t localY = y % CELL_H;
      if (localX >= 2 && localX <= 5 && localY >= 2 && localY <= 5) pixelOn = 0;
      else pixelOn = 1;
    }

    if (cellX == px && cellY == py) {
      uint8_t localX = x % CELL_W;
      uint8_t localY = y % CELL_H;
      if (localX >= 2 && localX <= 5 && localY >= 2 && localY <= 5) pixelOn = 0;
    }

    if (enemyActive && cellX == ex && cellY == ey) {
      uint8_t localX = x % CELL_W;
      uint8_t localY = y % CELL_H;
      if (localX >= 1 && localX <= 3 && localY >= 1 && localY <= 3) pixelOn = 0;
    }

    if (cellX == 0 && cellY == 0 && x == 0) pixelOn = 1;
    if (cell == 2 && cellX == MAZE_W - 1 && x == 127) pixelOn = 1;

    if (pixelOn) byteOut |= (1 << bit);
  }

  return byteOut;
}

void drawGame() {
  for (uint8_t page = 0; page < 8; page++) {
    ssd1306_setpos(0, page);
    ssd1306_send_data_start();
    for (uint8_t x = 0; x < 128; x++) {
      ssd1306_send_byte(getPageByte(x, page));
    }
    ssd1306_send_data_stop();
  }
}

void setup() {
  pinMode(BTN_DIR, INPUT);
  pinMode(BTN_MENU, INPUT);
  ssd1306_init();
  drawTitleScreen();
}

void loop() {
  uint8_t menuBtn = readMenuButton();
  uint8_t dirBtn = readDir();

  if (gameState == STATE_TITLE) {
    drawTitleScreen();
    if (menuBtn == BTN_RED && inputReady(250)) {
      gameState = STATE_MENU;
    }
    delay(30);
    return;
  }

  if (gameState == STATE_MENU) {
    drawMenuScreen();

    if ((dirBtn == DIR_UP || dirBtn == DIR_DOWN) && inputReady(180)) {
      menuIndex = 1 - menuIndex;
    }

    if (menuBtn == BTN_RED && inputReady(250)) {
      if (menuIndex == 0) startNewGame();
      else resumeGame();
    }

    delay(30);
    return;
  }

  if (gameState == STATE_GAME) {
    if (dirBtn == DIR_UP && inputReady(90)) movePlayer(0, -1, DIR_UP);
    if (dirBtn == DIR_RIGHT && inputReady(90)) movePlayer(1, 0, DIR_RIGHT);
    if (dirBtn == DIR_DOWN && inputReady(90)) movePlayer(0, 1, DIR_DOWN);
    if (dirBtn == DIR_LEFT && inputReady(90)) movePlayer(-1, 0, DIR_LEFT);

    if (gameState == STATE_GAME) updateEnemy();
    if (gameState == STATE_GAME) drawGame();

    delay(20);
    return;
  }

  if (gameState == STATE_WIN) {
    drawWinScreen();
    if (menuBtn == BTN_RED && inputReady(250)) startNewGame();
    delay(30);
    return;
  }

  if (gameState == STATE_OVER) {
    delay(30);
    return;
  }
} 
