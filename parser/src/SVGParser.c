#include <SVGParser.h>
#include <math.h>

#define PI 3.14159265358979323846264338327950288419716939937510

SVGimage *initSVGimage();
Rectangle *createRectangle();
Circle *createCircle();
Path *createPath();
Group *createGroup();
Attribute *createAttribute(const void *, const void *);

List *rectangleList();
List *circleList();
List *pathList();
List *groupList();
List *subGroupList();
List *attributeList();

xmlDoc *SVGimageToDoc(SVGimage* img);
void buildSVGimage(SVGimage *svg, xmlNode *root, Group *cGroup);
bool validateXMLDoc(xmlDoc* doc, char* schemaFile);

void setSVGNamespace(SVGimage *svg, xmlDoc *doc, xmlNode *root);
void createFromNode(void *element, xmlNode *node, void (*nodeParser)(void*, char*, char*));
void rectNodeParser(void *element, char *name, char *value);
void circleNodeParser(void *element, char *name, char *value);
void pathNodeParser(void *element, char *name, char *value);
void groupNodeParser(void *element, char *name, char *value);

char *propValue(float f, char *units);
void newAttribute(xmlNode *node, char* name, char* value);
void addAttributes(xmlNode *node, List *attrs);
void addRectangles(xmlNode* root, List *rectList);
void addCircles(xmlNode* root, List *circleList);
void addPaths(xmlNode* root, List *pathList);
void addGroups(xmlNode* root, List *groupList);

bool validateAttributes(List *attributes);
bool validateRectangles(List *rectangles);
bool validateCircles(List *circles);
bool validatePaths(List *paths);
bool validateGroups(List *groups);

int searchSVGImage(List *pool, List *found, const void* data, int (*compare)(const void*, const void*));
void findAll(List *list, List *found, const void* record, int (*compare)(const void*, const void*));
void getFromGroups(List *groups, List *found, elementType type);

int compRectangleArea(const void *, const void *);
int compGroupLength(const void *, const void *);
int compCircleArea(const void *, const void *);
int compPathData(const void *, const void *);

void updateSVGimageAttributes(SVGimage *img, Attribute *newAttr);
void updateAttribute(List *attributes, Attribute *key);
void updateRectangleAttributes(Rectangle *rect, Attribute *newAttr);
void updateCircleAttributes(Circle *circle, Attribute *newAttr);
void updatePathAttributes(Path *path, Attribute *newAttr);
void updateGroupAttributes(Group *group, Attribute *newAttr);

char* listToJSON(char *str, char *json[], int n);
void copyList(List *source, List *dest);
void copyField(char *field, char *content);
char* toString2(List *list, char *delim);
void *listAt(List *list, int index);

void dummyDelete(void* data) {}



SVGimage* createSVGimage(char* fileName) {
    if ( !fileName ) return NULL;

    xmlDoc *doc;
    xmlNode *root_element;
    SVGimage *svg;

    LIBXML_TEST_VERSION

    if ( !(doc = xmlReadFile(fileName, NULL, 0)) ) {
        xmlCleanupParser();
        return NULL;
    }

    root_element = xmlDocGetRootElement(doc);
    svg = initSVGimage();

    setSVGNamespace(svg, doc, root_element);
    buildSVGimage(svg, root_element, NULL);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return svg;
}

SVGimage* createValidSVGimage(char* fileName, char* schemaFile) {
    if ( !fileName || !schemaFile ) return NULL;

    xmlDoc *doc = NULL;
    SVGimage *svg = NULL;
    xmlNode *root_element;

    LIBXML_TEST_VERSION

    if ( !(doc = xmlReadFile(fileName, NULL, 0)) )
        goto cleanup;

    if ( !validateXMLDoc(doc, schemaFile) )
        goto cleanup;

    root_element = xmlDocGetRootElement(doc);
    svg = initSVGimage();

    setSVGNamespace(svg, doc, root_element);
    buildSVGimage(svg, root_element, NULL);

    cleanup:
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return svg;
}

void buildSVGimage(SVGimage *svg, xmlNode *root, Group *cGroup) {
    if ( !svg || !root ) return;

    char *type;

    for (xmlNode *node = root; node; node=node->next) {
        type = (char *) node->name;

        if ( node->type == XML_ELEMENT_NODE ) {
            if ( !strcmp(type, "svg") ) {
                buildSVGimage(svg, node->children, NULL);

                for (xmlAttr *attr = node->properties; attr; attr = attr->next) {
                    Attribute *otherAttr = createAttribute(attr->name, (attr->children)->content);
                    insertBack(svg->otherAttributes, otherAttr);
                }

            } else if ( !strcmp(type, "title") ) {
                xmlNode *title = node->children;
                copyField(svg->title, (char *) title->content);

            } else if ( !strcmp(type, "desc") ) {
                xmlNode *desc = node->children;
                copyField(svg->description, (char *) desc->content);

            } else if ( !strcmp(type, "rect") ) {
                Rectangle *rect = createRectangle();
                createFromNode(rect, node, rectNodeParser);

                ( cGroup ) ? insertBack(cGroup->rectangles, rect) : insertBack(svg->rectangles, rect);

            } else if ( !strcmp(type, "circle") ) {
                Circle *circle = createCircle();
                createFromNode(circle, node, circleNodeParser);

                ( cGroup ) ? insertBack(cGroup->circles, circle) : insertBack(svg->circles, circle);

            } else if ( !strcmp(type, "path") ) {
                Path *path = createPath();
                createFromNode(path, node, pathNodeParser);

                ( cGroup ) ? insertBack(cGroup->paths, path) : insertBack(svg->paths, path);

            } else if ( !strcmp(type, "g") ) {
                Group *group = createGroup();
                createFromNode(group, node, groupNodeParser);

                ( cGroup ) ? insertBack(cGroup->groups, group) : insertBack(svg->groups, group);

                buildSVGimage(svg, node->children, group);
            }
        }
    }
}

bool writeSVGimage(SVGimage* image, char* fileName) {
    if ( !image || !fileName ) return false;

    xmlDoc *doc;
    bool status = true;

    if ( !(doc = SVGimageToDoc(image)) )
        status = false;

    if ( xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1) == -1 )
        status = false;

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return status;
}

char* validateSVG(char *filepath) {
    SVGimage *svg = createValidSVGimage(filepath, "./xsd/svg.xsd");

    if ( validateSVGimage(svg, "./xsd/svg.xsd") )
        return "Valid";
    else
        return "Invalid";
}

bool validateSVGimage(SVGimage* img, char* schemaFile) {
    if ( !img || !schemaFile ) return false;

    xmlDoc *doc = SVGimageToDoc(img);
    bool status = true;

    if ( !doc || !validateXMLDoc(doc, schemaFile) ) {
        xmlFreeDoc(doc);
        return false;
    }

    if ( !validateAttributes(img->otherAttributes) ) status = false;
    if ( !validateRectangles(img->rectangles) ) status = false;
    if ( !validateCircles(img->circles) ) status = false;
    if ( !validatePaths(img->paths) ) status = false;
    if ( !validateGroups(img->groups) ) status = false;

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return status;
}

