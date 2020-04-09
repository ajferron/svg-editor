#include <SVGParser.h>
#include <LinkedListAPI.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define N 12

void printSVG(SVGimage *svg) {
    char *str = SVGimageToString(svg);
    printf("%s\n", str);
    free(str);
}

void printfr(char *str) {
    printf("\n%s\n", str);
    free(str);
}

void compSafe(char *str1, char *str2) {
    if ( strcmp(str1, str2) )
        printf("[!] COMPARISON FAILED\n%s \n%s", str1, str2);
    else
        // printf("[!] COMPARISON PASSED\n");
        
    free(str1);
    free(str2);
}

int main(int argc, char **argv) {
    SVGimage *image;
    
    char *files[] = { "./svg/Emoji_grinning.svg",    // 0
                     "./svg/Emoji_party_A2.svg",    // 1
                     "./svg/Emoji_poo_A2.svg",      // 2
                     "./svg/Emoji_shades.svg",      // 3
                     "./svg/Emoji_smiling.svg",     // 4
                     "./svg/Emoji_thumb.svg",       // 5
                     "./svg/hen_and_chicks.svg",    // 6
                     "./svg/quad01_A2.svg",         // 7
                     "./svg/rects.svg",             // 8
                     "./svg/rects_gg.svg",          // 9
                     "./svg/satisfaction.svg",      // 10
                     "./svg/vest.svg",              // 11
                     "./svg/image.svg",             // 12
                     "./svg/yoyo.svg"               // 13
                    };
    // printf("%s\n", addRectFromJSON(files[13], "{\"x\":\"1234\",\"y\":\"22\",\"w\":\"11\",\"h\":\"00\"}") );
    printf("%s\n", updateFileTitle(files[13], "AHHHHHH") );
    // image = createSVGimage(files[13]);
    // Rectangle *r = JSONtoRect("{\"x\":\"33\",\"y\":\"22\",\"w\":\"11\",\"h\":\"00\"}");
    // insertBack(image->rectangles, r);
    // printfr(SVGtoJSON(image));
    // deleteSVGimage(image);


    // image = createSVGimage(files[0]);
    // printfr(SVGtoJSON(image));
    // deleteSVGimage(image);
    // Rectangle *r = JSONtoRect(argv[1]);
    // printf("\n%s\n", addRectFromJSON("./svg/yoyo.svg", "{\"x\":\"22\",\"y\":\"33\",\"w\":\"11\",\"h\":\"44\"}"));
    // printf("\n\"%s\"\n\nx: %s, y: %s, w: %s, h: %s\n\n", argv[2], r->x, r->y, r->width, r->height);

    
    // printfr( fileToSummaryJSON(files[12]) );

    // for (int i = 0; i < 12; i++) {
        // printfr( fileToJSON(files[i]) );
        // printf("\n\n");
    // }

    /* SVGimage *svg = JSONtoSVG("{\"title\":\"\",\"descr\":\"\"}");
    Rectangle *rect = JSONtoRect("{\"x\":1,\"y\":2,\"w\":19,\"h\":15,\"units\":\"cm\"}");
    Circle *circle = JSONtoCircle("{\"cx\":32,\"cy\":32,\"r\":30,\"units\":\"cm\"}");

    printfr(SVGtoJSON(svg));
    printfr(rectToJSON(rect));
    printfr(circleToJSON(circle));

    deleteSVGimage(svg);
    deleteRectangle(rect);
    deleteCircle(circle); */

    return 0;
}
