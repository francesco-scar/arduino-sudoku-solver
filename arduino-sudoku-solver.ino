// Including the librarys, see the versions I used to solve back-compatibility problems

#include <Adafruit_GFX.h>             // I used version 1.5.3
#include <Adafruit_TFTLCD.h>          // At the time I wrote this code it doesn't have a proper version I think, anyway see https://github.com/adafruit/TFTLCD-Library and good luck ;P
#include <TouchScreen.h>              // I used version 1.0.4

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#define YP A3
#define XM A2 
#define YM 9
#define XP 8

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

#define  BLACK   0x0000
#define BLUE    0x00FF
#define WHITE   0xFFFF        // RGB and bright
#define RED   0xF00F
#define GREEN 0x0F0F

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define PENRADIUS 3
#define MINPRESSURE 10
#define MAXPRESSURE 1000

int count = 0;

/*int TS_MINX = 311;
int TS_MINY = 155;
int TS_MAXX = 828;
int TS_MAXY = 924;*/
int TS_MINX = 300;
int TS_MINY = 145;
int TS_MAXX = 840;
int TS_MAXY = 930;

int selectedX = 0;
int selectedY = 0;

int sudoku[9][9] = {              // The sudoku is rappresented by this matrix
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0}
};
/*
int defaultSudoku[9][9] = {
{5, 3, 0, 0, 7, 0, 0, 0, 0},
{6, 0, 0, 1, 9, 5, 0, 0, 0},
{0, 9, 8, 0, 0, 0, 0, 6, 0},
{8, 0, 0, 0, 6, 0, 0, 0, 3},
{4, 0, 0, 8, 0, 3, 0, 0, 1},
{7, 0, 0, 0, 2, 0, 0, 0, 6},
{0, 6, 0, 0, 0, 0, 2, 8, 0},
{0, 0, 0, 4, 1, 9, 0, 0, 5},
{0, 0, 0, 0, 8, 0, 0, 7, 9}  
};
*/
int defaultSudoku[9][9] = {         // A grid to test functionality
  {5, 6, 0, 8, 4, 7, 0, 0, 0},
  {3, 0, 9, 0, 0, 0, 6, 0, 0},
  {0, 0, 8, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 8, 0, 0, 4, 0},
  {7, 9, 0, 6, 0, 2, 0, 1, 8},
  {0, 5, 0, 0, 3, 0, 0, 9, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0},
  {0, 0, 6, 0, 0, 0, 8, 0, 7},
  {0, 0, 0, 3, 1, 6, 0, 5, 9}
};

int setted[81];

void setup(void) {
  Serial.begin(9600);

  for (int i=0; i<81; i++){
    setted[i] = -1;
  }
/*  initial_setted(sudoku, setted);
  Serial.println(String(setted[0])+String(setted[1])+String(setted[2])+String(setted[3]));*/

  tft.reset();

  uint16_t identifier = 0x9341;

  tft.begin(identifier);
  pinMode(13, OUTPUT);
  tft.fillScreen(BLACK);                // The background is black

  tft.setTextSize(5);                   // Display title before beginning
  tft.setTextColor(BLUE);
  tft.setCursor(30, 100);
  tft.println("SUDOKU");
  tft.setCursor(30, 150);
  tft.println("SOLVER");
  
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(100, 280);
  tft.println("BY");
  tft.setCursor(10, 300);
  tft.println("S****** Francesco");

  delay(3000);

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  tft.setTextSize(3);                   // Countdown before starting
  tft.setTextColor(RED);
  tft.setCursor(50, 230);
  tft.println("3 ");
  delay(1000);
  tft.setCursor(90, 230);
  tft.println("2 ");
  delay(1000);
  tft.setCursor(130, 230);
  tft.println("1 ");
  delay(1000);
  tft.setCursor(160, 230);
  tft.println(". ");
  delay(200);
  tft.setCursor(170, 230);
  tft.println(". ");
  delay(200);
  tft.setCursor(180, 230);
  tft.println(". ");
  delay(200);
  
  tft.fillScreen(BLACK);                // Clear the screen
  
  grid();                               // Draw an empty sudoku grid
  option();                             // Draw "buttons" to insert number and to start solving
}

