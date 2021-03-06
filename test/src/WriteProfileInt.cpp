#include "Profile.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s NAME VALUE\n", argv[0]);
    return 1;
  }

#ifdef _UNICODE
  TCHAR name[1024];
  int length = ::MultiByteToWideChar(CP_ACP, 0, argv[1], -1, name, 1024);
  if (length == 0)
    return 2;
#else
  const char *name = argv[1];
#endif

  int value = atoi(argv[2]);
  if (!Profile::Set(name, value)) {
    fputs("Error\n", stderr);
    return 2;
  }

  return 0;
}
