 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <EEPROM.h>


#define largeur 16
#define hauteur 8
#define largeur_pixel 8
#define hauteur_pixel 8
#define I2C_SCL PB2
#define I2C_SDA PB0
#define SSD1306_ADDR 0x78
#define MPU6050_ADDR_W 0xD0
#define MPU6050_ADDR_R 0xD1
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_LEVEL_ADDR 1
#define EEPROM_MAGIC 66
#define menu_labttiny 0
#define menu_principal 1
#define menu_mode 2
#define jeu 3
#define victoire 4
#define echec 5
#define btn_direction A3
#define btn_menu A2
#define btn_jaune 1
#define btn_vert 2
#define btn_rouge 3
#define btn_bleu 4
#define btn_haut 1
#define btn_droite 2
#define btn_bas 3
#define btn_gauche 4
#define CONTROL_BUTTON 0
#define CONTROL_MPU    1
#define arrivee_ennemi 5000
#define vitesse_ennemi 170
#define deplacement 100 //c'est la matrice qui stocke le deplacement du joueur pour que l'ennemi les refait. on s'est dit vu que en pratique yaura pas plus de 100 deplacement, ca prendra pas trop despace sur la ram de lattiny
#define seuil_mpu 9000


//on a defini les matrices des labyrinthes de chaque niveau : 0 represente le blanc et 1 represente le noir aka les murs
const uint8_t level1[hauteur][largeur] PROGMEM = {
  {0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
  {1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1},
  {1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
  {1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0},
  {1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,2}
};

const uint8_t level2[hauteur][largeur] PROGMEM = {
  {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0},
  {0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0},
  {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0},
  {1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0},
  {0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0},
  {0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0},
  {0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0},
  {1,1,0,1,1,1,0,1,1,1,1,1,1,0,0,2}
};

uint8_t jx = 0; //coordonnees du joueur
uint8_t jy = 0;
uint8_t ex = 0;// coordonnees du joueur. 
uint8_t ey = 0;

uint8_t etat = menu_labttiny;
uint8_t level_actuel = 1;
uint8_t menuIndex = 0;
uint8_t modeIndex = 0;
uint8_t controlMode = CONTROL_BUTTON;
unsigned long lastInputTime = 0;

uint8_t pas_ennemi[deplacement];
uint8_t nbr_move = 0;
uint8_t enemyReadIndex = 0;
bool enemyActive = false;
bool enemyWarningShown = false;
unsigned long levelStartTime = 0;
unsigned long lastEnemyStep = 0;
 
// on a essaye de faire avec des librairie de texte mais le texte ne s'affichait ps sur l'ecran et il yavait des symboles bizarres. on a vu qlors qu'on 
//pouvait definir chaque lettre manuellement, et uniquement les lettres qui nous interessait. economie de mémoire!!! Chaque octet représente une colonne verticale de pixels (bit 0 = haut, bit 6 = bas)
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

//protocol i2c: on a constate que les librairie tiny4OLED et mpu sont tres grandes et l'ecran ne s'allumait pas. la solution: rediger les registres i2c à la main .... 
void i2c_sda_high() {
   DDRB &= ~(1 << I2C_SDA); //Configure la pin comme entrée
   PORTB |= (1 << I2C_SDA);//Active la résistance de pull-up interne, la ligne monte à HIGH grâce au pull-up. 
    }


void i2c_sda_low()  {
  
   DDRB |=  (1 << I2C_SDA); //Configure la pin comme sortie.
   PORTB &= ~(1 << I2C_SDA);// pull down, valeur 0.
   
    }
void i2c_scl_high() {
  
   DDRB &= ~(1 << I2C_SCL);
    PORTB |= (1 << I2C_SCL); } // meme logique 



void i2c_scl_low()  {
  
   DDRB |=  (1 << I2C_SCL); 
   PORTB &= ~(1 << I2C_SCL); }

uint8_t i2c_read_sda() { 
  return (PINB & (1 << I2C_SDA)) ? 1 : 0; } //lecture des etats. Utilisé pour lire les données et les acquittements (ACK).

void i2c_delay() { 
  
  delayMicroseconds(4);
  
   }

void i2c_start() {
  i2c_sda_high();
  i2c_scl_high();
  i2c_delay();
  i2c_sda_low();
  i2c_delay();
  i2c_scl_low();
}

void i2c_stop() {
  i2c_sda_low();
  i2c_delay();
  i2c_scl_high();
  i2c_delay();
  i2c_sda_high();
  i2c_delay();
}

bool i2c_write_byte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {

    if (b & 0x80) i2c_sda_high();

    else i2c_sda_low();

    i2c_delay();
    i2c_scl_high();
    i2c_delay();
    i2c_scl_low();
    i2c_delay();
    b <<= 1;
  }

  i2c_sda_high();
  i2c_delay();
  i2c_scl_high();
  i2c_delay();
  bool ack = (i2c_read_sda()==0);
  i2c_scl_low();
  i2c_delay();
  return ack;
}

uint8_t i2c_read_byte(bool ack) {
  uint8_t b = 0;
  i2c_sda_high();

  for (uint8_t i = 0; i < 8; i++) {
    b <<= 1;
    i2c_scl_high();
    i2c_delay();
    if (i2c_read_sda()) b |= 1;
    i2c_scl_low();
    i2c_delay();
  }

  if (ack) i2c_sda_low();
  else i2c_sda_high();

  i2c_delay();
  i2c_scl_high();
  i2c_delay();
  i2c_scl_low();
  i2c_delay();
  i2c_sda_high();

  return b;
}
//ecriture et lecture des bits.

void ssd1306_send_command(uint8_t cmd) {
  i2c_start();
  i2c_write_byte(SSD1306_ADDR);
  i2c_write_byte(0x00);
  i2c_write_byte(cmd);
  i2c_stop();
}
//cette partie est pour la configuration de la communication avec l'ecran. les fonctions ci dessous sont utilise pour envoyer des commandes, positionner le cursseu
//initialiser l'ecran et le remplir. 
void ssd1306_send_data_start(void) {
  i2c_start();
  i2c_write_byte(SSD1306_ADDR);
  i2c_write_byte(0x40);
}

void ssd1306_send_data_stop(void) {
  i2c_stop();
}

void ssd1306_setpos(uint8_t x, uint8_t page) {
  i2c_start();
  i2c_write_byte(SSD1306_ADDR);
  i2c_write_byte(0x00);
  i2c_write_byte(0xB0 + page);
  i2c_write_byte(((x & 0xF0) >> 4) | 0x10);
  i2c_write_byte((x & 0x0F) | 0x01);
  i2c_stop();
}

void ssd1306_init(void) {
  i2c_sda_high();
  i2c_scl_high();

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
    for (uint8_t x = 0; x < 128; x++) i2c_write_byte(value);
    ssd1306_send_data_stop();
  }
}

