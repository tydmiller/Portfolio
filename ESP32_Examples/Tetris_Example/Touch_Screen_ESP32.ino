/*Author: Tyler Miller
* Date: 9/4/2026
* Description: This is an example of how to use a
* touch screen with a ESP32-C6 DevKitC-1. In this example I have created
* a tetris-like game. If you use a different screen there are
* several modifications you will have to make (I had to make
* a lot of changes because originally I had the ESP32 installed 
* with the screen). Due to the initialization of screens being
* different I put them at the bottom of the this code.
* Here are some places that will need to be changed if a
* different screen is used:
* CELL_SIZE
* Arduino_GFX *gfx = new Arduino_ST7796();
* gfx->setRotation()
* gfx->invertDisplay()
*/

#include <lvgl.h>
#include <TouchDrv.hpp>
#include <Arduino_GFX_Library.h>

#include <Wire.h>
#include <SPI.h>

#define GFX_BL 21

//for touch
#define Touch_I2C_SDA 6
#define Touch_I2C_SCL 7
#define Touch_RST 1
#define Touch_INT 2

//for screen
#define LCD_CS	  5
#define LCD_RS	  4
#define LCD_RST	  3
#define LCD_SCK	  20
#define LCD_MOSI  19
#define LCD_BL	  21

//Grid size
#define GRID_COLS 10
#define GRID_ROWS 20
#define CELL_SIZE 24

SPIClass *spi = &SPI;

Arduino_DataBus *bus = new Arduino_HWSPI(
	4,		// DC
	5,		// CS
	20,		// SCK
	19,		// MOSI
	18		// MISO
);

Arduino_GFX *gfx = new Arduino_ST7796(
	bus,
	3,		// RST
	0,		// Rotation
	true,	// IPS
	320,
	480
);

TouchDrvFT6X36 touch;

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

// LVGL variables
uint32_t screenWidth, screenHeight, bufSize;
lv_disp_draw_buf_t draw_buf;
lv_color_t* disp_draw_buf;
lv_disp_drv_t disp_drv;


lv_obj_t* tetris_container;

lv_obj_t* canvas;
lv_color_t* canvas_buf;

lv_color_t grid[GRID_ROWS][GRID_COLS]; // simple color grid
int currentPiece[4][2]; // x,y positions of 4 blocks
//the highlighted placement
int ghostPiece[4][2];
lv_color_t currentColor;
lv_color_t lastGrid[GRID_ROWS][GRID_COLS]; // for diff updates

//FLAGS
bool needsRedraw = true;

//Labels
lv_obj_t *gameOverLabel;
lv_obj_t *scoreLabel;

#define NUM_PIECES 7

// Each piece has 4 blocks (x,y)
const int pieces[NUM_PIECES][4][2] =
{
	// O
	{{0,0},{1,0},{0,1},{1,1}},
	// I
	{{-1,0},{0,0},{1,0},{2,0}},
	// T
	{{-1,0},{0,0},{1,0},{0,1}},
	// L
	{{-1,0},{0,0},{1,0},{1,1}},
	// J
	{{-1,0},{0,0},{1,0},{-1,1}},
	// S
	{{0,0},{1,0},{-1,1},{0,1}},
	// Z
	{{-1,0},{0,0},{0,1},{1,1}}
};

lv_color_t pieceColors[NUM_PIECES] =
{
	LV_COLOR_MAKE(255,255,0), // O - Yellow
	LV_COLOR_MAKE(0,255,255), // I - Cyan
	LV_COLOR_MAKE(128,0,128), // T - Purple
	LV_COLOR_MAKE(0,0,255),	  // L - Orange
	LV_COLOR_MAKE(255,165,0), // J - Blue
	LV_COLOR_MAKE(0,255,0),	  // S - Green
	LV_COLOR_MAKE(255,0,0)	  // Z - Red
};

//make sure that all pieces are chosen from
int bag[NUM_PIECES];
int bagIndex = 0;
int currentType = 0;

enum GameState
{
	STATE_MENU,
	STATE_PLAYING,
	STATE_GAMEOVER
};

GameState gameState = STATE_PLAYING;
unsigned long lastInput = 0;
const unsigned long inputDelay = 100;
int score = 0;

unsigned long lockStart = 0;
const unsigned long LOCK_DELAY = 300;

void resetGame()
{
	lv_obj_add_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN);

	for(int r = 0; r < GRID_ROWS; r++)
	{
		for(int c = 0; c < GRID_COLS; c++)
		{
			grid[r][c] = LV_COLOR_MAKE(0, 0, 0);
		}
	}

	gameState = STATE_PLAYING;

	shuffleBag();
	spawnPiece();

	needsRedraw = true;
	score = 0;
	updateScore();
}

