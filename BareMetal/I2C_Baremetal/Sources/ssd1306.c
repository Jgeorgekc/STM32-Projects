#include "ssd1306.h"
#include "font5x7.h"
#include "i2c.h"

#define SSD1306_ADDRESS 0x78
#define SSD1306_WIDTH   128
#define SSD1306_PAGES   8   // 64 rows / 8

/*
 * WHAT: RAM framebuffer, 1 byte = 8 vertical pixels, page-organized (matches SSD1306 GDDRAM layout)
 * WHY : Lets us build a frame in RAM, then push it in one I2C burst, instead of
 *       issuing separate I2C transactions per pixel (which would be painfully slow + bus-heavy)
 */
static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_PAGES];

void SSD1306_Command(uint8_t cmd)
{
    I2C_Start();
    I2C_SendAddress(SSD1306_ADDRESS);
    I2C_WriteByte(0x00);   // Control byte: command follows
    I2C_WriteByte(cmd);
    I2C_Stop();
}

void SSD1306_Init(void)
{
    SSD1306_Command(0xAE);
    SSD1306_Command(0x8D);
    SSD1306_Command(0x14);
    SSD1306_Command(0xA8);
    SSD1306_Command(0x3F);
    SSD1306_Command(0xD3);
    SSD1306_Command(0x00);
    SSD1306_Command(0x40);
    SSD1306_Command(0xA1);
    SSD1306_Command(0xC8);
    SSD1306_Command(0xDA);
    SSD1306_Command(0x12);
    SSD1306_Command(0x81);
    SSD1306_Command(0x7F);
    SSD1306_Command(0xA4);
    SSD1306_Command(0xA6);
    SSD1306_Command(0xAF);
}

/*
 * WHAT: Zeroes out the RAM framebuffer (does NOT touch the physical screen yet)
 * WHY : Must call SSD1306_UpdateScreen() afterward to actually push blank data over I2C
 */
void SSD1306_Clear(void)
{
    uint16_t i;
    for (i = 0; i < SSD1306_WIDTH * SSD1306_PAGES; i++)
        framebuffer[i] = 0x00;
}

/*
 * WHAT: Pushes the entire RAM framebuffer to the SSD1306 GDDRAM, page by page
 * WHY : SSD1306 page-addressed mode requires setting page+column before each row of data
 * HOW : For each of 8 pages, set page addr, reset column to 0, then stream 128 bytes as one I2C write
 */
void SSD1306_UpdateScreen(void)
{
    uint8_t page;
    for (page = 0; page < SSD1306_PAGES; page++)
    {
        SSD1306_Command(0xB0 + page);  // Set page address
        SSD1306_Command(0x00);         // Lower column = 0
        SSD1306_Command(0x10);         // Higher column = 0

        I2C_Start();
        I2C_SendAddress(SSD1306_ADDRESS);
        I2C_WriteByte(0x40);   // Control byte: data follows

        uint16_t col;
        for (col = 0; col < SSD1306_WIDTH; col++)
        {
            I2C_WriteByte(framebuffer[page * SSD1306_WIDTH + col]);
        }

        I2C_Stop();
    }
}

/*
 * WHAT: Sets or clears a single pixel in the RAM framebuffer (not on screen yet)
 * WHY : Building block for text/shapes — call SSD1306_UpdateScreen() after batch of changes
 * HOW : y determines which page (y/8) and which bit within that page's byte (y%8)
 */
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= 64) return;

    uint8_t page = y / 8;
    uint8_t bit  = y % 8;

    if (color)
        framebuffer[page * SSD1306_WIDTH + x] |= (1 << bit);
    else
        framebuffer[page * SSD1306_WIDTH + x] &= ~(1 << bit);
}

/*
 * WHAT: Draws one character directly into the framebuffer at (x, page)
 * WHY : Font bytes are already page-aligned (5 bytes = 5 columns, 8 vertical pixels each) —
 *       we can copy them straight into the framebuffer without per-pixel looping
 * HOW : x = starting column (0-127), page = row group (0-7), char must be space..'z' (32-122)
 */
void SSD1306_PrintChar(uint8_t x, uint8_t page, char c)
{
    if (c < ' ' || c > 'z') c = ' ';
    uint8_t index = c - ' ';

    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        if ((x + i) < SSD1306_WIDTH)
            framebuffer[page * SSD1306_WIDTH + (x + i)] = font5x7[index][i];
    }
    // 1-pixel gap after each character
    if ((x + 5) < SSD1306_WIDTH)
        framebuffer[page * SSD1306_WIDTH + (x + 5)] = 0x00;
}

/*
 * WHAT: Draws a full string starting at (x, page), advancing 6 px per character
 */
void SSD1306_PrintString(uint8_t x, uint8_t page, const char *str)
{
    while (*str)
    {
        SSD1306_PrintChar(x, page, *str);
        x += 6;   // 5px glyph + 1px gap
        str++;
    }
}