xmlDoc *SVGimageToDoc(SVGimage* img) {
    if ( !img ) return NULL;

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;
    xmlNs *ns = NULL;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "svg");

    ns = xmlNewNs(root, BAD_CAST "http://www.w3.org/2000/svg", NULL);
    xmlSetNs(root, ns);

    if ( strlen(img->title) )
        xmlNewChild(root, NULL, (xmlChar *) "title", (xmlChar *) img->title);

    if ( strlen(img->description) )
        xmlNewChild(root, NULL, (xmlChar *) "desc", (xmlChar *) img->description);

    xmlDocSetRootElement(doc, root);

    addAttributes(root, img->otherAttributes);
    addRectangles(root, img->rectangles);
    addCircles(root, img->circles);
    addPaths(root, img->paths);
    addGroups(root, img->groups);

    return doc;
}

bool validateXMLDoc(xmlDoc* doc, char* schemaFile) {
    if ( !doc || !schemaFile ) return false;

    xmlSchema *schema = NULL;
    xmlSchemaParserCtxt *pCtxt = NULL;
    xmlSchemaValidCtxt *vCtxt = NULL;
    bool status = true;

    xmlLineNumbersDefault(1);

    if ( !(pCtxt = xmlSchemaNewParserCtxt(schemaFile)) ) {
        status = false;
        goto cleanup;
    }

    if ( !(schema = xmlSchemaParse(pCtxt)) ) {
        status = false;
        goto cleanup;
    }

    if ( !(vCtxt = xmlSchemaNewValidCtxt(schema)) ) {
        status = false;
        goto cleanup;
    }

    if ( xmlSchemaValidateDoc(vCtxt, doc) ) {
        status = false;
        goto cleanup;
    }

    cleanup:
    if (schema) xmlSchemaFree(schema);
    if (pCtxt) xmlSchemaFreeParserCtxt(pCtxt);
    if (vCtxt) xmlSchemaFreeValidCtxt(vCtxt);
    xmlSchemaCleanupTypes();
    xmlMemoryDump();

    return status;
}

void setAttribute(SVGimage* image, elementType type, int elemIndex, Attribute* newAttribute) {
    if ( !image || !newAttribute ) return;
    
    switch ( type ) {
        case SVG_IMAGE:
            updateSVGimageAttributes(image, newAttribute);
            break;

        case RECT: {
            Rectangle *r = listAt(image->rectangles, elemIndex);
            if ( r ) updateRectangleAttributes(r, newAttribute);

        } break;

        case CIRC: {
            Circle *c = listAt(image->circles, elemIndex);
            if ( c ) updateCircleAttributes(c, newAttribute);

        } break;

        case PATH: {
            Path *p = listAt(image->paths, elemIndex);
            if ( p ) updatePathAttributes(p, newAttribute);

        } break;

        case GROUP: {
            Group *g = listAt(image->groups, elemIndex);
            if ( g ) updateGroupAttributes(g, newAttribute);

        } break;
    }
}

void addComponent(SVGimage* image, elementType type, void* newElement) {
    if ( !image || !newElement ) return;
    
    switch ( type ) {
        case RECT:
            insertBack(image->rectangles, newElement);
            break;

        case CIRC:
            insertBack(image->circles, newElement);
            break;

        case PATH:
            insertBack(image->paths, newElement);
            break;

        default: 
            break;
    }
}

//   ######  ##     ##  ######   #### ##     ##    ###     ######   ######## 
//  ##    ## ##     ## ##    ##   ##  ###   ###   ## ##   ##    ##  ##       
//  ##       ##     ## ##         ##  #### ####  ##   ##  ##        ##       
//   ######  ##     ## ##   ####  ##  ## ### ## ##     ## ##   #### ######   
//        ##  ##   ##  ##    ##   ##  ##     ## ######### ##    ##  ##       
//  ##    ##   ## ##   ##    ##   ##  ##     ## ##     ## ##    ##  ##       
//   ######     ###     ######   #### ##     ## ##     ##  ######   ######## 

char* SVGimageToString(SVGimage* img) {
    if ( !img ) return NULL;

    char *rectangles = toString(img->rectangles, "\n");
    char *circles = toString(img->circles, "\n");
    char *paths = toString(img->paths, "\n");
    char *groups = toString(img->groups, "\n");
    char *attributes = toString(img->otherAttributes, " ");

    int length = strlen(attributes) +
                 strlen(rectangles) +
                 strlen(circles) +
                 strlen(paths) +
                 strlen(groups) +
                 strlen(img->title) +
                 strlen(img->description) +
                 strlen(img->namespace);

    char *str = malloc(sizeof(char) * (length + 200));

    if ( !str ) return NULL;

    sprintf(str, "\n<svg%s xmlns=\"%s\" > \
                  \n<title> %s </title> \
                  \n<desc> %s </desc> \
                  \n%s%s%s%s \
                  \n</svg>\n",
                attributes, img->namespace, img->title, img->description, 
                rectangles, circles, paths, groups
            );

    free(rectangles);
    free(circles);
    free(paths);
    free(groups);
    free(attributes);

    return str;
}

void deleteSVGimage(SVGimage* img) {
    if ( !img ) return;

    freeList(img->rectangles);
    freeList(img->circles);
    freeList(img->paths);
    freeList(img->groups);
    freeList(img->otherAttributes);

    free(img);
}



//     ###    ######## ######## ########  #### ########  ##     ## ######## ########  ######  
//    ## ##      ##       ##    ##     ##  ##  ##     ## ##     ##    ##    ##       ##    ## 
//   ##   ##     ##       ##    ##     ##  ##  ##     ## ##     ##    ##    ##       ##       
//  ##     ##    ##       ##    ########   ##  ########  ##     ##    ##    ######    ######  
//  #########    ##       ##    ##   ##    ##  ##     ## ##     ##    ##    ##             ## 
//  ##     ##    ##       ##    ##    ##   ##  ##     ## ##     ##    ##    ##       ##    ## 
//  ##     ##    ##       ##    ##     ## #### ########   #######     ##    ########  ######  

char* attributeToString(void* data) {
    if ( !data ) return NULL;

    Attribute *attr = (Attribute *) data;
    int len = strlen(attr->name) + strlen(attr->value) + 50;
    char *str = malloc( len * sizeof(char) );

    if ( !str ) return NULL;

    sprintf(str, "%s=\"%s\"", attr->name, attr->value);

    return str;
}

void deleteAttribute(void* data) {
    if ( !data ) return;

    Attribute *attr = (Attribute *) data;

    free(attr->name);
    free(attr->value);

    free(attr);
}

int compareAttributes(const void *first, const void *second) {
    if ( !first || !second ) return 1;

    Attribute *attr1 = (Attribute *) first;
    Attribute *attr2 = (Attribute *) second;

    int c1 = !strcmp(attr1->name, attr2->name);
    int c2 = !strcmp(attr1->value, attr2->value);

    return (c1 && c2);
}



//   ######   ########   #######  ##     ## ########   ######  
//  ##    ##  ##     ## ##     ## ##     ## ##     ## ##    ## 
//  ##        ##     ## ##     ## ##     ## ##     ## ##       
//  ##   #### ########  ##     ## ##     ## ########   ######  
//  ##    ##  ##   ##   ##     ## ##     ## ##              ## 
//  ##    ##  ##    ##  ##     ## ##     ## ##        ##    ## 
//   ######   ##     ##  #######   #######  ##         ######  