void shuffleBag()
{
	for(int i = 0; i < NUM_PIECES; i++)
	{
		bag[i] = i;
	}

	for(int i = NUM_PIECES - 1; i > 0; i--)
	{
		int j = random(i + 1);

		int temp = bag[i];
		bag[i] = bag[j];
		bag[j] = temp;
	}
	bagIndex = 0;
}

// Function to create a new Tetris piece
void spawnPiece()
{
	if(gameState == STATE_GAMEOVER)
	{
		return;
	}
	if(bagIndex >= NUM_PIECES)
	{
		shuffleBag();
	}

	currentType = bag[bagIndex++];
	int startX = GRID_COLS / 2;
	int startY = 0;

	for(int i = 0; i < 4; i++)
	{
		currentPiece[i][0] = startX + pieces[currentType][i][0];
		currentPiece[i][1] = startY + pieces[currentType][i][1];
	}

	currentColor = pieceColors[currentType];

	//collision check for game over
	for(int i = 0; i < 4; i++)
	{
		int x = currentPiece[i][0];
		int y = currentPiece[i][1];
		if(grid[y][x].full != 0)
		{
			gameState = STATE_GAMEOVER;
			lv_obj_clear_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN);
			lv_obj_move_foreground(gameOverLabel);
			lv_obj_set_pos(gameOverLabel, 40, 80);
			return;
		}
	}
}

void drawCell(int gx, int gy, lv_color_t color)
{
	int px = gx * CELL_SIZE;
	int py = gy * CELL_SIZE;

	lv_draw_rect_dsc_t dsc;

	lv_draw_rect_dsc_init(&dsc);

	dsc.bg_color = color;
	dsc.bg_opa = LV_OPA_COVER;

	lv_canvas_draw_rect(
		canvas,
		px,
		py,
		CELL_SIZE,
		CELL_SIZE,
		&dsc
	);
}

void updateGhost()
{
	//Copy current piece into ghost
	for(int i = 0; i < 4; i++)
	{
		ghostPiece[i][0] = currentPiece[i][0];
		ghostPiece[i][1] = currentPiece[i][1];
	}

	bool falling = true;

	while(falling == true)
	{
		for(int i = 0; i < 4; i++)
		{
			int x = ghostPiece[i][0];
			int y = ghostPiece[i][1];

			if(y + 1 >= GRID_ROWS)
			{
				falling = false;
				break;
			}

			if(grid[y + 1][x].full != 0)
			{
				falling = false;
				break;
			}
		}

		if(falling == true)
		{
			for(int i = 0; i < 4; i++)
			{
				ghostPiece[i][1]++;
			}
		}
	}
}

//Draws grid and current piece
void drawTetris()
{
	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

	for(int r=0; r < GRID_ROWS; r++)
	{
		for(int c=0;c<GRID_COLS;c++)
		{
			if(grid[r][c].full != 0)
			{
				drawCell(c, r, grid[r][c]);
			}
		}
	}

	//draws ghost piece (dimmed color of current piece dropping)
	lv_color_t ghostColor = lv_color_make(currentColor.ch.red / 2, currentColor.ch.green / 2, currentColor.ch.blue / 2);

	for(int i = 0; i < 4; i++)
	{
		drawCell(ghostPiece[i][0], ghostPiece[i][1], ghostColor);
	}

	for(int i = 0; i < 4; i++)
	{
		drawCell(currentPiece[i][0], currentPiece[i][1], currentColor);
	}
	if(gameState == STATE_GAMEOVER)
	{
	}
}

bool isPartOfCurrentPiece(int x, int y)
{
	for(int i = 0; i < 4; i++)
	{
		if(currentPiece[i][0] == x && currentPiece[i][1] == y)
		{
			return true;
		}
	}
	return false;
}

//Moves current piece down
bool moveDown()
{
	for(int i=0;i<4;i++){
		int x = currentPiece[i][0];
		int y = currentPiece[i][1];

		if(y + 1 >= GRID_ROWS)
		{
			return false;
		}

		//Only block if it's not part of current piece
		if(grid[y+1][x].full != 0 && isPartOfCurrentPiece(x, y+1) == false)
		{
			return false;
		}
	}

	for(int i = 0;i < 4; i++) 
	{
		currentPiece[i][1]++;
	}
	return true;
}

