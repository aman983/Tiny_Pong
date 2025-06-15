#define F_CPU 9600000UL
#include <avr/io.h>
#include <util/delay.h>

#define MISO PB1 
#define MOSI PB0
#define CLK  PB2
#define CS   PB1

#define BUTTON_1    PB4 // LEFT
#define BUTTON_2    PB3 // RIGHT

#define LOW(x) PORTB &= ~(1 << (x) );
#define HIGH(x) PORTB |= (1 << (x) );


#define NO_OP_REG           0x00
#define INTENSITY_REG       0x0A
#define SCAN_LIMIT_REG      0x0B
#define SHUTDOWN_REG        0x0C
#define DISPLAY_TEST_REG    0x0F 

// MAX7219 ROWS
#define R1  0x01

void SPI_Transmit(uint8_t *Data, uint8_t len){
    for(uint8_t byte = 0; byte<len; byte++){
        
        for(uint8_t i=0; i<8; i++){
            LOW(CLK);
            if(Data[byte] & ( 1 << (7-i) ) ){
                HIGH(MOSI);
            }else{
                LOW(MOSI);
            }
            HIGH(CLK);
        }
        
    }
    
    LOW(CLK);
}

void MAX7219_SendCommand(uint8_t reg, uint8_t data) {
    LOW(CS);
    _delay_us(1);
    uint8_t cmd[2] = {reg, data};
    SPI_Transmit(cmd, 2);
    _delay_us(1);
    HIGH(CS);

}
void MAX7219_Init(void){
    MAX7219_SendCommand(INTENSITY_REG, 0x02);
    MAX7219_SendCommand(SCAN_LIMIT_REG, 0x07);
    MAX7219_SendCommand(SHUTDOWN_REG, 0x01);
    MAX7219_SendCommand(DISPLAY_TEST_REG, 0x00);

}

void MAX7219_Display(uint8_t *Frame){
    for(uint8_t segmnets = 0x01; segmnets <= 0x08; segmnets++){
        MAX7219_SendCommand(segmnets, Frame[segmnets - 1]);
    }
}

void MAX7219_clear(){
    for(uint8_t segmnets = 0x01; segmnets <= 0x08; segmnets++){
        MAX7219_SendCommand(segmnets, 0);
    }
}


void GAME_OVER(uint8_t *Frame){
    // GAME OVER
    MAX7219_clear();
    _delay_ms(500);
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            _delay_ms(50);
            Frame[i] |= (1 << j);
            MAX7219_Display(Frame);
        }
   }

    for (uint8_t i = 0; i < 8; i++) {
        Frame[i] = 0;
    }
                
    _delay_ms(500);
    MAX7219_clear();

}

uint8_t lfsr = 0xB8; // Any non-zero seed

// Lightweight pseudo-random generator using LFSR
uint8_t rand8_lfsr() {
    static uint8_t lfsr = 0xB8;
    lfsr ^= lfsr >> 1;
    lfsr ^= lfsr << 1;
    lfsr ^= lfsr >> 2;
    return lfsr;
}
/*      RIGHT
       x3 x2 x1  
 T  y1 |  |  |  E
 O  y2 |  |  |  N
 P  y3 |  |  |  D
        LEFT
 */

int main(){
    DDRB |= (1 << MOSI) | (1 << CLK) | (1 << CS); 
    DDRB &= ~ ( (1 << BUTTON_1) | (1 << BUTTON_2) ); 
    MAX7219_Init();

    uint8_t x = 4, y = 4;
    int8_t dx = 1, dy = -1;
    uint8_t hit_pos = 0;

    uint8_t paddle_y = 3;
    const uint8_t paddle_size = 3;

    uint8_t frame[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    while (1) 
    {
        // clear Frame
        for(uint8_t i=0; i<8 ; i++){
            frame[i] = 0;
        }

        MAX7219_clear();


        // Draw the Ball (row, col)
        frame[y] |= 1 << x;
        
        // Draw the Paddle
        if (PINB & (1 << BUTTON_1)) {
            if (paddle_y + paddle_size < 8) paddle_y++;  // Move down
        } else if (PINB & (1 << BUTTON_2)) {
            if (paddle_y > 0) paddle_y--;               // Move up
        }
            for(uint8_t i=0; i<paddle_size; i++){
            frame[paddle_y + i] |= 1;
        }

        // Draw the frame
        MAX7219_Display(frame);

        _delay_ms(100);


        // Check for wall and Paddle Collision

        //Bottom
        if (x == 0) {
            // Ball hits paddle?
            if (y >= paddle_y && y < paddle_y + paddle_size) {
                hit_pos = y - paddle_y;
                
                

                if (hit_pos == 0) {
                    dx = 1;
                    dy = -1 + (rand8_lfsr() % 2);  // -1 or 0
                } else if (hit_pos == 1) {
                    dx = 1;
                    dy = (rand8_lfsr() % 3) - 1;  // -1, 0, or 1
                } else {
                    dx = 1;
                    dy = 0 + (rand8_lfsr() % 2);  // 0 or 1
                }
                // Clamp dy to stay in bounds
                if ((y + dy) < 0) dy = 1;
                if ((y + dy) > 7) dy = -1;
                 
            } else {
                GAME_OVER(frame);
                // Resets
                x = (rand8_lfsr() % 6) + 1;
                y = (rand8_lfsr() % 6) + 1;
                dx = (rand8_lfsr() % 2) ? -1 : 1;
                // Make sure dy is never 0 (to avoid horizontal bouncing)
                uint8_t r = rand8_lfsr() % 3; // 0, 1, or 2
                dy = (r == 0) ? -1 : (r == 1) ? 1 : -1;
            }
        }

        //TOP 
        if(x == 7){
            dx = -dx;
        }

        // Right and Left

        if( y == 0 || y == 7){
            dy = -dy;
        }

        // Update the position
        x += dx;
        y += dy;
    }

    return 0;
}