char* groupToString(void* data) {
    if ( !data ) return NULL;

    Group *group = (Group *) data;

    char *rectangles = toString(group->rectangles, "\n    ");
    char *circles = toString(group->circles, "\n    ");
    char *paths = toString(group->paths, "\n    ");
    char *groups = toString(group->groups, "\n");
    char *attributes = toString(group->otherAttributes, " ");

    int length = strlen(rectangles) +
                 strlen(circles) +
                 strlen(paths) +
                 strlen(attributes) +
                 strlen(groups);

    char *str = malloc(sizeof(char) * (length + 200));

    if ( !str ) return NULL;

    sprintf(str, "<g%s > %s %s %s %s \n</g>\n",
                attributes,
                rectangles,
                circles,
                paths,
                groups
            );

    free(rectangles);
    free(circles);
    free(paths);
    free(groups);
    free(attributes);

    return str;
}

void deleteGroup(void* data) {
    if ( !data ) return;

    Group *group = (Group *) data;

    freeList(group->rectangles);
    freeList(group->circles);
    freeList(group->paths);
    freeList(group->groups);
    freeList(group->otherAttributes);

    free(group);
}

int compareGroups(const void *first, const void *second) {
    if ( !first || !second ) return 1;

    Group *g1 = (Group *) first;
    Group *g2 = (Group *) second;

    long sum1 = getLength(g1->rectangles) +
                getLength(g1->circles) +
                getLength(g1->paths) +
                getLength(g1->groups);

    long sum2 = getLength(g2->rectangles) +
                getLength(g2->circles) +
                getLength(g2->paths) +
                getLength(g2->groups);

    return sum1 - sum2;
}



//  ########  ########  ######  ########    ###    ##    ##  ######   ##       ########  ######  
//  ##     ## ##       ##    ##    ##      ## ##   ###   ## ##    ##  ##       ##       ##    ## 
//  ##     ## ##       ##          ##     ##   ##  ####  ## ##        ##       ##       ##       
//  ########  ######   ##          ##    ##     ## ## ## ## ##   #### ##       ######    ######  
//  ##   ##   ##       ##          ##    ######### ##  #### ##    ##  ##       ##             ## 
//  ##    ##  ##       ##    ##    ##    ##     ## ##   ### ##    ##  ##       ##       ##    ## 
//  ##     ## ########  ######     ##    ##     ## ##    ##  ######   ######## ########  ######  

char* rectangleToString(void* data) {
    if ( !data ) return NULL;

    Rectangle *rect = (Rectangle *) data;
    char *attributes = toString(rect->otherAttributes, " ");

    int len = strlen(rect->x) + 
              strlen(rect->y) + 
              strlen(rect->width) + 
              strlen(rect->height) +
              strlen(attributes);

    char *str = malloc(sizeof(char) * (100 + len));

    if ( !str ) return NULL;

    sprintf(str, "<rect x=\"%s\" y=\"%s\" width=\"%s\" height=\"%s\"%s \\>", 
                rect->x,
                rect->y,
                rect->width,
                rect->height,
                attributes
            );

    free(attributes);

    return str;
}

void deleteRectangle(void* data) {
    if ( !data ) return;

    Rectangle *rect = (Rectangle *) data;

    free(rect->x);
    free(rect->y);
    free(rect->width);
    free(rect->height);

    freeList(rect->otherAttributes);

    free(rect);
}

int compareRectangles(const void *first, const void *second) {
    if ( !first || !second ) return 1;

    return 1;
}



//   ######  #### ########   ######  ##       ########  ######  
//  ##    ##  ##  ##     ## ##    ## ##       ##       ##    ## 
//  ##        ##  ##     ## ##       ##       ##       ##       
//  ##        ##  ########  ##       ##       ######    ######  
//  ##        ##  ##   ##   ##       ##       ##             ## 
//  ##    ##  ##  ##    ##  ##    ## ##       ##       ##    ## 
//   ######  #### ##     ##  ######  ######## ########  ######  

char* circleToString(void* data) {
    if ( !data ) return NULL;

    Circle *circle = (Circle *) data;
    char *attributes = toString(circle->otherAttributes, " ");

    int len = strlen(circle->cx) + 
              strlen(circle->cy) + 
              strlen(circle->r) + 
              strlen(attributes);

    char *str = malloc(sizeof(char) * (100 + len));

    if ( !str ) return NULL;

    sprintf(str, "<circle cx=\"%s\" cy=\"%s\" r=\"%s\"%s \\>", 
                circle->cx,
                circle->cy,
                circle->r,
                attributes
            );

    free(attributes);

    return str;
}

void deleteCircle(void* data) {
    if ( !data ) return;

    Circle *circle = (Circle *) data;

    free(circle->cx);
    free(circle->cy);
    free(circle->r);
    freeList(circle->otherAttributes);

    free(circle);
}

int compareCircles(const void *first, const void *second) {
    if ( !first || !second ) return 1;

    return 1;
}



//  ########     ###    ######## ##     ##  ######  
//  ##     ##   ## ##      ##    ##     ## ##    ## 
//  ##     ##  ##   ##     ##    ##     ## ##       
//  ########  ##     ##    ##    #########  ######  
//  ##        #########    ##    ##     ##       ## 
//  ##        ##     ##    ##    ##     ## ##    ## 
//  ##        ##     ##    ##    ##     ##  ######  

char* pathToString(void* data) {
    if ( !data ) return NULL;

    Path *path = (Path *) data;
    char *attributes = toString(path->otherAttributes, " ");
    char *str = malloc(sizeof(char) * (100 + strlen(path->data) + strlen(attributes)));

    if ( !str ) return NULL;

    sprintf(str, "<path d=\"%s\"%s \\>", path->data, attributes);

    free(attributes);

    return str;
}

void deletePath(void* data) {
    if ( !data ) return;

    Path *path = (Path *) data;

    free(path->data);
    freeList(path->otherAttributes);

    free(path);
}

int comparePaths(const void *first, const void *second) {
    if ( !first || !second ) return 1;

    Path *path1 = (Path *) first;
    Path *path2 = (Path *) second;

    return strlen(path1->data) - strlen(path2->data);
}



 
//   ######   ######## ######## ######## ######## ########   ######  
//  ##    ##  ##          ##       ##    ##       ##     ## ##    ## 
//  ##        ##          ##       ##    ##       ##     ## ##       
//  ##   #### ######      ##       ##    ######   ########   ######  
//  ##    ##  ##          ##       ##    ##       ##   ##         ## 
//  ##    ##  ##          ##       ##    ##       ##    ##  ##    ## 
//   ######   ########    ##       ##    ######## ##     ##  ######  
 

List* getRects(SVGimage* img) {
    if ( !img ) return NULL;

    List *rectangles = initializeList(rectangleToString, dummyDelete, compareRectangles);

    copyList(img->rectangles, rectangles);
    getFromGroups(img->groups, rectangles, RECT);

    return rectangles;
}