//Locks piece into grid
void lockPiece()
{
	for(int i = 0; i < 4 ; i++)
	{
		int x = currentPiece[i][0];
		int y = currentPiece[i][1];
		grid[y][x] = currentColor;
	}
}

//Checks for full rows
void checkRows()
{
	for(int r=0;r<GRID_ROWS;r++)
	{
		bool full = true;
		for(int c=0;c<GRID_COLS;c++)
		{
			if(grid[r][c].full==0)
			{
				full = false;
			}
		}
		if(full == true)
		{
			//Moves rows above down
			for(int rr = r; rr > 0; rr--)
			{
				for(int c = 0; c < GRID_COLS; c++)
				{
					grid[rr][c]=grid[rr-1][c];
				}
			}
			for(int c = 0; c < GRID_COLS; c++)
			{
				grid[0][c]=LV_COLOR_MAKE(0, 0, 0);
			}
			score += 100;
			updateScore();
		}
	}
}

//Controls
void moveLeft()
{
	for(int i = 0; i < 4; i++)
	{
		if(currentPiece[i][0]==0 || grid[currentPiece[i][1]][currentPiece[i][0]-1].full != 0)
		{
			return;
		}
	}
	for(int i = 0; i < 4; i++)
	{
		currentPiece[i][0] -= 1;
	}
}

void moveRight()
{
	for(int i = 0; i < 4; i++)
	{
		if(currentPiece[i][0]==GRID_COLS-1 || grid[currentPiece[i][1]][currentPiece[i][0]+1].full != 0)
		{
			return;
		}
	}
	for(int i = 0; i < 4; i++)
	{
		currentPiece[i][0] += 1;
	}
}

void rotatePiece()
{
	int pivotX = currentPiece[1][0];
	int pivotY = currentPiece[1][1];

	int rotated[4][2];

	for(int i = 0; i < 4; i++)
	{
		int relX = currentPiece[i][0] - pivotX;
		int relY = currentPiece[i][1] - pivotY;

		rotated[i][0] = pivotX - relY;
		rotated[i][1] = pivotY + relX;
	}

	//Checks the validity of piece
	for(int i = 0; i < 4; i++)
	{
		int x = rotated[i][0];
		int y = rotated[i][1];

		if(x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS)
			return;

		if(grid[y][x].full != 0)
			return;
	}

	//Apply
	for(int i = 0; i < 4; i++)
	{
		currentPiece[i][0] = rotated[i][0];
		currentPiece[i][1] = rotated[i][1];
	}

	needsRedraw = true;
}

void hardDrop()
{
	updateGhost();
	//Move piece to ghost position
	for(int i = 0; i < 4; i++)
	{
		currentPiece[i][0] = ghostPiece[i][0];
		currentPiece[i][1] = ghostPiece[i][1];
	}

	//Locks immediately
	lockPiece();
	checkRows();
	spawnPiece();

	needsRedraw = true;
}



