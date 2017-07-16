#include <stdio.h>
#include <string.h>
#include <windows.h>

char* getDir(char* input) {
  char* pos = input;
  char* ch = input;
  while((ch=strchr(pos+1,'\\'))!=NULL) pos = ch;
  if(pos!=input) {
    char* result = new char[pos-input+1];
    strncpy(result,input,pos-input);
    result[pos-input] = 0;
    return result;
  }
  else {
    int len = GetCurrentDirectory(0,NULL);
    char* result = new char[len];
    GetCurrentDirectory(len,result);
    return result;
  }
}

int main(int argc, char** argv) {
  if(argc<2) return 1;
  char* dir = argv[1];
  puts(getDir(dir));
  getchar();
}