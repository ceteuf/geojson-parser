#include "geojson.h"
#include "include/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>


const char* allowed_types[] = {
    "Point",
    "MultiPoint",
    "LineString",
    "MultiLineString",
    "Polygon",
    "MultiPolygon",
    "Feature",
    "FeatureCollection",
    "GeometryCollection"
};

int isTypeInAllowedTypes(char* str){
    for (int i=0; i<9; i++){
        if (strcmp(str, allowed_types[i]) == 0){
            return 1;
        }
    }
    return 0;
}

char* verifyType(json_value* value){
    /*check if 'type' is in names of depth 0*/
    // AFAIRE : acceder au depth !!!!!
    int length, x;
    char* type;
    length = value->u.object.length; 
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "type") == 0){
            //printf("%s\n", value->u.object.values[x].value->u.string.ptr);
            type = value->u.object.values[x].value->u.string.ptr;
            if (isTypeInAllowedTypes(type) == 1){
                return type;
            }
        }
    }
    return 0;
}

int getCoordinatesIndex(json_value* value){
    /*check if 'coordinates' is in names of depth*/
    // renvoi - 1 si fail : pas de name 'coordinates'
    // renvoi l'indice de coordinate dans l'objet
    int length;
    length = value->u.object.length;

    for (int x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "coordinates") == 0){
            return x;
        }
    }
    return -1;
}

int verifyProperties(json_value* value){
    int length, x;

    length = value->u.object.length; 
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "properties") == 0){
            if (value->u.object.values[x].value->type == json_object ||
                value->u.object.values[x].value->type == json_null){
                return 1;
            }
        }
    }
    return 0;
}

int correctGeometryCollectionMembers(json_value* value){
    int length, x;
    length = value->u.object.length;
    //printf("length = %d\n", length);
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "type") == 0){
            if (value->u.object.values[x].value->type == json_string){
                if (strcmp(value->u.object.values[x].value->u.string.ptr, "Feature") != 0 && 
                    strcmp(value->u.object.values[x].value->u.string.ptr, "FeatureCollection") != 0 ){
                    return 1;
                }
            }
        }
    }
    return 0;
}



int isCorrect1dArr(json_value* value, int length_should_be){
    // 1 : valid
    // 0 : invalid
    // en général "length_should_be" représente la taille du premier sous-array dans le cadre d'un array multiple
    // déterminer a l'intérieur de 2Darr (ou plus généralement par la fonction qui appelle 1Darray)
    
    double lon = 0;
    double lat = 0;
    json_value** valeurs;
    int array_length = value->u.array.length;
    // printf("array_length = %d\n\n", array_length);


    if (array_length < 2 && array_length > 3){
        return 0;
    }
    if (array_length != length_should_be){
        return 0;
    }
    valeurs = value->u.array.values;

    // lon conversion
    if (valeurs[0]->type == 3) {
        lon = (double)valeurs[0]->u.integer;
    }
    else if (valeurs[0]->type == 4){
        lon = valeurs[0]->u.dbl;
    }
    else {
        return 0;
    }

    // lat conversion
    if (valeurs[1]->type == 3) {
        lat = (double)valeurs[1]->u.integer;
    }
    else if (valeurs[1]->type == 4){
        lat = valeurs[1]->u.dbl;
    }
    else {
        return 0;
    }

    // check lon & lat bounds -- WGS84
    if (lon < -180.0 || lon > 180){
        return 0;
    }
    if (lat < -90.0 || lat > 90.0){
        return 0;
    }

    // printf("lon = %.4f\n", lon);
    // printf("lat = %.4f\n", lat);

    return 1;
}