//Setup global variables, set gpio, and LCD screen
void setup()
{
	Serial.begin(115200);
	Serial.println("Tetris LVGL Example");

	// Init display
	if(!gfx->begin())
	{
		while(1);
	}

	gfx->setRotation(0); 
	gfx->invertDisplay(true);

	pinMode(GFX_BL, OUTPUT);
	digitalWrite(GFX_BL, HIGH);

	//Init touch
	Wire.begin(Touch_I2C_SDA, Touch_I2C_SCL);

	touch.setPins(Touch_RST, Touch_INT);

	if (touch.begin(Wire, 0x38, Touch_I2C_SDA, Touch_I2C_SCL) == false)
	{
		Serial.println("FT6336 initialization failed!");
		while (1)
		{
			delay(100);
		}
	}

	touch.setTargetResolution(gfx->width(), gfx->height());

	//Touch orientation settings
	touch.setSwapXY(false);
	touch.setMirrorXY(false, false);
	
	Serial.println("FT6336 initialized!");

	//Init LVGL
	lv_init();
	screenWidth = gfx->width();
	screenHeight = gfx->height();
	bufSize = screenWidth * 40;
	disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
	lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = screenWidth;
	disp_drv.ver_res = screenHeight;
	disp_drv.flush_cb = my_disp_flush;
	disp_drv.draw_buf = &draw_buf;
	lv_disp_drv_register(&disp_drv);

	static lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = touchpad_read_cb;
	lv_indev_drv_register(&indev_drv);
	
	canvas_buf = (lv_color_t *)heap_caps_malloc(
	GRID_COLS * CELL_SIZE * GRID_ROWS * CELL_SIZE * sizeof(lv_color_t),
	MALLOC_CAP_DMA);

	canvas = lv_canvas_create(lv_scr_act());
	lv_canvas_set_buffer(canvas, canvas_buf,
	GRID_COLS * CELL_SIZE,
	GRID_ROWS * CELL_SIZE,
	LV_IMG_CF_TRUE_COLOR);

	lv_obj_center(canvas);

	lv_obj_move_background(canvas);

	shuffleBag();

	spawnPiece();

	gameOverLabel = lv_label_create(lv_scr_act());

	lv_label_set_text(
	gameOverLabel,
	"GAME OVER"
	);

	lv_obj_set_style_text_color(
	gameOverLabel,
	lv_color_make(255, 0, 0),
	0
	);

	lv_obj_set_style_bg_color(
	gameOverLabel,
	lv_color_white(),
	0
	);

	lv_obj_set_style_bg_opa(gameOverLabel, LV_OPA_COVER, 0);

	lv_obj_center(gameOverLabel);

	lv_obj_add_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN);

	scoreLabel = lv_label_create(lv_scr_act());

	lv_label_set_text(scoreLabel, "Score: 0");

	lv_obj_set_style_bg_color(scoreLabel, lv_color_white(), 0);

	lv_obj_set_style_bg_opa(scoreLabel, LV_OPA_COVER, 0);

	lv_obj_set_style_text_color(scoreLabel, lv_color_black(), 0);

	lv_obj_set_style_pad_left(scoreLabel, 10, 0);
	lv_obj_set_style_pad_right(scoreLabel, 10, 0);
	lv_obj_set_style_pad_top(scoreLabel, 5, 0);
	lv_obj_set_style_pad_bottom(scoreLabel, 5, 0);

	lv_obj_align(scoreLabel, LV_ALIGN_TOP_LEFT, 10, 10);
}

void updateScore()
{
	char buffer[32];

	snprintf(buffer, sizeof(buffer), "Score: %d", score);

	lv_label_set_text(scoreLabel, buffer);
}

unsigned long lastDrop = 0;

void loop()
{
	lv_timer_handler();

	switch(gameState)
	{
		case STATE_PLAYING:
		{
			if(millis() - lastDrop > 500)
			{
				if(moveDown() == false)
				{
					if(lockStart == 0)
					{
						lockStart = millis();
					}
					else if(millis() - lockStart >= LOCK_DELAY)
					{
						lockPiece();
						checkRows();
						spawnPiece();
						lockStart = 0;
					}
				}
				else
				{
					lockStart = 0;
				}
				needsRedraw = true;
				lastDrop = millis();
			}

			if(needsRedraw == true)
			{
				updateGhost();
				drawTetris();
				needsRedraw = false;
			}
		}
		break;

		case STATE_GAMEOVER:
		{
			//freeze game logic completely
		}
		break;

		case STATE_MENU:
		default:
		{
			break;
		}
	}

	delay(5);
}

// --- DISPLAY & TOUCH CALLBACKS ---
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);

	gfx->draw16bitRGBBitmap(
		area->x1,
		area->y1,
		(uint16_t *)color_p,
		w,
		h
	);

	lv_disp_flush_ready(disp_drv);
}

bool inputReady()
{
	if (millis() - lastInput >= inputDelay)
	{
		lastInput = millis();
		return true;
	}

	return false;
}

void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
	LV_UNUSED(indev_drv);

	const TouchPoints &points = touch.getTouchPoints();

	if (points.hasPoints() == false)
	{
		data->state = LV_INDEV_STATE_RELEASED;
		return;
	}

	data->state = LV_INDEV_STATE_PRESSED;

	const TouchPoint &pt = points.getPoint(0);

	data->point.x = pt.x;
	data->point.y = pt.y;

	if (gameState == STATE_GAMEOVER)
	{
		resetGame();

		lv_obj_add_flag(gameOverLabel, LV_OBJ_FLAG_HIDDEN);

		gameState = STATE_PLAYING;
		return;
	}
	if (inputReady() == false)
	{
		return;
	}
	if (pt.y < gfx->height() / 3)
	{
		rotatePiece();
	}
	else if (pt.y > 2 * gfx->height() / 2.5)
	{
		hardDrop();
	}
	else if (pt.x < gfx->width() / 3)
	{
		moveLeft();
	}
	else if (pt.x > 2 * gfx->width() / 3)
	{
		moveRight();
	}
}

/*These functions are screen specific. If your screen does not support SPI using the ST7796S
* there will be issues. I am using the "Hosyond 4.0 Inch 320x480 TN Capacitive Touch Screen 
* LCD Display Module SPI Serial ST7796S Driver for Arduino" for this project.
*/

