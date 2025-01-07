//
// Created by Wiktor on 17.10.2024.
//
#include <cstdint>
#include <cstdio>
#include <GL/gl.h>


int32_t EAN13_WIDTH = 1 ,EAN13_HEIGHT = 69;
double EAN13_WIDTH_D = 31.35/95.0, EAN13_HEIGHT_D = 22.85, EAN13_MAX_HEIGHT = 25.95, EAN13_FIRST_SPACE = 3.66, EAN13_TEXT_SIZE_D = 1.5;

struct ean13{
    uint8_t A[10] = {0b0001101, 0b0011001, 0b0010011,0b0111101, 0b0100011,
                     0b0110001,0b0101111,0b0111011,0b0110111,0b0001011};
    uint8_t B[10] = {0b0100111, 0b0110011, 0b0011011,0b0100001, 0b0011101,
                     0b0111001,0b0000101,0b0010001,0b0001001,0b0010111};

    uint8_t oddity[10] = {
        0b111111, 0b110100,0b110010,0b110001,0b101100, 0b100110, 0b100011, 0b101010, 0b101001, 0b100101
    };
};


///SVG

void initSVG(FILE *svg, int width, int height){
    fprintf(svg , "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    fprintf(svg , "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" " );
    fprintf( svg, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n" );
    fprintf(svg , "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" style=\"background-color: white\">\n", width, height, width, height );
    fprintf(svg, "<rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" style=\"fill: white\"/>\n", width, height);
}

void initSVG_d(FILE *svg, double width, double height){
    fprintf(svg , "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    fprintf(svg , "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" " );
    fprintf( svg, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n" );
    fprintf(svg , "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%f\" height=\"%f\" viewBox=\"0 0 %f %f\" style=\"background-color: white\">\n", width, height, width, height );
    fprintf(svg, "<rect x=\"0\" y=\"0\" width=\"%f\" height=\"%f\" style=\"fill: white\"/>\n", width, height);
}

void endSVG(FILE *svg){
    fprintf( svg, "</svg>" );
    fclose( svg );
    //free( svg );
}

void svg_line( FILE *svg, int x1, int y1, int x2, int y2, int stroke_width, unsigned int color )
{
    fprintf( svg, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"stroke-width:%d; stroke:#%06x\" />\n", x1, y1, x2, y2, stroke_width, color );
}

void svg_line_d( FILE *svg, double x1, double y1, double x2, double y2, double stroke_width, unsigned int color )
{
    fprintf( svg, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"stroke-width:%f; stroke:#%06x\" />\n", x1, y1, x2, y2, stroke_width, color );
}

void svg_text_d(FILE *svg, double x1, double y1, double size, unsigned int color, const char text)
{
    fprintf(svg, "<text x=\"%f\" y=\"%f\" font-size=\"%f\" fill=\"#%06x\">%c</text>\n",
            x1, y1, size, color, text);
}

void encodeEan(const char* code, char * finalCode){
    struct ean13 _ean;
    FILE *file = fopen("output2.svg","w");

    // Input validation
    for(int i = 0; i < 13; i++) {
        if (code[i] < '0' || code[i] > '9') {
            printf("Invalid key entered\n");
            return;
        }
    }
    int marginX = 10;
    int marginY = 10;
    double loc= marginX;
    double width = 95* EAN13_WIDTH_D + 2 * marginX;
    double height = EAN13_HEIGHT_D + 2 * marginY;
    initSVG_d(file,width,height);
    svg_text_d(file,loc,EAN13_HEIGHT_D+2,EAN13_TEXT_SIZE_D,0,code[0]);
    loc+=EAN13_FIRST_SPACE;
    // Add the first guard bar
    finalCode[0] = '1';
    finalCode[1] = '0';
    finalCode[2] = '1';
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    loc+=2*EAN13_WIDTH_D;
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    // Calculate the parity bit for the left part based on the first digit
    uint8_t oddity = _ean.oddity[code[0] - '0'];
    int index = 3; // Start index for the encoded left part

    // Encode the left part (6 digits)
    for(int i = 1; i < 7; i++) {
        uint8_t digit = code[i] - '0';
        uint8_t *encodingSet = (oddity & (1 << (6 - i))) ? _ean.A : _ean.B;
        svg_text_d(file,loc+1,EAN13_HEIGHT_D+2,EAN13_TEXT_SIZE_D,0,code[i]);
        printf("I : %c ,:",code[i]);
        for (int j = 0; j < 7; j++) {
            finalCode[index++] = (encodingSet[digit] >> (6 - j)) & 1 ? '1' : '0';
            printf("%c", finalCode[index-1]);
            if(finalCode[index-1] == '1')
                svg_line_d(file,loc,0,loc,EAN13_HEIGHT_D, EAN13_WIDTH_D, 0);
            loc+=EAN13_WIDTH_D;
        }
    }
    // Add the middle guard bar
    finalCode[index++] = '0';
    finalCode[index++] = '1';
    finalCode[index++] = '0';
    finalCode[index++] = '1';
    finalCode[index++] = '0';

    loc+=EAN13_WIDTH_D;
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    loc+=2*EAN13_WIDTH_D;
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    // Encode the right part (6 digits) using A encoding flipped (i.e., 0s become 1s and vice versa)
    for(int i = 7; i < 13; i++) {
        uint8_t digit = code[i] - '0';
        svg_text_d(file,loc+1,EAN13_HEIGHT_D+2,EAN13_TEXT_SIZE_D,0,code[i]);
        for (int j = 0; j < 7; j++) {
            finalCode[index++] = (_ean.A[digit] >> (6 - j)) & 1 ? '0' : '1';
            if(finalCode[index-1] == '1')
                svg_line_d(file,loc,0,loc,EAN13_HEIGHT_D, EAN13_WIDTH_D, 0);
            loc+=EAN13_WIDTH_D;
        }
    }

    // Add the final guard bar
    finalCode[index++] = '1';
    finalCode[index++] = '0';
    finalCode[index++] = '1';
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    loc+=2*EAN13_WIDTH_D;
    svg_line_d(file,loc,0,loc,EAN13_MAX_HEIGHT, EAN13_WIDTH_D, 0);
    endSVG(file);

    // Null-terminate the final code
    finalCode[index] = '\0';
    printf("Encoded EAN-13: %s\n", finalCode);
}

void drawCode(char* filename, char* encoded){
    FILE *f = fopen(filename, "w");

    // Add some space around the code
    int marginX = 150;
    int marginY = 150;
    int width = strlen(encoded) * EAN13_WIDTH + 2 * marginX;
    int height = EAN13_HEIGHT + 2 * marginY;

    initSVG(f, width, height);
    int i = 0 ;
    while(encoded[i]!='\0'){
        if(encoded[i] == '1')
        {
            // Add some space around the code
            svg_line(f, marginX + i*EAN13_WIDTH, marginY, marginX + i*EAN13_WIDTH, marginY + EAN13_HEIGHT, EAN13_WIDTH, 0);
        }
        i++;
    }

    endSVG(f);
}

int main(){

    char code[] = "0811538013024";
    char finalCode[100];
    encodeEan(code, finalCode);
    printf("Encoded EAN-13: %s\n", finalCode);
    char filename[] = "output.svg\0";
    drawCode(filename, finalCode);
}