void loop()
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);


  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {           // Check if a touch is detected
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);             // Rescale x touch coordinate
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);             // Rescale y touch coordinate
    if(p.y > 76){
      if (selectedX == p.x*9/240 && selectedY == (p.y-76)*9/(320-76) && sudoku[selectedX][selectedY] != 0){
        sudoku[selectedX][selectedY] = 0;
        tft.fillRect(selectedX*240/9+10, selectedY*240/9+84, 16, 16, BLACK);
      } else {
        selectedX = p.x*9/240;
        selectedY = (p.y-76)*9/(320-76);
      }
    } else if (p.y > 36) {
      int val = (p.x*5/240) + 5;
      setNumber(selectedX, selectedY, val);
      sudoku[selectedX][selectedY] = val;
    } else {
      int val = ((p.x*5/240) + 1)%5;
      if (val != 0){
        setNumber(selectedX, selectedY, val);
        sudoku[selectedX][selectedY] = val;
      } else if (val == 0){
        initial_setted(sudoku, setted);
        solve(sudoku, setted);
      }
    }
  }

  if (sudoku[0][0] == 3 && sudoku[0][1] == 3 && sudoku[0][2] == 3){
    Serial.println("Sudoku Default");
    initial_setted(defaultSudoku, setted);
    tft.fillScreen(BLACK);
    grid();
    option();
    int i = 0;
    while (setted[i] != -1){
      setNumber(setted[i]/9, setted[i]%9, defaultSudoku[setted[i]/9][setted[i]%9]);
      i++;
    }
    for(i=0;i<81; i++){
      sudoku[i/9][i%9] = defaultSudoku[i/9][i%9];
    }
//    printSudoku(sudoku);
  }
}


void grid(){
  int j = 0;
  for(int y=76; y<320; y+=26){
    tft.drawFastHLine(0, y, 240, WHITE);
    if (j%3 == 0){
      y++;
      tft.drawFastHLine(0, y, 240, WHITE);
      y++;
      tft.drawFastHLine(0, y, 240, WHITE);
    }
    j++;
  }
  
  j = 0;
  for(int x=0; x<240; x+=25){
    tft.drawFastVLine(x, 76, 240, WHITE);
    if (j%3 == 0){
      x++;
      tft.drawFastVLine(x, 76, 240, WHITE);
      x++;
      tft.drawFastVLine(x, 76, 240, WHITE);
      x+=2;
    }
    j++;
  }
}

void option(){
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.setCursor(5, 5);
  tft.println("1  2  3  4  R");
  tft.setCursor(5, 50);
  tft.println("5  6  7  8  9");
}


void setNumber (int x, int y, int val){
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(x*240/9+10, y*240/9+84);
  tft.println(val);
}

void solutionNumber (int x, int y, int val){
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(x*240/9+10, y*240/9+84);
  tft.println(val);
}

void initial_setted(int grid[9][9], int *result){
  int j = 0;
  for (int i = 0; i < 81; i++){
    if (grid[i/9][i%9] != 0){
      result[j] = i;
      j++;
    }
  }
}


int check(int grid[9][9], int x, int y) {
  int val = grid[x][y];
  for (int c = 0; c<9; c++){                                        // Check row
    if (grid[x][c] == val && c != y){
      return x*9 + c;
    }
  }

  for (int c = 0; c < 9; c++){                                        // Check column
    if (grid[c][y] == val && c != x){
      return c*9+y;
    }
  }

  for (int i = floor(x/3)*3; i<floor(x/3)*3 + 3; i++){
    for (int j = floor(y/3)*3; j<floor(y/3)*3 + 3; j++){
      if(grid[i][j] == val && (i != x || j != y)){
        return i*9+j;
      }
    }
  }

  return -1;                                                        // Return -1 if the grid is OK!
}


void solve(int sudoku[9][9], int *initial){
//  printSudoku(sudoku);
  Serial.println("Solve");
  int current_cell = 0;
  unsigned long t = millis();
  while (current_cell < 81){
//    Serial.println(current_cell);
    if (in_array(current_cell, initial)){
      current_cell++;
    } else {
      sudoku[current_cell/9][current_cell%9]++;
      if (sudoku[current_cell/9][current_cell%9] == 10){
  //      echo $current_cell."  ".$sudoku[$current_cell]."<br>";
        sudoku[current_cell/9][current_cell%9] = 0;
        do {
          current_cell--;
        } while (in_array(current_cell, initial) && current_cell >= 0);
  
        if (current_cell < 0) {
          Serial.println("Not Solvable");
          tft.setTextSize(2);
          tft.setTextColor(GREEN);
          tft.setCursor(40, 50);
          tft.println("NON RISOLVIBILE");
          break;
        }
      } else if (check(sudoku, current_cell/9, current_cell%9) == -1){
          current_cell++;
      }
    }
  }
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(90, 30);
  tft.println(String(millis()-t)+" ms");
  
  printSudoku(sudoku);
}

bool in_array(int n, int *Array){
  for (int i=0; Array[i]<=n && i<81; i++){
    if(Array[i] == n){
      return 1;
    }
  }
  return 0;
}

void printSudoku(int sudoku[9][9]){
  for (int y=0; y<9; y++){
    for (int x=0; x<9; x++){
      if (!in_array(x*9+y, setted)){
        solutionNumber(x, y, sudoku[x][y]);
        delay(30);
      }
      Serial.print(sudoku[x][y]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
}