List* getCircles(SVGimage* img) {
    if ( !img ) return NULL;

    List *circles = initializeList(circleToString, dummyDelete, compareCircles);

    copyList(img->circles, circles);
    getFromGroups(img->groups, circles, CIRC);

    return circles;
}

List* getGroups(SVGimage* img) {
    if ( !img ) return NULL;

    List *groups = initializeList(groupToString, dummyDelete, compareGroups);

    copyList(img->groups, groups);
    getFromGroups(img->groups, groups, GROUP);

    return groups;
}

List* getPaths(SVGimage* img) {
    if ( !img ) return NULL;
    
    List *paths = initializeList(pathToString, dummyDelete, comparePaths);

    copyList(img->paths, paths);
    getFromGroups(img->groups, paths, PATH);

    return paths;
}



//  ##    ## ##     ## ##     ##    ##      ##       ## 
//  ###   ## ##     ## ###   ###    ##  ##  ##      ##  
//  ####  ## ##     ## #### ####    ##  ##  ##     ##   
//  ## ## ## ##     ## ## ### ##    ##  ##  ##    ##    
//  ##  #### ##     ## ##     ##    ##  ##  ##   ##     
//  ##   ### ##     ## ##     ##    ##  ##  ##  ##      
//  ##    ##  #######  ##     ##     ###  ###  ##       

int numRectsWithArea(SVGimage* img, float area) {
    if ( !img ) return 0;

    List *found = initializeList(rectangleToString, dummyDelete, compareRectangles);

    return searchSVGImage(getRects(img), found, &area, compRectangleArea);
}

int numCirclesWithArea(SVGimage* img, float area) {
    if ( !img ) return 0;

    List *found = initializeList(circleToString, dummyDelete, compareCircles);

    return searchSVGImage(getCircles(img), found, &area, compCircleArea);
}

int numPathsWithdata(SVGimage* img, char* data) {
    if ( !img || !data ) return 0;

    List *found = initializeList(pathToString, dummyDelete, comparePaths);

    return searchSVGImage(getPaths(img), found, data, compPathData);
}

int numGroupsWithLen(SVGimage* img, int len) {
    if ( !img ) return 0;

    List *found = initializeList(groupToString, dummyDelete, compareGroups);

    return searchSVGImage(getGroups(img), found, &len, compGroupLength);
}

int numAttr(SVGimage* img) {
    if ( !img ) return 0;

    long total = getLength(img->otherAttributes);
    
    List *rectangles = getRects(img);
    List *circles = getCircles(img);
    List *paths = getPaths(img);
    List *groups = getGroups(img);

    ListIterator iter;
    Rectangle *r;
    Circle *c;
    Path *p;
    Group *g;

    iter = createIterator(rectangles);
    while ( (r = nextElement(&iter)) )
        total += getLength(r->otherAttributes);

    iter = createIterator(circles);
    while ( (c = nextElement(&iter)) )
        total += getLength(c->otherAttributes);

    iter = createIterator(paths);
    while ( (p = nextElement(&iter)) )
        total += getLength(p->otherAttributes);

    iter = createIterator(groups);
    while ( (g = nextElement(&iter)) )
        total += getLength(g->otherAttributes);

    freeList(rectangles);
    freeList(circles);
    freeList(paths);
    freeList(groups);

    return total;
}



//   ######  ######## ########  ##     ##  ######  ########    #### ##    ## #### ########     
//  ##    ##    ##    ##     ## ##     ## ##    ##    ##        ##  ###   ##  ##     ##        
//  ##          ##    ##     ## ##     ## ##          ##        ##  ####  ##  ##     ##        
//   ######     ##    ########  ##     ## ##          ##        ##  ## ## ##  ##     ##        
//        ##    ##    ##   ##   ##     ## ##          ##        ##  ##  ####  ##     ##        
//  ##    ##    ##    ##    ##  ##     ## ##    ##    ##        ##  ##   ###  ##     ##    ### 
//   ######     ##    ##     ##  #######   ######     ##       #### ##    ## ####    ##    ### 

SVGimage *initSVGimage() {
    SVGimage *svg = malloc(sizeof(SVGimage));

    if ( !svg ) return NULL;

    svg->rectangles = rectangleList();
    svg->circles = circleList();
    svg->paths = pathList();
    svg->groups = groupList();
    svg->otherAttributes = attributeList();

    *(svg->namespace) = 0;
    *(svg->title) = 0;
    *(svg->description) = 0;

    return svg;
}

Attribute *createAttribute(const void *name, const void *value) {
    Attribute *attr = malloc(sizeof(Attribute));
    
    if ( !attr ) return NULL;

    attr->name = calloc(strlen(name) + 1, sizeof(char));
    attr->value = calloc(strlen(value) + 1, sizeof(char));

    if ( !attr->name || !attr->value ) return NULL;
    
    strcpy(attr->name, (char *) name);
    strcpy(attr->value, (char *) value);

    return attr;
}

Rectangle *createRectangle() {
    Rectangle *rect = malloc(sizeof(Rectangle));

    if ( !rect ) return NULL;

    rect->x = calloc(50, sizeof(char));
    rect->y = calloc(50, sizeof(char));
    rect->width = calloc(50, sizeof(char));
    rect->height = calloc(50, sizeof(char));
    rect->otherAttributes = attributeList();

    return rect;
}

Circle *createCircle() {
    Circle *circle = malloc(sizeof(Circle));

    if ( !circle ) return NULL;

    circle->cx = calloc(50, sizeof(char));
    circle->cy = calloc(50, sizeof(char));
    circle->r = calloc(50, sizeof(char));
    circle->otherAttributes = attributeList();

    return circle;
}

Path *createPath() {
    Path *path = malloc(sizeof(Path));

    if ( !path ) return NULL;

    path->data = malloc(sizeof(char));
	*(path->data) = 0;

    path->otherAttributes = attributeList();

    return path;
}

Group *createGroup() {
    Group *group = malloc(sizeof(Group));

    if ( !group ) return NULL;

    group->rectangles = rectangleList();
    group->circles = circleList();
    group->paths = pathList();
    group->groups = groupList();
    group->otherAttributes = attributeList();

    return group;
}



//  ##       ####  ######  ########    #### ##    ## #### ########     
//  ##        ##  ##    ##    ##        ##  ###   ##  ##     ##        
//  ##        ##  ##          ##        ##  ####  ##  ##     ##        
//  ##        ##   ######     ##        ##  ## ## ##  ##     ##        
//  ##        ##        ##    ##        ##  ##  ####  ##     ##        
//  ##        ##  ##    ##    ##        ##  ##   ###  ##     ##    ### 
//  ######## ####  ######     ##       #### ##    ## ####    ##    ### 

List *rectangleList() { 
    return initializeList(rectangleToString, deleteRectangle, compareRectangles);
}

List *circleList() { 
    return initializeList(circleToString, deleteCircle, compareCircles);
}

List *pathList() { 
    return initializeList(pathToString, deletePath, comparePaths);
}

List *groupList() { 
    return initializeList(groupToString, deleteGroup, compareGroups);
}

List *attributeList() { 
    return initializeList(attributeToString, deleteAttribute, compareAttributes);
}