int isCorrect2dArr(json_value* value, char* type){
    // seul cas qui matche sans comparaison = MultiPoint
    json_value* v_start;
    json_value* v_end;

    int array_length = value->u.array.length;
    int length_should_be = value->u.array.values[0]->u.array.length; // init avec len de '****' [ ****[a,b]***, [c,d], .... ]

    if (strcmp(type, "LineString") == 0) { 
        if (array_length < 2){ // une LineString doit etre composée de 2 ou plusieurs positions
            return 0;
        }
    }

    if (strcmp(type, "Polygon") == 0) { 
        if (array_length < 4){ // une LineString doit etre composée de 2 ou plusieurs positions
            return 0;
        }

        // le premier et le dernier élément doivent êtres identiques 
        v_start = value->u.array.values[0];
        v_end = value->u.array.values[array_length-1];
        
        if (isCorrect1dArr(v_start, length_should_be) == 1 && isCorrect1dArr(v_end, length_should_be) == 1){
            json_value** valeurs_start = v_start->u.array.values;
            json_value** valeurs_end = v_end->u.array.values;

            double lon_start, lat_start, lon_end, lat_end;

            // lon start conversion
            if (valeurs_start[0]->type == 3) {
                lon_start = (double)valeurs_start[0]->u.integer;
            }
            else if (valeurs_start[0]->type == 4){
                lon_start = valeurs_start[0]->u.dbl;
            }
            else {
                return 0;
            }

            // lat start conversion
            if (valeurs_start[1]->type == 3) {
                lat_start = (double)valeurs_start[1]->u.integer;
            }
            else if (valeurs_start[1]->type == 4){
                lat_start = valeurs_start[1]->u.dbl;
            }
            else {
                return 0;
            }

            // lon end conversion
            if (valeurs_end[0]->type == 3) {
                lon_end = (double)valeurs_end[0]->u.integer;
            }
            else if (valeurs_end[0]->type == 4){
                lon_end = valeurs_end[0]->u.dbl;
            }
            else {
                return 0;
            }

            // lat end conversion
            if (valeurs_end[1]->type == 3) {
                lat_end = (double)valeurs_end[1]->u.integer;
            }
            else if (valeurs_end[1]->type == 4){
                lat_end = valeurs_end[1]->u.dbl;
            }
            else {
                return 0;
            }

            if ((lon_start != lon_end) ||
                (lat_start != lat_end)){
                printf("twerk\n\n");
                return 0;
            }
        }
    }

    for (int i=0; i<array_length; i++){
        json_value* curr_array = value->u.array.values[i];
        if (isCorrect1dArr(curr_array, length_should_be) == 0) {
            return 0;
        }
    }

    return 1;
}

int isCorrect3dArr(json_value* value, char* type){
    int array_length = value->u.array.length;
    char* sub_type;
    if (strcmp(type, "MultiLineString") == 0) { 
        sub_type = "LineString";
    }
    if (strcmp(type, "Polygon") == 0) { 
        sub_type = "Polygon";
    }

    int length_should_be = value->u.array.values[0]->u.array.length;

    for (int i=0; i<array_length; i++){
        json_value* curr_array = value->u.array.values[i];
        if (isCorrect2dArr(curr_array, sub_type) == 0) {
            return 0;
        }
    }

    return 1;
}

int isCorrect4dArr(json_value* value, char* type){
    // le seul type acceptant ce point de départ est le "MultiPolygon"
    int array_length = value->u.array.length;
    char* sub_type;
    if (strcmp(type, "MultiPolygon") == 0) { 
        sub_type = "Polygon";
    }

    int length_should_be = value->u.array.values[0]->u.array.length;

    for (int i=0; i<array_length; i++){
        json_value* curr_array = value->u.array.values[i];

        if (isCorrect3dArr(curr_array, sub_type) == 0) {
            return 0;
        }
    }

    return 1; 
}



int isCorrectPoint(json_value* value){
    // 1 : valid
    // 0 : invalid

    int x = getCoordinatesIndex(value);

    if (x != -1){
        int curr_json_type = value->u.object.values[x].value->type;
        json_value* curr_array = value->u.object.values[x].value;
        if (curr_json_type == json_array){
            return isCorrect1dArr(curr_array, value->u.object.values[x].value->u.array.length);
        }
    }

    return 0;
}

int isCorrectMultiPoint(json_value* value){
    // 1 : valid
    // 0 : invalid

    int x = getCoordinatesIndex(value);
    int curr_json_type = value->u.object.values[x].value->type;
    json_value* curr_array = value->u.object.values[x].value;
    if (curr_json_type == json_array) {
        return isCorrect2dArr(curr_array, "MultiPoint");
    }

    return 0;
}

int isCorrectLineString(json_value* value){
    // 1 : valid
    // 0 : invalid

    int x = getCoordinatesIndex(value);
    int curr_json_type = value->u.object.values[x].value->type;
    json_value* curr_array = value->u.object.values[x].value;
    if (curr_json_type == json_array) { // position 2 in json_type enum == array
        return isCorrect2dArr(curr_array, "LineString");
    }

    return 0;
}

int isCorrectMultiLineString(json_value* value){
    // 1 : valid
    // 0 : invalid

    int x = getCoordinatesIndex(value);
    int curr_json_type = value->u.object.values[x].value->type;
    json_value* curr_array = value->u.object.values[x].value;
    if (curr_json_type == json_array) { // position 2 in json_type enum == array
        return isCorrect3dArr(curr_array, "MultiLineString");
    }

    return 0;
}

int isCorrectPolygon(json_value* value){
    // 1 : valid
    // 0 : invalid

    int x = getCoordinatesIndex(value);
    int curr_json_type = value->u.object.values[x].value->type;
    json_value* curr_array = value->u.object.values[x].value;
    if (curr_json_type == json_array) { // position 2 in json_type enum == array
        return isCorrect3dArr(curr_array, "Polygon");
    }

    return 0;
}

