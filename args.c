#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if WIN32

    #include "args.h"

#else
extern "C"{
    #include "args.h"
}
#endif
int indexOf(char* src, char key) {
  int i = 0;
  for(; src[i] != '\0'; i++) {
    if(src[i] == key) return i;
  }
  return -1;
}

int sizeofstr(char *str) {
  int i = 0;
  for(; str[i] != '\0'; i++);
  return i;
}

char *substrFromIndex(char *src, int index) {
  int i = index;
  char *str = (char *)malloc(sizeofstr(src) - index);
  int _i = 0;
  for(; src[i] != '\0'; i++) {
    str[_i] = src[i];
    _i++;
  }
  return str;
}

char *substrUntil(char *src, int index) {
  int i = 0;
  char *str = (char *)malloc(index);
  for(; i < index; i++) {
    str[i] = src[i];
  }
  return str;
}

char *getArg(int argc, char** argv, char* key) {

  int i = 0;
  for(; i < argc; i++) {
    int index = indexOf(argv[i], '=');
    char *strToComp = (index == -1) ? argv[i] : substrUntil(argv[i], index);
    if(strcmp(strToComp, key) == 0) {
      if(index != -1) {
	return substrFromIndex(argv[i], index+1);
      } else {
	if(argv[i+1] && indexOf(argv[i+1], '-') != 0) {
	  return argv[i+1];
	} else {
	  return "true";
	}
      }
    }
  }
  return "false";
}
