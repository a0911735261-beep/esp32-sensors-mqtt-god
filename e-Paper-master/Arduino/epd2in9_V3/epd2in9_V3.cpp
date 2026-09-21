/**
 *  @filename   :   epd2in9_V3.cpp
 *  @brief      :   Implements for e-paper library
 *
 *  Copyright (C) Waveshare
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include "epd2in9_V3.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
    display_update_control = 0xF7;
};

int Epd::Init() {
    /* this calls the peripheral hardware interface, see epdif */
    if (IfInit() != 0) {
        return -1;
    }

    Reset();
    DelayMs(100);

    /* EPD hardware init start */
    WaitUntilIdle();
    SendCommand(0x12);  // SWRESET
    WaitUntilIdle();

    SendCommand(0x01);  // Driver output control
    SendData(0x27);
    SendData(0x01);
    SendData(0x00);

    SendCommand(0x11);  // data entry mode
    SendData(0x03);

    SetMemoryArea(0, 0, width - 1, height - 1);

    SendCommand(0x21);  // Display update control
    SendData(0x00);
    SendData(0x80);

    SetMemoryPointer(0, 0);
    WaitUntilIdle();

    display_update_control = 0xF7;
    /* EPD hardware init end */
    return 0;
}

int Epd::Init_Fast() {
    /* this calls the peripheral hardware interface, see epdif */
    if (IfInit() != 0) {
        return -1;
    }

    Reset();
    DelayMs(100);

    /* EPD hardware init start */
    WaitUntilIdle();
    SendCommand(0x12);  // SWRESET
    WaitUntilIdle();

    SendCommand(0x1A);  // Write to temperature register
    SendData(100);

    SendCommand(0x01);  // Driver output control
    SendData(0x27);
    SendData(0x01);
    SendData(0x00);

    SendCommand(0x11);  // data entry mode
    SendData(0x03);

    SetMemoryArea(0, 0, width - 1, height - 1);

    SendCommand(0x3C);
    SendData(0x05);

    SendCommand(0x21);  // Display update control
    SendData(0x00);
    SendData(0x80);

    SetMemoryPointer(0, 0);

    display_update_control = 0xD7;
    /* EPD hardware init end */
    return 0;
}

int Epd::Init_4Gray() {
    /* this calls the peripheral hardware interface, see epdif */
    if (IfInit() != 0) {
        return -1;
    }

    Reset();
    DelayMs(100);

    /* EPD hardware init start */
    WaitUntilIdle();
    SendCommand(0x12);  // SWRESET
    WaitUntilIdle();

    SendCommand(0x1A);  // Write to temperature register
    SendData(90);

    SendCommand(0x74);  // set analog block control
    SendData(0x54);
    SendCommand(0x7E);  // set digital block control
    SendData(0x3B);

    SendCommand(0x01);  // Driver output control
    SendData(0x27);
    SendData(0x01);
    SendData(0x00);

    SendCommand(0x11);  // data entry mode
    SendData(0x03);

    SetMemoryArea(0, 0, width - 1, height - 1);

    SendCommand(0x3C);
    SendData(0x00);

    SendCommand(0x21);  // Display update control
    SendData(0x00);
    SendData(0x80);

    SetMemoryPointer(0, 0);
    WaitUntilIdle();

    display_update_control = 0xD7;
    /* EPD hardware init end */
    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    DigitalWrite(cs_pin, LOW);
    SpiTransfer(command);
    DigitalWrite(cs_pin, HIGH);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    DigitalWrite(cs_pin, LOW);
    SpiTransfer(data);
    DigitalWrite(cs_pin, HIGH);
}

/**
 *  @brief: Wait until the busy_pin goes LOW
 */