void LCD_WR_REG(uint8_t reg)
{
	digitalWrite(LCD_RS, LOW);
	digitalWrite(LCD_CS, LOW);

	spi->transfer(reg);

	digitalWrite(LCD_CS, HIGH);
}

void LCD_WR_DATA(uint8_t data)
{
	digitalWrite(LCD_RS, HIGH);
	digitalWrite(LCD_CS, LOW);

	spi->transfer(data);

	digitalWrite(LCD_CS, HIGH);
}

void LCD_WR_DATA16(uint16_t data)
{
	digitalWrite(LCD_RS, HIGH);
	digitalWrite(LCD_CS, LOW);

	spi->transfer(data >> 8);
	spi->transfer(data & 0xFF);

	digitalWrite(LCD_CS, HIGH);
}


//Reset the LCD screen
void LCD_RESET()
{
	digitalWrite(LCD_RST, HIGH);
	delay(10);

	digitalWrite(LCD_RST, LOW);
	delay(100);

	digitalWrite(LCD_RST, HIGH);
	delay(50);
}


// Manufacturer's ST7796S initialization
void LCD_Init()
{
	LCD_RESET();

	// 4.0" ST7796S TN initialization
	// From LCDWiki MSP4030/MSP4031

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0xC3);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x96);

	//original value for data: 0x48
	LCD_WR_REG(0x36);
	LCD_WR_DATA(0x48);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x05);

	LCD_WR_REG(0xB0);
	LCD_WR_DATA(0x80);

	LCD_WR_REG(0xB6);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x02);

	LCD_WR_REG(0xB5);
	LCD_WR_DATA(0x02);
	LCD_WR_DATA(0x03);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x04);

	LCD_WR_REG(0xB1);
	LCD_WR_DATA(0x80);
	LCD_WR_DATA(0x10);

	LCD_WR_REG(0xB4);
	LCD_WR_DATA(0x00);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA(0xC6);

	LCD_WR_REG(0xC5);
	LCD_WR_DATA(0x1C);

	LCD_WR_REG(0xE4);
	LCD_WR_DATA(0x31);

	LCD_WR_REG(0xE8);
	LCD_WR_DATA(0x40);
	LCD_WR_DATA(0x8A);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x29);
	LCD_WR_DATA(0x19);
	LCD_WR_DATA(0xA5);
	LCD_WR_DATA(0x33);

	LCD_WR_REG(0xC2);
	LCD_WR_REG(0xA7);

	// Gamma
	LCD_WR_REG(0xE0);
	LCD_WR_DATA(0xF0);
	LCD_WR_DATA(0x09);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x12);
	LCD_WR_DATA(0x12);
	LCD_WR_DATA(0x2B);
	LCD_WR_DATA(0x3C);
	LCD_WR_DATA(0x44);
	LCD_WR_DATA(0x4B);
	LCD_WR_DATA(0x1B);
	LCD_WR_DATA(0x18);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x1D);
	LCD_WR_DATA(0x21);

	// Gamma
	LCD_WR_REG(0xE1);
	LCD_WR_DATA(0xF0);
	LCD_WR_DATA(0x09);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x0D);
	LCD_WR_DATA(0x27);
	LCD_WR_DATA(0x3B);
	LCD_WR_DATA(0x44);
	LCD_WR_DATA(0x4D);
	LCD_WR_DATA(0x0B);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x1D);
	LCD_WR_DATA(0x21);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x3C);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x69);

	LCD_WR_REG(0x13);

	LCD_WR_REG(0x11);
	delay(120);

	LCD_WR_REG(0x29);
	delay(20);
}

// Set address window
void LCD_SetAddress( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	LCD_WR_REG(0x2A);

	digitalWrite(LCD_RS, HIGH);
	digitalWrite(LCD_CS, LOW);

	spi->transfer(x0 >> 8);
	spi->transfer(x0 & 0xFF);
	spi->transfer(x1 >> 8);
	spi->transfer(x1 & 0xFF);

	digitalWrite(LCD_CS, HIGH);

	LCD_WR_REG(0x2B);

	digitalWrite(LCD_RS, HIGH);
	digitalWrite(LCD_CS, LOW);

	spi->transfer(y0 >> 8);
	spi->transfer(y0 & 0xFF);
	spi->transfer(y1 >> 8);
	spi->transfer(y1 & 0xFF);

	digitalWrite(LCD_CS, HIGH);

	LCD_WR_REG(0x2C);
}