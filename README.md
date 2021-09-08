Build on the top of [json-parser](https://github.com/json-parser/json-parser), the API is composed of only one function :
```c
int isGeoJSON(json_value*);
```
which return 1 if json is valid geojson, 0 otherwise.

This work try to implement the standard [rfc7946](https://datatracker.ietf.org/doc/html/rfc7946#section-3.1.1).   

Coordinate system reference implement is [EPSG:4326](https://epsg.io/4326)

***This is a work in progress.***


### TODO 
- Right-Hand Rule for polygons (propose a rewind ?)
- do extended tests
- bounding box
- revisit some rotten designs
