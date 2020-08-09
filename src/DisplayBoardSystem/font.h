#define DIGIT_WIDTH 16
#define DIGIT_HEIGHT 28
#define DIGIT_HEIGHT_OFFSET (SCREEN_HEIGHT - DIGIT_HEIGHT) / 2
#define NUM_DIGITS 10

const uint16_t digits[NUM_DIGITS][DIGIT_HEIGHT] =
    {
        {0xF81F, 0xF00F, 0xE007, 0xC003, 0x8001, 0x180, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x180, 0x8001, 0xC003, 0xE007, 0xF00F, 0xF81F},
        {0xF83F, 0xF03F, 0xE03F, 0xC03F, 0xC03F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xFC3F, 0xC003, 0xC003, 0xC003, 0xC003},
        {0xF00F, 0xE007, 0xC003, 0x8001, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFE0, 0xFFC1, 0xFF83, 0xFF07, 0xFE0F, 0xFC1F, 0xF83F, 0xF07F, 0xE0FF, 0xC1FF, 0x83FF, 0x7FF, 0x0, 0x0, 0x0, 0x0},
        {0xF00F, 0xE007, 0xC003, 0x8001, 0x3C1, 0x7E0, 0xFF0, 0xFF0, 0xFFF0, 0xFFF0, 0xFFE0, 0xFFC1, 0xF003, 0xF007, 0xF007, 0xF003, 0xFFC1, 0xFFE0, 0xFFF0, 0xFFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x8001, 0xC003, 0xE007, 0xF00F},
        {0xFF03, 0xFF03, 0xFE03, 0xFE03, 0xFC03, 0xFC03, 0xF843, 0xF843, 0xF0C3, 0xF0C3, 0xE1C3, 0xE1C3, 0xC3C3, 0xC3C3, 0x87C3, 0x87C3, 0x0, 0x0, 0x0, 0x0, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3},
        {0x0, 0x0, 0x0, 0x0, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0x7, 0x3, 0x1, 0x0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFF0, 0xFF0, 0xFF0, 0x0, 0x1, 0xC003, 0xE007},
        {0xE007, 0xC003, 0x8001, 0x0, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFFF, 0xFFF, 0x7FF, 0x3FF, 0x7, 0x3, 0x1, 0x0, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x0, 0x8001, 0xC003, 0xE007},
        {0x0, 0x0, 0x0, 0x0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFFE0, 0xFFC3, 0xFF87, 0xFF0F, 0xFE1F, 0xFC3F, 0xF87F, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF, 0xF0FF},
        {0xE007, 0xC003, 0x8001, 0x0, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0xC003, 0xE007, 0xE007, 0xC003, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x0, 0x8001, 0xC003, 0xE007},
        {0xE007, 0xC003, 0x8001, 0x0, 0x3C0, 0x7E0, 0xFF0, 0xFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x0, 0x8000, 0xC000, 0xE000, 0xFFE0, 0xFFF0, 0xFFF0, 0xFFF0, 0xFF0, 0xFF0, 0x7E0, 0x3C0, 0x0, 0x8001, 0xC003, 0xE007}};

const uint16_t dot[] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3};
const uint16_t colon[] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFC3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
