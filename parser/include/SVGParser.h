#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"


typedef enum COMP { SVG_IMAGE, CIRC, RECT, PATH, GROUP } elementType;


typedef struct  {
	char* 	name;
	char*	value; 
} Attribute;


typedef struct {
    List*   rectangles;
    List*   circles;
    List*   paths;
    List*   groups;

    List* otherAttributes;
} Group;


typedef struct {
    char* x;
    char* y;
    char* width;
    char* height;

    List* otherAttributes;
} Rectangle;


typedef struct {
    char* cx;
    char* cy;
    char* r;

    List* otherAttributes;
} Circle;


typedef struct {
    char* data;

    List* otherAttributes;
} Path;


typedef struct {
    char namespace[256];
    char title[256];
    char description[256];

    List* rectangles;
    List* circles;
    List* paths;
    List* groups;  

    List* otherAttributes;
} SVGimage;



//     ###     ######  ##     ## ##    ## ########           ##   
//    ## ##   ##    ## ###   ### ###   ##    ##            ####   
//   ##   ##  ##       #### #### ####  ##    ##              ##   
//  ##     ##  ######  ## ### ## ## ## ##    ##              ##   
//  #########       ## ##     ## ##  ####    ##              ##   
//  ##     ## ##    ## ##     ## ##   ###    ##    ###       ##   
//  ##     ##  ######  ##     ## ##    ##    ##    ###     ###### 

SVGimage* createSVGimage(char* fileName);

char* SVGimageToString(SVGimage* img);

void deleteSVGimage(SVGimage* img);

List* getRects(SVGimage* img);
List* getCircles(SVGimage* img);
List* getGroups(SVGimage* img);
List* getPaths(SVGimage* img);

int numRectsWithArea(SVGimage* img, float area);
int numCirclesWithArea(SVGimage* img, float area);
int numPathsWithdata(SVGimage* img, char* data);
int numGroupsWithLen(SVGimage* img, int len);

int numAttr(SVGimage* img);



//     ###     ######  ##     ## ##    ## ########         #######  
//    ## ##   ##    ## ###   ### ###   ##    ##           ##     ## 
//   ##   ##  ##       #### #### ####  ##    ##                  ## 
//  ##     ##  ######  ## ### ## ## ## ##    ##            #######  
//  #########       ## ##     ## ##  ####    ##           ##        
//  ##     ## ##    ## ##     ## ##   ###    ##    ###    ##        
//  ##     ##  ######  ##     ## ##    ##    ##    ###    ######### 

bool validateSVGimage(SVGimage* image, char* schemaFile);

SVGimage* createValidSVGimage(char* fileName, char* schemaFile);

char* createSVGFile(char *filename);

bool writeSVGimage(SVGimage* image, char* fileName);

void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);

void addComponent(SVGimage* image, elementType type, void* newElement);


char* validateSVG(char *filepath);
char* fileToJSON(char *filepath);
char* fileToSummaryJSON(char *filepath);

char* attrToJSON(const Attribute *a);
char* circleToJSON(const Circle *c);
char* rectToJSON(const Rectangle *r);
char* pathToJSON(const Path *p);
char* groupToJSON(const Group *g);

char* attrListToJSON(const List *list);
char* circListToJSON(const List *list);
char* rectListToJSON(const List *list);
char* pathListToJSON(const List *list);
char* groupListToJSON(const List *list);
char* SVGtoJSON(const SVGimage* img);


char* setAttributeInFile(char *filename, char *type, int idx, char *name, char *value);

char* addRectFromJSON(char *filename, char *json);
char* addCircleFromJSON(char *filename, char *json);

char* updateFileTitle(char *filename, char *title);
char* updateFileDesc(char *filename, char *desc);

SVGimage* JSONtoSVG(const char* svgString);
Rectangle* JSONtoRect(const char* svgString);
Circle* JSONtoCircle(const char* svgString);



//  ##     ## ######## ##       ########  ######## ########     ######## ##    ##  ######      
//  ##     ## ##       ##       ##     ## ##       ##     ##    ##       ###   ## ##    ##     
//  ##     ## ##       ##       ##     ## ##       ##     ##    ##       ####  ## ##           
//  ######### ######   ##       ########  ######   ########     ######   ## ## ##  ######      
//  ##     ## ##       ##       ##        ##       ##   ##      ##       ##  ####       ##     
//  ##     ## ##       ##       ##        ##       ##    ##     ##       ##   ### ##    ## ### 
//  ##     ## ######## ######## ##        ######## ##     ##    ##       ##    ##  ######  ### 

void deleteAttribute( void* data);
char* attributeToString( void* data);
int compareAttributes(const void *first, const void *second);

void deleteGroup(void* data);
char* groupToString( void* data);
int compareGroups(const void *first, const void *second);

void deleteRectangle(void* data);
char* rectangleToString(void* data);
int compareRectangles(const void *first, const void *second);

void deleteCircle(void* data);
char* circleToString(void* data);
int compareCircles(const void *first, const void *second);

void deletePath(void* data);
char* pathToString(void* data);
int comparePaths(const void *first, const void *second);

#endif
