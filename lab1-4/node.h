#ifndef NODE
#define NODE
#include <stdio.h>
#include <stdlib.h>
enum NodeType { NTML, NVL, VL };
struct Node {
    char* nodeName;
    enum NodeType nodeType;
    int lineNum;
    union {
        int Valint;
        float Valfloat;
        char* Valstr;
    };
    struct Node* firstChild;
    struct Node* Sibc;
};
#endif