//  ########   #######   ######     #### ##    ## #### ########     
//  ##     ## ##     ## ##    ##     ##  ###   ##  ##     ##        
//  ##     ## ##     ## ##           ##  ####  ##  ##     ##        
//  ##     ## ##     ## ##           ##  ## ## ##  ##     ##        
//  ##     ## ##     ## ##           ##  ##  ####  ##     ##        
//  ##     ## ##     ## ##    ##     ##  ##   ###  ##     ##    ### 
//  ########   #######   ######     #### ##    ## ####    ##    ### 

char *propValue(float f, char *units) {
    char *prop = malloc(sizeof(char) * (strlen(units) + 20));
    
    sprintf(prop, "%f%s", (float) f, units);

    return prop;
}

void newAttribute(xmlNode *node, char* name, char* value) {
    if ( !node ) return;

    xmlSetProp(node, (xmlChar *) name, (xmlChar*) value);
    // free(value);
}

void addAttributes(xmlNode *node, List *attrs) {
    if ( !node || !attrs ) return;

    ListIterator iter = createIterator(attrs);
    Attribute *attr;
    
    while ( (attr = nextElement(&iter)) )
        xmlSetProp(node, (xmlChar *) attr->name, (xmlChar *) attr->value);
}

void addRectangles(xmlNode* root, List *rectList) {
    if (!root || !rectList) return;
    
    ListIterator iter = createIterator(rectList);
    Rectangle *rect;
    xmlNode *node;

    while ( (rect = nextElement(&iter)) ) {
        node = xmlNewChild(root, NULL, (xmlChar *) "rect", NULL);

        newAttribute(node, "x", rect->x);
        newAttribute(node, "y", rect->y);
        newAttribute(node, "width", rect->width);
        newAttribute(node, "height", rect->height);
        addAttributes(node, rect->otherAttributes);
    }
}

void addCircles(xmlNode* root, List *circleList) {
    if (!root || !circleList) return;
    
    ListIterator iter = createIterator(circleList);
    Circle *circle;
    xmlNode *node;

    while ( (circle = nextElement(&iter)) ) {
        node = xmlNewChild(root, NULL, (xmlChar *) "circle", NULL);

        newAttribute(node, "cx", circle->cx);
        newAttribute(node, "cy", circle->cy);
        newAttribute(node, "r", circle->r);
        addAttributes(node, circle->otherAttributes);
    }
}

void addPaths(xmlNode* root, List *pathList) {
    if (!root || !pathList) return;
    
    ListIterator iter = createIterator(pathList);
    Path *path;
    xmlNode *node;

    while ( (path = nextElement(&iter)) ) {
        node = xmlNewChild(root, NULL, (xmlChar *) "path", NULL);

        xmlSetProp(node, (xmlChar *) "d", (xmlChar*) path->data);
        addAttributes(node, path->otherAttributes);
    }
}

void addGroups(xmlNode* root, List *groupList) {
    if (!root || !groupList) return;

    ListIterator iter = createIterator(groupList);
    Group *group;
    xmlNode *node;

    while ( (group = nextElement(&iter)) ) {
        node = xmlNewChild(root, NULL, (xmlChar *) "g", NULL);

        addRectangles(node, group->rectangles);
        addCircles(node, group->circles);
        addPaths(node, group->paths);
        addGroups(node, group->groups);
        addAttributes(node, group->otherAttributes);
    }
}



//  ##     ##    ###    ##       #### ########     ###    ######## ####  #######  ##    ## 
//  ##     ##   ## ##   ##        ##  ##     ##   ## ##      ##     ##  ##     ## ###   ## 
//  ##     ##  ##   ##  ##        ##  ##     ##  ##   ##     ##     ##  ##     ## ####  ## 
//  ##     ## ##     ## ##        ##  ##     ## ##     ##    ##     ##  ##     ## ## ## ## 
//   ##   ##  ######### ##        ##  ##     ## #########    ##     ##  ##     ## ##  #### 
//    ## ##   ##     ## ##        ##  ##     ## ##     ##    ##     ##  ##     ## ##   ### 
//     ###    ##     ## ######## #### ########  ##     ##    ##    ####  #######  ##    ## 

bool validateAttributes(List *attributes) {
    if ( !attributes ) return false;

    ListIterator iter = createIterator(attributes);
    Attribute *attr;

    while ( (attr = nextElement(&iter)) ) {
        if ( !attr->name ) return false;
        if ( !attr->value ) return false;
    }

    return true;
}

bool validateRectangles(List *rectangles) {
    if ( !rectangles ) return false;

    ListIterator iter = createIterator(rectangles);
    Rectangle *rect;

    while ( (rect = nextElement(&iter)) ) {
        if ( rect->width < 0 ) return false;
        if ( rect->height < 0 ) return false;
        if ( !validateAttributes(rect->otherAttributes) ) return false;
    }

    return true;
}

bool validateCircles(List *circles) {
    if ( !circles ) return false;

    ListIterator iter = createIterator(circles);
    Circle *circle;

    while ( (circle = nextElement(&iter)) ) {
        if ( circle->r < 0 ) return false;
        if ( !validateAttributes(circle->otherAttributes) ) return false;
    }

    return true;
}
bool validatePaths(List *paths) {
    if ( !paths ) return false;

    ListIterator iter = createIterator(paths);
    Path *path;

    while ( (path = nextElement(&iter)) ) {
        if ( !path->data ) return false;
        if ( !validateAttributes(path->otherAttributes) ) return false;
    }

    return true;
}

bool validateGroups(List *groups) {
    if ( !groups ) return false;

    ListIterator iter = createIterator(groups);
    Group *group;

    while ( (group = nextElement(&iter)) ) {
        if ( !group->rectangles ) return false;
        if ( !group->circles ) return false;
        if ( !group->paths ) return false;
        if ( !validateAttributes(group->otherAttributes) ) return false;
        if ( !validateGroups(group->groups) ) return false;
    }

    return true;
}



//  ##    ##  #######  ########  ########    ########     ###    ########   ######  #### ##    ##  ######   
//  ###   ## ##     ## ##     ## ##          ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##  
//  ####  ## ##     ## ##     ## ##          ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##        
//  ## ## ## ##     ## ##     ## ######      ########  ##     ## ########   ######   ##  ## ## ## ##   #### 
//  ##  #### ##     ## ##     ## ##          ##        ######### ##   ##         ##  ##  ##  #### ##    ##  
//  ##   ### ##     ## ##     ## ##          ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##  
//  ##    ##  #######  ########  ########    ##        ##     ## ##     ##  ######  #### ##    ##  ######   

void setSVGNamespace(SVGimage *svg, xmlDoc *doc, xmlNode *root) {
    if ( !svg || !doc || !root) return;

    xmlNs **namespace = xmlGetNsList(doc, root);

    if ( namespace ) 
        copyField(svg->namespace, (char *) (*namespace)->href);

    free(namespace);
}

void createFromNode(void *element, xmlNode *node, void (*nodeParser)(void*, char*, char*)) {
    if ( !element || !node || !nodeParser ) return;

    for (xmlAttr *attr = node->properties; attr; attr = attr->next) {
        char *name = (char *) attr->name;
        char *value = (char *) (attr->children)->content;

        nodeParser(element, name, value);
    }
}

