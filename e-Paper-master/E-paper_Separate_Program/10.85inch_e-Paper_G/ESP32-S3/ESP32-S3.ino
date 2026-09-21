#include "EPD_10in85g.h"
#include "GUI_Paint.h"
#include "fonts.h"
#include "ImageData.h"

void setup() {
    printf("EPD_10IN85G_test Demo\r\n");
    DEV_Module_Init();

     Debug("e-Paper Init and Clear...\r\n");
    EPD_10in85g_Init();
    EPD_10in85g_Clear(EPD_10in85g_WHITE);
    DEV_Delay_ms(500);

    //Create a new image cache
    UBYTE *Image;
    UDOUBLE Imagesize = ((EPD_10in85g_WIDTH % 4 == 0)? (EPD_10in85g_WIDTH / 4 ): (EPD_10in85g_WIDTH / 4 + 1)) * EPD_10in85g_HEIGHT;
    if((Image = (UBYTE *)ps_malloc(Imagesize*2)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(Image, EPD_10in85g_WIDTH*2, EPD_10in85g_HEIGHT, 0, WHITE);
    Paint_SetScale(4);

#if 1   // show bmp
    EPD_10in85g_Init_Fast();
    EPD_10in85g_Display(gImage_10in85G);
    DEV_Delay_ms(1500);
#endif

#if 1 // Drawing on the image
    //1.Select Image
    printf("SelectImage:BlackImage\r\n");
    Paint_SelectImage(Image);
    Paint_Clear(EPD_10in85g_WHITE);

    // 2.Drawing on the image
    printf("Drawing:Image\r\n");
    Paint_DrawPoint(10, 80, EPD_10in85g_RED, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, EPD_10in85g_YELLOW, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, EPD_10in85g_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, EPD_10in85g_RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, EPD_10in85g_RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, EPD_10in85g_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, EPD_10in85g_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(45, 95, 20, EPD_10in85g_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, EPD_10in85g_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, EPD_10in85g_RED, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, EPD_10in85g_YELLOW, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 0, "Red, yellow, white and black", &Font24, EPD_10in85g_RED, EPD_10in85g_YELLOW);
    Paint_DrawString_EN(10, 30, "Four color e-Paper", &Font16, EPD_10in85g_YELLOW, EPD_10in85g_BLACK);
    Paint_DrawString_CN(150, 45, "微雪电子", &Font24CN, EPD_10in85g_RED, EPD_10in85g_WHITE);
    Paint_DrawNum(10, 50, 123456, &Font12, EPD_10in85g_RED, EPD_10in85g_WHITE);

    printf("EPD_Display\r\n");
    EPD_10in85g_Display(Image);
    DEV_Delay_ms(3000);
#endif

    Debug("Clear...\r\n");
    EPD_10in85g_Init();
    EPD_10in85g_Clear(EPD_10in85g_WHITE);

    Debug("Goto Sleep...\r\n");
    EPD_10in85g_Sleep();
    free(Image);
    DEV_Delay_ms(2000);

    // close 5V
    Debug("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
}


void loop() {

}