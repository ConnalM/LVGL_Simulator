// Basic setup for TFT_eSPI with ESP32
#define USER_SETUP_LOADED

// Select your display driver (uncomment one):
// #define ILI9341_DRIVER
// #define ST7735_DRIVER
// #define ST7789_DRIVER
#define ILI9488_DRIVER  // Common for 3.5" displays

// Define display dimensions
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// For ESP32
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin

// For better performance, enable only the fonts you need
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.

// SPI frequency
#define SPI_FREQUENCY  27000000

// Comment out the next line to use SPI
// #define USE_HSPI_PORT

// For better performance, enable this
#define SPI_READ_FREQUENCY  6000000

// For better performance, enable this if your TFT has a built-in touch controller
// #define TOUCH_CS 5  // Chip select pin (T_CS) of touch screen