void groupNodeParser(void *element, char *name, char *value) {
    if ( !element ) return;

    Group *group = (Group *) element;

    Attribute *otherAttr = createAttribute(name, value);
    insertBack(group->otherAttributes, otherAttr);
}

void rectNodeParser(void *element, char *name, char *value) {
    if ( !element ) return;

    Rectangle *rect = (Rectangle *) element;
    char *val_cpy = malloc(sizeof(char) * (strlen(value) + 1));

    strcpy(val_cpy, value);

    if ( !strcmp(name, "x") ) {
        rect->x = val_cpy;

    } else if ( !strcmp(name, "y") ) {
        rect->y = val_cpy;

    } else if ( !strcmp(name, "width") ) {
        rect->width = val_cpy;
        
    } else if ( !strcmp(name, "height") ) {
        rect->height = val_cpy;

    } else {
        Attribute *otherAttr = createAttribute(name, value);
        insertBack(rect->otherAttributes, otherAttr);
        free(val_cpy);
    }
}

void circleNodeParser(void *element, char *name, char *value) {
    if ( !element ) return;

    Circle *circle = (Circle *) element;
    char *val_cpy = malloc(sizeof(char) * (strlen(value) + 1));

    strcpy(val_cpy, value);

    if ( !strcmp(name, "cx") ) {
        circle->cx = val_cpy;

    } else if ( !strcmp(name, "cy") ) {
        circle->cy = val_cpy;

    } else if ( !strcmp(name, "r") ) {
        circle->r = val_cpy;
        
    } else {
        Attribute *otherAttr = createAttribute(name, value);
        insertBack(circle->otherAttributes, otherAttr);
        free(val_cpy);
    }
}

void pathNodeParser(void *element, char *name, char *value) {
    if ( !element ) return;

    Path *path = (Path *) element;

    if ( !strcmp(name, "d") ) {
        free(path->data);
        
        if ( !(path->data = malloc(strlen(value) + 1)) ) 
            return;
        
        strcpy(path->data, value);

    } else {
        Attribute *otherAttr = createAttribute(name, value);
        insertBack(path->otherAttributes, otherAttr);
    }
}



//   ######  ########    ###    ########   ######  ##     ##    ######## ##    ##  ######  
//  ##    ## ##         ## ##   ##     ## ##    ## ##     ##    ##       ###   ## ##    ## 
//  ##       ##        ##   ##  ##     ## ##       ##     ##    ##       ####  ## ##       
//   ######  ######   ##     ## ########  ##       #########    ######   ## ## ##  ######  
//        ## ##       ######### ##   ##   ##       ##     ##    ##       ##  ####       ## 
//  ##    ## ##       ##     ## ##    ##  ##    ## ##     ##    ##       ##   ### ##    ## 
//   ######  ######## ##     ## ##     ##  ######  ##     ##    ##       ##    ##  ######  

int searchSVGImage(List *pool, List *found, const void* data, int (*compare)(const void*, const void*)) {
    findAll(pool, found, data, compare);

    int total = getLength(found);

    freeList(pool);
    freeList(found);

    return total;
}

void findAll(List *list, List *found, const void* record, int (*compare)(const void*, const void*)) {
	if ( !record && compare ) return;
    
    ListIterator itr = createIterator(list);
	void* data;

	while ( (data = nextElement(&itr)) )
		if ( !compare || compare(data, record) )
			insertBack(found, data);
}

void getFromGroups(List *groups, List *found, elementType type) {
    Group *group;
    ListIterator iter = createIterator(groups);

    while ( (group = nextElement(&iter)) ) {
        switch ( type ) {
            case RECT:
                copyList(group->rectangles, found); break;

            case CIRC:
                copyList(group->circles, found); break;

            case PATH:
                copyList(group->paths, found); break;

            case GROUP:
                copyList(group->groups, found); break;

            default:
                break;
        }

        getFromGroups(group->groups, found, type);
    }
}

int compRectangleArea(const void *first, const void *second) {
    return 1;
}


int compGroupLength(const void *first, const void *second) {
    if ( !first || !second ) return 0;

    Group *g = (Group *) first;
    int length = *( (int*) second );

    long groupLength = getLength(g->rectangles) +
                       getLength(g->circles) +
                       getLength(g->paths) +
                       getLength(g->groups);

    return length == groupLength;
}

int compCircleArea(const void *first, const void *second) {
    return 1;
}

int compPathData(const void *first, const void *second) {
    if ( !first || !second ) return 0;

    Path *path = (Path *) first;
    char *data = (char *) second;

    return !strcmp(data, path->data);
}



//  ########  #######           ##  ######   #######  ##    ## 
//     ##    ##     ##          ## ##    ## ##     ## ###   ## 
//     ##    ##     ##          ## ##       ##     ## ####  ## 
//     ##    ##     ##          ##  ######  ##     ## ## ## ## 
//     ##    ##     ##    ##    ##       ## ##     ## ##  #### 
//     ##    ##     ##    ##    ## ##    ## ##     ## ##   ### 
//     ##     #######      ######   ######   #######  ##    ##  

char *empty(char *type) {
    char *str = malloc(sizeof(char) * 3);
    
    strcpy(str, type);
    
    return str;
}

char* fileToJSON(char *filepath) {
    SVGimage *svg = createValidSVGimage(filepath, "./xsd/svg.xsd");

    char *json = SVGtoJSON(svg);
    deleteSVGimage(svg);

    return json;
}

char* fileToSummaryJSON(char *filepath) {
    SVGimage *svg = createValidSVGimage(filepath, "./xsd/svg.xsd");

    if ( !svg ) return "Invalid SVG";

    List *rects = getRects(svg);
    List *circles = getCircles(svg);
    List *groups = getGroups(svg);
    List *paths = getPaths(svg);

    char *str = malloc(sizeof(char) * 150);

    sprintf(str, "{\"numRects\":%d,\"numCircles\":%d,\"numPaths\":%d,\"numGroups\":%d}",
            getLength(rects), getLength(circles), getLength(paths), getLength(groups)
           );

    deleteSVGimage(svg);

    freeList(rects);
    freeList(circles);
    freeList(groups);
    freeList(paths);

    return str;
}

char* SVGtoJSON(const SVGimage* img) {
    if ( !img ) return empty("{}");

    char *rects = rectListToJSON(img->rectangles);
    char *circles = circListToJSON(img->circles);
    char *paths = pathListToJSON(img->paths);
    char *groups = groupListToJSON(img->groups);
    char *otherAttributes = attrListToJSON(img->otherAttributes);

    int len = strlen(img->title)
            + strlen(img->description)
            + strlen(rects)
            + strlen(circles)
            + strlen(paths)
            + strlen(groups)
            + strlen(otherAttributes)
            + 100;

    char *str = malloc(sizeof(char) * len);

    sprintf(str, "{\"title\":\"%s\",\"description\":\"%s\",\"rectangles\":%s,\"circles\":%s,\"paths\":%s,\"groups\":%s,\"otherAttributes\":%s}", 
            img->title, img->description, rects, circles, paths, groups, otherAttributes
           );

    free(rects);
    free(circles);
    free(paths);
    free(groups);
    free(otherAttributes);
    
    return str;
}

