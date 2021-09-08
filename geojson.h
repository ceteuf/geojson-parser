#ifndef CGEOJSON__H
#define CGEOJSON__H
#include "include/json.h"

/*prototypes*/

int isTypeInAllowedTypes(char*);
char* verifyType(json_value*);
int getCoordinatesIndex(json_value*);
int verifyProperties(json_value*);
int correctGeometryCollectionMembers(json_value*);

int isCorrect1dArr(json_value*, int);
int isCorrect2dArr(json_value*, char*);
int isCorrect3dArr(json_value*, char*);
int isCorrect4dArr(json_value*, char*);

int isCorrectPoint(json_value*);
int isCorrectMultiPoint(json_value*);
int isCorrectLineString(json_value*);
int isCorrectMultiLineString(json_value*);
int isCorrectPolygon(json_value*);
int isCorrectMultiPolygon(json_value*);
int isCorrectFeature(json_value*);
int isCorrectFeatureCollection(json_value*);
int isCorrectGeometryCollection(json_value*);

int isGeoJSON(json_value*);

#endif