void Epd::WaitUntilIdle(void) {
    while (1) {  // HIGH: busy, LOW: idle
        if (DigitalRead(busy_pin) == LOW)
            break;
        DelayMs(50);
    }
    DelayMs(50);
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    DigitalWrite(reset_pin, HIGH);
    DelayMs(10);
    DigitalWrite(reset_pin, LOW);
    DelayMs(2);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(10);
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 */
void Epd::SetFrameMemory(
    const unsigned char* image_buffer,
    int x,
    int y,
    int image_width,
    int image_height
) {
    int x_end;
    int y_end;

    if (
        image_buffer == NULL ||
        x < 0 || image_width < 0 ||
        y < 0 || image_height < 0
    ) {
        return;
    }
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= this->width) {
        x_end = this->width - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= this->height) {
        y_end = this->height - 1;
    } else {
        y_end = y + image_height - 1;
    }
    SetMemoryArea(x, y, x_end, y_end);
    SetMemoryPointer(x, y);
    SendCommand(0x24);
    /* send the image data */
    for (int j = 0; j < y_end - y + 1; j++) {
        for (int i = 0; i < (x_end - x + 1) / 8; i++) {
            SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
}

void Epd::SetFrameMemory_Partial(
    const unsigned char* image_buffer,
    int x,
    int y,
    int image_width,
    int image_height
) {
    int x_end;
    int y_end;

    if (
        image_buffer == NULL ||
        x < 0 || image_width < 0 ||
        y < 0 || image_height < 0
    ) {
        return;
    }
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= this->width) {
        x_end = this->width - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= this->height) {
        y_end = this->height - 1;
    } else {
        y_end = y + image_height - 1;
    }

    Reset();

    SendCommand(0x3C);  // BorderWaveform
    SendData(0x80);

    SetMemoryArea(x, y, x_end, y_end);
    SetMemoryPointer(x, y);
    SendCommand(0x24);
    /* send the image data */
    for (int j = 0; j < y_end - y + 1; j++) {
        for (int i = 0; i < (x_end - x + 1) / 8; i++) {
            SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 *
 *          Question: When do you use this function instead of
 *          void SetFrameMemory(
 *              const unsigned char* image_buffer,
 *              int x,
 *              int y,
 *              int image_width,
 *              int image_height
 *          );
 *          Answer: SetFrameMemory with parameters only reads image data
 *          from the RAM but not from the flash in AVR chips (for AVR chips,
 *          you have to use the function pgm_read_byte to read buffers
 *          from the flash).
 */
void Epd::SetFrameMemory(const unsigned char* image_buffer) {
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
    SendCommand(0x24);
    /* send the image data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(pgm_read_byte(&image_buffer[i]));
    }
}

void Epd::SetFrameMemory_Base(const unsigned char* image_buffer) {
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
    SendCommand(0x24);
    /* send the image data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(pgm_read_byte(&image_buffer[i]));
    }
    SendCommand(0x26);
    /* send the image data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(pgm_read_byte(&image_buffer[i]));
    }
}

/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void Epd::ClearFrameMemory(unsigned char color) {
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
    SendCommand(0x24);
    /* send the color data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(color);
    }

    SendCommand(0x26);
    /* send the color data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(color);
    }
}

void Epd::Display4Gray(const unsigned char *Image)
{
    int i, j, k;
    unsigned char temp1, temp2, temp3;

    SendCommand(0x24);
    for (i = 0; i < 4736; i++)
    {
        temp3 = 0;
        for (j = 0; j < 2; j++)
        {
            temp1 = pgm_read_byte(&Image[i * 2 + j]);
            for (k = 0; k < 2; k++)
            {
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x00;  // white
                else if (temp2 == 0x00)
                    temp3 |= 0x01;  // black
                else if (temp2 == 0x80)
                    temp3 |= 0x01;  // gray1
                else
                    temp3 |= 0x00;  // gray2
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x00;
                else if (temp2 == 0x00)
                    temp3 |= 0x01;
                else if (temp2 == 0x80)
                    temp3 |= 0x01;
                else
                    temp3 |= 0x00;
                if (j != 1 || k != 1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }
        }
        SendData(temp3);
    }

    SendCommand(0x26);
    for (i = 0; i < 4736; i++)
    {
        temp3 = 0;
        for (j = 0; j < 2; j++)
        {
            temp1 = pgm_read_byte(&Image[i * 2 + j]);
            for (k = 0; k < 2; k++)
            {
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x00;  // white
                else if (temp2 == 0x00)
                    temp3 |= 0x01;  // black
                else if (temp2 == 0x80)
                    temp3 |= 0x00;  // gray1
                else
                    temp3 |= 0x01;  // gray2
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x00;
                else if (temp2 == 0x00)
                    temp3 |= 0x01;
                else if (temp2 == 0x80)
                    temp3 |= 0x00;
                else
                    temp3 |= 0x01;
                if (j != 1 || k != 1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }
        }
        SendData(temp3);
    }

    DisplayFrame();
}

/**
 *  @brief: update the display
 */
void Epd::DisplayFrame(void) {
    TurnOnDisplay(display_update_control);
}

void Epd::DisplayFrame_Partial(void) {
    TurnOnDisplay(0xFF);
}

void Epd::TurnOnDisplay(unsigned char update_control) {
    SendCommand(0x22);  // Display Update Control
    SendData(update_control);
    SendCommand(0x20);  // Activate Display Update Sequence
    WaitUntilIdle();
}

/**
 *  @brief: private function to specify the memory area for data R/W
 */
void Epd::SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
    SendCommand(0x44);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x_start >> 3) & 0xFF);
    SendData((x_end >> 3) & 0xFF);
    SendCommand(0x45);
    SendData(y_start & 0xFF);
    SendData((y_start >> 8) & 0xFF);
    SendData(y_end & 0xFF);
    SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void Epd::SetMemoryPointer(int x, int y) {
    SendCommand(0x4E);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x >> 3) & 0xFF);
    SendCommand(0x4F);
    SendData(y & 0xFF);
    SendData((y >> 8) & 0xFF);
    WaitUntilIdle();
}

/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          You can use Epd::Init() to awaken
 */
void Epd::Sleep() {
    SendCommand(0x10);
    SendData(0x01);
    DelayMs(100);
}

/* END OF FILE */
