// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <Wire.h>


// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define CELL_SIZE 8

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,&Wire, -1);



// // Labyrinthe 8 lignes x 16 colonnes
// // 1 = mur, 0 = chemin, 2 = sortie
// const uint8_t maze[8][16] PROGMEM = {
//   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//   {1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1},
//   {1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1},
//   {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1},
//   {1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
//   {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
//   {1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,1},
//   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1},
// };

// void setup() {
//   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//   display.clearDisplay();
//   dessinerLabyrinthe();
//   display.display();
// }

// void loop() {
//   // rien pour l'instant
// }

// void dessinerLabyrinthe() {
//   for (int row = 0; row < 8; row++) {
//     for (int col = 0; col < 16; col++) {
//       uint8_t cell = pgm_read_byte(&maze[row][col]);

//       int x = col * CELL_SIZE;
//       int y = row * CELL_SIZE;

//       if (cell == 1) {
//         // Mur = carré blanc rempli
//         display.fillRect(x, y, CELL_SIZE, CELL_SIZE, WHITE);
//       }
//       else if (cell == 2) {
//         // Sortie = petit triangle
//         display.drawTriangle(
//           x, y + CELL_SIZE,
//           x + CELL_SIZE, y + CELL_SIZE/2,
//           x, y,
//           WHITE
//         );
//       }
//       // 0 = chemin = rien (fond noir)
//     }
//   }
// }

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// Adafruit_SSD1306 display(128, 64, &Wire, -1);

// void setup() {
//   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//   delay(1000);
//   display.clearDisplay();
//   display.setTextSize(2);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 0);
//   display.print("Hello");
//   display.display();
// }

// void loop() {}
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET -1

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
//                           &Wire, OLED_RESET);

// #define CELL_SIZE 8

// // Boutons
// #define BTN_HAUT   2
// #define BTN_BAS    3
// #define BTN_GAUCHE 4
// #define BTN_DROITE 5

// // Joueur
// int playerX = 1;
// int playerY = 1;

// const uint8_t maze[8][16] PROGMEM = {
//   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//   {1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1},
//   {1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,1},
//   {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1},
//   {1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
//   {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
//   {1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,1},
//   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1},
// };

// void setup() {
//   // Boutons en INPUT avec résistances pull-down externes
//   pinMode(BTN_HAUT,   INPUT);
//   pinMode(BTN_BAS,    INPUT);
//   pinMode(BTN_GAUCHE, INPUT);
//   pinMode(BTN_DROITE, INPUT);

//   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//   display.clearDisplay();
//   afficherJeu();
// }

// void loop() {
//   int newX = playerX;
//   int newY = playerY;

//   // HIGH = bouton pressé (pull-down)
//   if (digitalRead(BTN_HAUT)   == HIGH) newY--;
//   if (digitalRead(BTN_BAS)    == HIGH) newY++;
//   if (digitalRead(BTN_GAUCHE) == HIGH) newX--;
//   if (digitalRead(BTN_DROITE) == HIGH) newX++;

//   // Vérifier si la nouvelle case est un mur
//   uint8_t cell = pgm_read_byte(&maze[newY][newX]);
//   if (cell != 1) {
//     playerX = newX;
//     playerY = newY;
//   }

//   afficherJeu();
//   delay(150);
// }

// void afficherJeu() {
//   display.clearDisplay();
//   dessinerLabyrinthe();
//   dessinerJoueur();
//   display.display();
// }

// void dessinerLabyrinthe() {
//   for (int row = 0; row < 8; row++) {
//     for (int col = 0; col < 16; col++) {
//       uint8_t cell = pgm_read_byte(&maze[row][col]);
//       int x = col * CELL_SIZE;
//       int y = row * CELL_SIZE;

//       if (cell == 1) {
//         display.fillRect(x, y, CELL_SIZE, CELL_SIZE, WHITE);
//       }
//       else if (cell == 2) {
//         display.drawTriangle(
//           x, y + CELL_SIZE,
//           x + CELL_SIZE, y + CELL_SIZE/2,
//           x, y,
//           WHITE
//         );
//       }
//     }
//   }
// }

// void dessinerJoueur() {
//   int x = playerX * CELL_SIZE;
//   int y = playerY * CELL_SIZE;
//   display.fillRect(x + 2, y + 2, 4, 4, WHITE);
// }