//comm MPU
void mpuWriteReg(uint8_t reg, uint8_t value) {
  i2c_start();
  i2c_write_byte(MPU6050_ADDR_W);
  i2c_write_byte(reg);
  i2c_write_byte(value);
  i2c_stop();
}

uint8_t mpuReadReg(uint8_t reg) {
  i2c_start();
  i2c_write_byte(MPU6050_ADDR_W);
  i2c_write_byte(reg);
  i2c_start();
  i2c_write_byte(MPU6050_ADDR_R);
  uint8_t v = i2c_read_byte(false);
  i2c_stop();
  return v;
}

int16_t mpuRead16(uint8_t reg) {
  i2c_start();
  i2c_write_byte(MPU6050_ADDR_W);
  i2c_write_byte(reg);
  i2c_start();
  i2c_write_byte(MPU6050_ADDR_R);
  uint8_t hi = i2c_read_byte(true);
  uint8_t lo = i2c_read_byte(false);
  i2c_stop();
  return (int16_t)((hi << 8) | lo);
}

void mpuInit() {
  delay(100);
  mpuWriteReg(0x6B, 0x00); // wake up
  mpuWriteReg(0x1C, 0x00); // accel ±2g
  mpuWriteReg(0x1B, 0x00); // gyro ±250°/s
  mpuWriteReg(0x1A, 0x06); // low pass filter
}