int isCorrectMultiPolygon(json_value* value){
    // 1 : valid
    // 0 : invalid
    int x = getCoordinatesIndex(value);
    int curr_json_type = value->u.object.values[x].value->type;
    json_value* curr_array = value->u.object.values[x].value;
    if (curr_json_type == json_array) { // position 2 in json_type enum == array
        return isCorrect4dArr(curr_array, "MultiPolygon");
    }
    return 0;
}

int isCorrectFeature(json_value* value){
    // 1 : valid
    // 0 : invalid
    int length, x;

    length = value->u.object.length; 
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "geometry") == 0){

            int curr_json_type = value->u.object.values[x].value->type;
            if (curr_json_type == json_object) {
                int object_len_under_geometry = value->u.object.values[x].value->u.object.length;
                
                json_value* sub_object = value->u.object.values[x].value;
                return isGeoJSON(sub_object);
            }
    
        }
    }
    return 0;  
}

int isCorrectFeatureCollection(json_value* value){
    // 1 : valid
    // 0 : invalid
    int length, x;
    int statut = 1;
    length = value->u.object.length; 
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "features") == 0){

            int curr_json_type = value->u.object.values[x].value->type;

            if (curr_json_type == json_array) {
                int array_len_under_geometry = value->u.object.values[x].value->u.object.length;

                for (int y=0; y<array_len_under_geometry; y++){
                    int array_elem_type = value->u.object.values[x].value->u.array.values[y]->type;

                    if (array_elem_type == json_object){
                        //printf("array_len_under_geometry = %d\n", array_len_under_geometry);
                        json_value* array_elem = value->u.object.values[x].value->u.array.values[y];
                        
                        statut = statut * isGeoJSON(array_elem);
                    }
                }
            }
            if (statut == 1){
                return 1;
            }
        }
    }
    return 0;  
}

int isCorrectGeometryCollection(json_value* value){
    // 1 : valid
    // 0 : invalid
    int length, x;
    int statut = 1;
    length = value->u.object.length;
    for (x=0; x<length; x++) {
        if (strcmp(value->u.object.values[x].name, "geometries") == 0){

            int curr_json_type = value->u.object.values[x].value->type;
            //printf("curr type = %d\n", curr_json_type);
            if (curr_json_type == json_array) {
                int array_len_under_geometry = value->u.object.values[x].value->u.object.length;
                //printf("array_len_under_geometry = %d\n", array_len_under_geometry);
                for (int y=0; y<array_len_under_geometry; y++){
                    int array_elem_type = value->u.object.values[x].value->u.array.values[y]->type;
                    //printf("y = %d\n", y);
                    if (array_elem_type == json_object){
                        //printf("array_len_under_geometry = %d\n", array_len_under_geometry);
                        json_value* array_elem = value->u.object.values[x].value->u.array.values[y];

                        // check type inside
                        if (correctGeometryCollectionMembers(array_elem) == 1){
                            //printf("iscorrect Feature = %d\n", isGeoJSON(array_elem));
                            statut = statut * isGeoJSON(array_elem);
                        }
                        else {
                            return 0;
                        }
                        
                    }
                }
            }
            if (statut == 1){
                return 1;
            }
        }
    }
    return 0; 
}


int isGeoJSON(json_value* value){
    // 1 good
    // 0 pas good
    char* type;
    type = verifyType(value);
    if (type == 0){
        return 0;
    }

    if (strcmp(type, "Point") == 0){
        if (isCorrectPoint(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "MultiPoint") == 0){
        if (isCorrectMultiPoint(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "LineString") == 0){
        if (isCorrectLineString(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "MultiLineString") == 0){
        if (isCorrectMultiLineString(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "Polygon") == 0){
        if (isCorrectPolygon(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "MultiPolygon") == 0){
        if (isCorrectMultiPolygon(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "Feature") == 0){
        if (isCorrectFeature(value) == 1 && verifyProperties(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "FeatureCollection") == 0){
        if (isCorrectFeatureCollection(value) == 1){
            return 1;
        }
    }
    if (strcmp(type, "GeometryCollection") == 0){
        if (isCorrectGeometryCollection(value) == 1){
            return 1;
        }
    }

    return 0;
}

int main(int argc, char** argv){
    char* filename;
    FILE *fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;

    if (argc != 2) {
        fprintf(stderr, "%s <file_json>\n", argv[0]);
        return 1;
    }

    filename = argv[1];

    if ( stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }

    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);

    if ( file_contents == NULL) {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename, "rt");

    if (fp == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }

    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
        fprintf(stderr, "Unable to read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }

    fclose(fp);

    //printf("%s\n", file_contents);

    // printf("--------------------------------\n\n");

    json = (json_char*)file_contents;

    value = json_parse(json, file_size);

    if (value == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    // process_value(value, 0);

    //printf("--------------------------------\n\n");

    int res;
    res = isGeoJSON(value);

    printf("--------------------------------\n\n");
    printf("Is geoJSON file ? rep: %d", res);

    json_value_free(value);
    free(file_contents);
    return 0;
}