char* attrToJSON(const Attribute *a) {
    if ( !a ) return empty("{}");

    int len = strlen(a->name) + strlen(a->value) + 25;
    char *str = malloc(sizeof(char) * len);

    sprintf(str, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
    
    return str;
}

char* rectToJSON(const Rectangle *r) {
    if ( !r ) return empty("{}");

    char *otherAttributes = attrListToJSON(r->otherAttributes);

    int len = strlen(r->x) + 
              strlen(r->y) + 
              strlen(r->width) + 
              strlen(r->height) +
              strlen(otherAttributes);

    char *str = malloc(sizeof(char) * (100 + len));

    sprintf(str, "{\"x\":\"%s\",\"y\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"otherAttributes\":%s}", 
            r->x, r->y, r->width, r->height, otherAttributes
           );

    free(otherAttributes);

    return str;
}

char* circleToJSON(const Circle *c) {
    if ( !c ) return empty("{}");

    char *otherAttributes = attrListToJSON(c->otherAttributes);

    int len = strlen(c->cx) + 
              strlen(c->cy) + 
              strlen(c->r) + 
              strlen(otherAttributes);

    char *str = malloc(sizeof(char) * (150 + len));

    sprintf(str, "{\"cx\":\"%s\",\"cy\":\"%s\",\"r\":\"%s\",\"otherAttributes\":%s}", 
            c->cx, c->cy, c->r, otherAttributes
           );

    free(otherAttributes);

    return str;
}

char* pathToJSON(const Path *p) {
    if ( !p ) return empty("{}");

    char *otherAttributes = attrListToJSON(p->otherAttributes);
    int len = strlen(p->data) + strlen(otherAttributes) + 50;
    char *str = malloc(sizeof(char) * len);

    sprintf(str, "{\"d\":\"%.64s\",\"otherAttributes\":%s}", p->data, otherAttributes);

    free(otherAttributes);

    return str;
}

char* groupToJSON(const Group *g) {
    if ( !g ) return empty("{}");

    char *rects = rectListToJSON(g->rectangles);
    char *circles = circListToJSON(g->circles);
    char *paths = pathListToJSON(g->paths);
    char *groups = groupListToJSON(g->groups);
    char *otherAttributes = attrListToJSON(g->otherAttributes);

    int len = strlen(rects) + 
              strlen(circles) + 
              strlen(paths) + 
              strlen(groups) + 
              strlen(otherAttributes);
              
    char *str = malloc(sizeof(char) * (len + 150));
    sprintf(str, "{\"rectangles\":%s,\"circles\":%s,\"paths\":%s,\"groups\":%s,\"otherAttributes\":%s}", rects, circles, paths, groups, otherAttributes);
    
    free(rects);
    free(circles);
    free(paths);
    free(groups);
    free(otherAttributes);

    return str;
}

char* listToJSON(char *str, char *json[], int n) {
    strcat(str, "[");
    
    for (int i = 0; i < n; i++) {
        strcat(str, json[i]);
        
        if (i != n-1) 
            strcat(str, ",");
        
        free(json[i]);
    }

    strcat(str, "]");

    return str;
}

char* attrListToJSON(const List *list) {
    if ( !list ) return empty("[]");

    int numA = getLength((List*) list);
    char *attrs[numA];
    int len = 0;

    ListIterator iter = createIterator((List*) list);
    Attribute *a;

    for (int i = 0; (a = nextElement(&iter)); i++) {
        attrs[i] = attrToJSON(a);
        len += strlen(attrs[i]);
    }

    return listToJSON(calloc(len + 50, sizeof(char)), attrs, numA);
}

char* circListToJSON(const List *list) {
    if ( !list ) return empty("[]");

    int numC = getLength((List*) list);
    char *circles[numC];
    int len = 0;

    ListIterator iter = createIterator((List*) list);
    Circle *c;

    for (int i = 0; (c = nextElement(&iter)); i++) {
        circles[i] = circleToJSON(c);
        len += strlen(circles[i]);
    }

    return listToJSON(calloc(len + 50, sizeof(char)), circles, numC);
}

char* rectListToJSON(const List *list) {
    if ( !list ) return empty("[]");

    int numR = getLength((List*) list);
    char *rects[numR];
    int len = 0;

    ListIterator iter = createIterator((List*) list);
    Rectangle *r;

    for (int i = 0; (r = nextElement(&iter)); i++) {
        rects[i] = rectToJSON(r);
        len += strlen(rects[i]);
    }

    return listToJSON(calloc(len + 50, sizeof(char)), rects, numR);
}

char* pathListToJSON(const List *list) {
    if ( !list ) return empty("[]");

    int numP = getLength((List*) list);
    char *paths[numP];
    int len = 0;

    ListIterator iter = createIterator((List*) list);
    Path *p;

    for (int i = 0; (p = nextElement(&iter)); i++) {
        paths[i] = pathToJSON(p);
        len += strlen(paths[i]) + 25;
    }

    return listToJSON(calloc(len + 50, sizeof(char)), paths, numP);
}

char* groupListToJSON(const List *list) {
    if ( !list ) return empty("[]");

    int numG = getLength((List*) list);
    char *groups[numG];
    int len = 0;

    ListIterator iter = createIterator((List*) list);
    Group *g;

    for (int i = 0; (g = nextElement(&iter)); i++) {
        groups[i] = groupToJSON(g);
        len += strlen(groups[i]);
    }

    return listToJSON(calloc(len + 50, sizeof(char)), groups, numG);
}



//  ######## ########   #######  ##     ##          ##  ######   #######  ##    ## 
//  ##       ##     ## ##     ## ###   ###          ## ##    ## ##     ## ###   ## 
//  ##       ##     ## ##     ## #### ####          ## ##       ##     ## ####  ## 
//  ######   ########  ##     ## ## ### ##          ##  ######  ##     ## ## ## ## 
//  ##       ##   ##   ##     ## ##     ##    ##    ##       ## ##     ## ##  #### 
//  ##       ##    ##  ##     ## ##     ##    ##    ## ##    ## ##     ## ##   ### 
//  ##       ##     ##  #######  ##     ##     ######   ######   #######  ##    ## 

char* createSVGFile(char *filename) {
    SVGimage *svg = initSVGimage();
    bool status = writeSVGimage(svg, filename);

    deleteSVGimage(svg);

    if ( status )
        return "Success";
    else
        return "Failure";
}

char* setAttributeInFile(char *filename, char *type, int idx, char *name, char *value) {
    SVGimage *svg = createSVGimage(filename);
    Attribute *attr = createAttribute(name, value);
    elementType t;

    if ( !strcmp(type, "SVG") ) t = SVG_IMAGE;
    if ( !strcmp(type, "Rectangle") ) t = RECT;
    if ( !strcmp(type, "Circle") ) t = CIRC;
    if ( !strcmp(type, "Path") ) t = PATH;
    if ( !strcmp(type, "Group") ) t = GROUP;

    setAttribute(svg, t, idx, attr);

    if ( validateSVGimage(svg, "./xsd/svg.xsd") ) {
        writeSVGimage(svg, filename);
        return "Success";
    }
    
    return "Failure";
}

char* addRectFromJSON(char *filename, char *json) {
    SVGimage *svg = createSVGimage(filename);
    Rectangle *r = JSONtoRect(json);

    insertBack(svg->rectangles, r);

    if ( validateSVGimage(svg, "./xsd/svg.xsd") ) {
        writeSVGimage(svg, filename);
        return "Success";
    }

    return "Failure";
}

char* addCircleFromJSON(char *filename, char *json) {
    SVGimage *svg = createSVGimage(filename);
    Circle *c = JSONtoCircle(json);

    insertBack(svg->circles, c);

    if ( validateSVGimage(svg, "./xsd/svg.xsd") ) {
        writeSVGimage(svg, filename);
        return "Success";
    } 

    return "Failure";
}

char* updateFileTitle(char *filename, char *title) {
    SVGimage *svg = createSVGimage(filename);

    strcpy(svg->title, title);

    if ( validateSVGimage(svg, "./xsd/svg.xsd") ) {
        writeSVGimage(svg, filename);
        return "Success";
    } 

    return "Failure";
}

char* updateFileDesc(char *filename, char *desc) {
    SVGimage *svg = createSVGimage(filename);

    strcpy(svg->description, desc);

    if ( validateSVGimage(svg, "./xsd/svg.xsd") ) {
        writeSVGimage(svg, filename);
        return "Success";
    } 
    
    return "Failure";
}

SVGimage* JSONtoSVG(const char* svgString) {
    SVGimage *svg = initSVGimage();

    strcpy(svg->namespace, "http://www.w3.org/2000/svg");

    sscanf(svgString, "{\"title\":\"%[^\"]\",\"descr\":\"%[^\"]\"}", svg->title, svg->description);

    return svg;
}

Rectangle* JSONtoRect(const char* json) {
    Rectangle *rect = createRectangle();

    sscanf(json, "{\"x\":\"%[^\"]\",\"y\":\"%[^\"]\",\"w\":\"%[^\"]\",\"h\":\"%[^\"]\"}", rect->x, rect->y, rect->width, rect->height);

    return rect;
}

Circle* JSONtoCircle(const char* json) {
    Circle *circle = createCircle();

    sscanf(json, "{\"cx\":\"%[^\"]\",\"cy\":\"%[^\"]\",\"r\":\"%[^\"]\"}", circle->cx, circle->cy, circle->r);

    return circle;
}

//     ###    ######## ######## ########         ######## ##    ##  ######  
//    ## ##      ##       ##    ##     ##        ##       ###   ## ##    ## 
//   ##   ##     ##       ##    ##     ##        ##       ####  ## ##       
//  ##     ##    ##       ##    ########         ######   ## ## ##  ######  
//  #########    ##       ##    ##   ##          ##       ##  ####       ## 
//  ##     ##    ##       ##    ##    ##  ###    ##       ##   ### ##    ## 
//  ##     ##    ##       ##    ##     ## ###    ##       ##    ##  ######  

void updateAttribute(List *attributes, Attribute *key) {
    if ( !attributes || !key ) return;

    ListIterator iter = createIterator(attributes);
    Attribute *attr;

    while ( (attr = nextElement(&iter)) ) {
        if ( !strcmp(attr->name, key->name) ) {
            free(attr->value);

            attr->value = key->value;
            key->value = NULL;

            deleteAttribute(key);
            return;
        }
    }

    insertBack(attributes, key);
}

void updateSVGimageAttributes(SVGimage *img, Attribute *newAttr) {
    updateAttribute(img->otherAttributes, newAttr);
}

void updateRectangleAttributes(Rectangle *rect, Attribute *newAttr) {
    if ( !strcmp("x", newAttr->name) ) {
        rect->x = newAttr->value;
        deleteAttribute(newAttr);

    } else if ( !strcmp("y", newAttr->name) ) {
        rect->y = newAttr->value;
        deleteAttribute(newAttr);

    } else if ( !strcmp("width", newAttr->name) ) {
        rect->width = newAttr->value;
        deleteAttribute(newAttr);

    } else if ( !strcmp("height", newAttr->name) ) {
        rect->height = newAttr->value;
        deleteAttribute(newAttr);

    } else {
        updateAttribute(rect->otherAttributes, newAttr);
    }
}

void updateCircleAttributes(Circle *circle, Attribute *newAttr) {
    if ( !strcmp("cx", newAttr->name) ) {
        circle->cx = newAttr->value;
        deleteAttribute(newAttr);

    } else if ( !strcmp("cy", newAttr->name) ) {
        circle->cy = newAttr->value;
        deleteAttribute(newAttr);

    } else if ( !strcmp("r", newAttr->name) ) {
        circle->r = newAttr->value;
        deleteAttribute(newAttr);

    } else {
        updateAttribute(circle->otherAttributes, newAttr);
    }
}

void updatePathAttributes(Path *path, Attribute *newAttr) {
    if ( !strcmp("d", newAttr->name) ) {
        free(path->data);

        path->data = newAttr->value;
        newAttr->value = NULL;

        deleteAttribute(newAttr);

    } else {
        updateAttribute(path->otherAttributes, newAttr);
    }
}

void updateGroupAttributes(Group *group, Attribute *newAttr) {
    updateAttribute(group->otherAttributes, newAttr);
}



//   #######  ######## ##     ## ######## ########     ######## ##    ##  ######  
//  ##     ##    ##    ##     ## ##       ##     ##    ##       ###   ## ##    ## 
//  ##     ##    ##    ##     ## ##       ##     ##    ##       ####  ## ##       
//  ##     ##    ##    ######### ######   ########     ######   ## ## ##  ######  
//  ##     ##    ##    ##     ## ##       ##   ##      ##       ##  ####       ## 
//  ##     ##    ##    ##     ## ##       ##    ##     ##       ##   ### ##    ## 
//   #######     ##    ##     ## ######## ##     ##    ##       ##    ##  ######  

void copyList(List *source, List *dest) {
    if ( !source || !dest ) return;
    
    findAll(source, dest, NULL, NULL);
}

void copyField(char *field, char *content) {
    if ( strlen(content) > 255) {
        strncpy(field, content, 256);
        field[255] = 0;
    } else
        strcpy(field, content);
}

char* toString2(List * list, char *delim) {
    if ( !list ) return NULL;

	ListIterator iter = createIterator(list);
	char* str = (char *) malloc(sizeof(char));
    if ( !str ) return NULL;

	*str = 0;
	
	void* elem;

	while( (elem = nextElement(&iter) ) != NULL) {
		char* currDescr = list->printData(elem);
		int newLen = strlen(str) + 50 + strlen(currDescr);
		str = (char*) realloc(str, newLen);
		strcat(str, delim);
		strcat(str, currDescr);
		
		free(currDescr);
	}
	
	return str;
}

void *listAt(List *list, int index) {
    if ( !list || index >= getLength(list)) return NULL;

    ListIterator iter = createIterator(list);
    void *item;
    int i;

    for (i = 0; (item = nextElement(&iter)); i++)
        if (i == index) break;

    return item;
}