uint8_t readMpuDir() {
  int16_t ax = mpuRead16(0x3B);
  int16_t ay = mpuRead16(0x3D);

  if (ax > seuil_mpu) return btn_droite;
  if (ax < -seuil_mpu) return btn_gauche;
  if (ay > seuil_mpu) return btn_bas;
  if (ay < -seuil_mpu) return btn_haut;
  return 0;
}




const uint8_t* getGlyph(char c) { //ce pointeur pointe vers le tableau des lettres utilise en haut. cette methode a ete vu sur stackoverflow. 
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
  for (uint8_t i = 0; i < 5; i++) i2c_write_byte(g[i]);
  i2c_write_byte(0x00);
  ssd1306_send_data_stop();
}

void drawText(const char* txt, uint8_t x, uint8_t page) {
  while (*txt) {
    drawChar5x7(*txt, x, page);
    x += 6;
    txt++;
  }
}


//on vouilait sauvegrader l'etat actuel du niveau dans la memoire eeprom
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


//lecture des valeurs analogique des boutons et determination d'une plage de tensions
uint8_t readMenuButton() {
  int val = analogRead(btn_menu);
  if (val > 490 && val < 540) return btn_jaune;
  if (val > 590 && val < 640) return btn_vert;
  if (val > 660 && val < 700) return btn_rouge;
  if (val > 710 && val < 760) return btn_bleu;
  return 0;
}

uint8_t readDirButtons() {
  int val = analogRead(btn_direction);
  if (val > 490 && val < 540) return btn_haut;
  if (val > 590 && val < 640) return btn_droite;
  if (val > 650 && val < 710) return btn_bas;
  if (val > 710 && val < 760) return btn_gauche;
  return 0;
}


// retourne true seulement si au moins d temps_ecouleecondes se sont écoulées depuis la dernière entrée acceptée. Évite les doubles appuis et le déplacement trop rapide.
bool inputReady(unsigned long d) {
  if (temps_ecoule() - lastInputTime < d) return false;
  lastInputTime = temps_ecoule();
  return true;
}

//acces au labyrinthe car les donnes sont stockes dans la memoire flash donc on doit faire une fonction qui y accede/
uint8_t getCell(uint8_t x, uint8_t y) {
  if (level_actuel == 1) return pgm_read_byte(&level1[y][x]);
  return pgm_read_byte(&level2[y][x]);
}


void resetEnemy() {
  ex = 0;
  ey = 0;
  nbr_move = 0;
  enemyReadIndex = 0;
  enemyActive = false;
  enemyWarningShown = false;
  levelStartTime = temps_ecoule();
  lastEnemyStep = temps_ecoule();
}

void loadLevel(uint8_t lvl) {
  level_actuel = lvl;
  jx = 0;
  jy = 0;
  saveLevel(lvl);
  resetEnemy();
}

void startNewGame() {
  loadLevel(1);
  etat = jeu;
}

void resumeGame() {
  loadLevel(getSavedLevel());
  controlMode = CONTROL_BUTTON;
  etat = jeu;
}
//ecriture des textes des menus.
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

void ecran_debut() {
  fillScreen(0x00);
  drawText("LABTTINY", 40, 2);
  drawText("BLEU", 40, 5);
  drawText("POUR", 34, 6);
  drawText("CONTINUER", 20, 7);
}

void ecran_menu() {
  fillScreen(0x00);
  drawText("MENU", 44, 1);
  if (menuIndex == 0) drawText(">", 18, 3);
  drawText("JOUER", 34, 3);
  if (menuIndex == 1) drawText(">", 18, 5);
  drawText("REPRENDRE", 34, 5);
}

