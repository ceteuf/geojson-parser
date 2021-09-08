Build on the top of 'json-parser', the API is composed of only one function :
```c
int isGeoJSON(json_value*);
```
which return 1 if succeed, 0 otherwise.

This work try to implement the standard [rfc7946](https://datatracker.ietf.org/doc/html/rfc7946#section-3.1.1).   

***This is a work in progress.***


### TODO 
- Right-Hand Rule for polygons (propose a rewind ?)
- do extended tests
- bounding box
- revisit some crappy design