void drawModeScreen() {
  fillScreen(0x00);
  drawText("MODE", 44, 1);
  if (modeIndex == 0) drawText(">", 18, 3);
  drawText("BOUTON", 28, 3);
  if (modeIndex == 1) drawText(">", 18, 5);
  drawText("MPU", 46, 5);
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

//logique des niveaux: qd le joueur atteint la sortie (case 2 dans la matrice du labyrinthe: si il etait au niveau 1, il passe au niveau 2, sil est au niveau 2,
//il est dans l'etat victoire.
void nextLevel() {
  if (level_actuel == 1) {
    showLevelDone(1);
    showLevelGo(2);
    loadLevel(2);
  } else {
    showLevelDone(2);
    clearSave();
    etat = victoire;
    drawWinScreen();
  }
}

void triggerGameOver() {
  etat = echec;
  drawGameOverScreen();
  delay(1200);
  etat = menu_principal;
}


//enregistrement des pas du joeur
void recordMove(uint8_t dir) {
  if (nbr_move < deplacement) {
    pas_ennemi[nbr_move++] = dir;
  }
}

//l'ennemi va utiliser dir pour se deplacer.
void moveEnemyOneStep(uint8_t dir) {
  if (dir == btn_haut && ey > 0) ey--;
  if (dir == btn_droite && ex < largeur - 1) ex++;
  if (dir == btn_bas && ey < hauteur - 1) ey++;
  if (dir == btn_gauche && ex > 0) ex--;
}

//Pendant les 5 premières secondes ,  l'ennemi est inactif.
//Après 5 secondes , l'ennemi s'active,  "ENNEMI SORTI" avec la fonction show ennemy warning.
void updateEnemy() {
  if (!enemyActive) {
    if (temps_ecoule() - levelStartTime >= arrivee_ennemi) { //si 5 secondes se sont ecoule depuis le start du niveau et le temps actuel, l'eenemi sort de sa caverne
      enemyActive = true;
      if (!enemyWarningShown) { ,//on a ete oblige de mettre cette condition qui evite que le emssage ennemi sorti s'affiche plusieurs fois. dans nos test il a ete affiche chaque 5 secondes
      //donc on a fait cette condition
        enemyWarningShown = true;
        showEnemyWarning();
      }
    } else {
      return;
    }
  }

  if (enemyReadIndex >= nbr_move) return;
  if (temps_ecoule() - lastEnemyStep < vitesse_ennemi) return;

  lastEnemyStep = temps_ecoule();
  moveEnemyOneStep(pas_ennemi[enemyReadIndex]); //On déplace l'ennemi d'un pas dans la direction stockée à l'index enemyReadIndex du tableau pas_ennemi[]
  enemyReadIndex++;

  if (ex == jx && ey == jy) triggerGameOver();
}
 // Calcule la nouvelle position en ajoutant le déplacement (dx, dy).
//Vérifie les limites du terrain.
//Vérifie si c'est un mur (cell == 1) : si oui, le joueur ne bouge pas.


void movePlayer(int8_t dx, int8_t dy, uint8_t dirCode) {
  int8_t nx = (int8_t)jx + dx;
  int8_t ny = (int8_t)jy + dy;
  if (nx < 0 || nx >= largeur || ny < 0 || ny >= hauteur) return;

  uint8_t cell = getCell((uint8_t)nx, (uint8_t)ny);

  if (cell != 1) {   //le joueur se deplace vu que cest pas un mur
    jx = (uint8_t)nx;
    jy = (uint8_t)ny;
    recordMove(dirCode); 
  }

  if (enemyActive && ex == jx && ey == jy) {
    triggerGameOver(); 
    return;
  } //si les coordoones de l'ennemi et du joueur sont les memes = partie perdue

  if (cell == 2) nextLevel(); // victoire ou passage au niveau suivant car il a atteinr la sortie.
}



//L'écran SSD1306 stocke les pixels en colonnes de 8 bits (une "page"). Cette fonction calcule un octet (8 pixels verticaux) pour une colonne x donnée dans une page donnée.
//Calcule les coordonnées pixel (x, y) et détermine à quelle cellule du labyrinthe ce pixel appartient

uint8_t getPageByte(uint8_t x, uint8_t page) {
  uint8_t byteOut = 0x00;

  for (uint8_t bit = 0; bit < 8; bit++) {
    uint8_t y = page * 8 + bit;
    uint8_t cellX = x / largeur_pixel;
    uint8_t cellY = y / hauteur_pixel;
    uint8_t pixelOn = 1;
    uint8_t cell = getCell(cellX, cellY);

    if (cell == 1) pixelOn = 0;

    if (cell == 2) {
      uint8_t localX = x % largeur_pixel;
      uint8_t localY = y % hauteur_pixel;
      if (localX >= 2 && localX <= 5 && localY >= 2 && localY <= 5) pixelOn = 0;
      else pixelOn = 1;
    }

    if (cellX == jx && cellY == jy) {
      uint8_t localX = x % largeur_pixel;
      uint8_t localY = y % hauteur_pixel;
      if (localX >= 2 && localX <= 5 && localY >= 2 && localY <= 5) pixelOn = 0;
    }

    if (enemyActive && cellX == ex && cellY == ey) {
      uint8_t localX = x % largeur_pixel;
      uint8_t localY = y % hauteur_pixel;
      if (localX >= 1 && localX <= 3 && localY >= 1 && localY <= 3) pixelOn = 0;
    }

    if (pixelOn) byteOut |= (1 << bit);
  }

  return byteOut;
}

//on dessine le jeu
void drawGame() {
  for (uint8_t page = 0; page < 8; page++) {
    ssd1306_setpos(0, page);
    ssd1306_send_data_start();
    for (uint8_t x = 0; x < 128; x++) {
      i2c_write_byte(getPageByte(x, page));
    }
    ssd1306_send_data_stop();
  }
}
//on configure les broches des boutons en input et on inituaise lecran et le mpu et on affiche le menu de demarrage.
void setup() {
  pinMode(btn_direction, INPUT);
  pinMode(btn_menu, INPUT);
  ssd1306_init();
  mpuInit();
  ecran_debut();
}

//À chaque itération, lit les deux groupes de boutons.
void loop() {
  uint8_t menuBtn = readMenuButton();
  uint8_t dirBtn = readDirButtons();


//gere le lancement de chaque etat
  if (etat == menu_labttiny) {
    ecran_debut();
    if ((menuBtn == btn_bleu || menuBtn == btn_rouge) && inputReady(250)) { //enfaite a un moment, qd on appuyait sur le bouton bleu ou rouge et ca lancais
    //la deuxieme page du menu donc on savait pas quoi mettre car ca depend de la lecture des valeur analogique donc on a decide de faire les 2 in case.
      etat = menu_principal;
    }
    delay(30);
    return;
  }

  if (etat == menu_principal) {
    ecran_menu();

    if ((dirBtn == btn_haut || dirBtn == btn_bas) && inputReady(180)) {
      menuIndex = 1 - menuIndex; //on deplacer le curseur.
    }

    if (menuBtn == btn_rouge && inputReady(250)) {
      if (menuIndex == 0) {
        etat = menu_mode;
      } else {
        resumeGame();
      }
    }

    delay(30);
    return;
  }

  if (etat == menu_mode) {
    drawModeScreen();

    if ((dirBtn == btn_haut || dirBtn == btn_bas) && inputReady(180)) {
      modeIndex = 1 - modeIndex;
    }

    if (menuBtn == btn_rouge && inputReady(250)) {
      controlMode = (modeIndex == 0) ? CONTROL_BUTTON : CONTROL_MPU;
      startNewGame();
    }

    delay(30);
    return;
  }

  if (etat == jeu) {
    uint8_t dir = 0;

    if (controlMode == CONTROL_BUTTON) {
      dir = readDirButtons();
    } else {
      dir = readMpuDir();
    }

    if (dir == btn_haut && inputReady(90)) movePlayer(0, -1, btn_haut);
    if (dir == btn_droite && inputReady(90)) movePlayer(1, 0, btn_droite);
    if (dir == btn_bas && inputReady(90)) movePlayer(0, 1, btn_bas);
    if (dir == btn_gauche && inputReady(90)) movePlayer(-1, 0, btn_gauche);

    if (etat == jeu) updateEnemy();
    if (etat == jeu) drawGame();

    delay(20);
    return;
  }

  if (etat == victoire) {
    drawWinScreen();
    if (menuBtn == btn_rouge && inputReady(250)) etat = menu_principal;
    delay(30);
    return;
  }

  if (etat == echec) {
    delay(30);
    return;
  }
}

//on a durement appris que chaque tableau necessitait un index ou un curseur. et le curseur etait lier a une variable. 
//on a aussi appris que faire une fonction pour TOUT et absolument TOUT est une bonne idee pour mettre nos idees au clair. 

