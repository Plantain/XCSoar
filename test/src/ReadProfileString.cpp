#include "Profile.hpp"

#include <stdio.h>
#include <windows.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s NAME\n", argv[0]);
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

  TCHAR value[1024];
  if (Profile::Get(name, value, 1024)) {
    _putts(value);
    return 0;
  } else {
    fputs("No such value\n", stderr);
    return 2;
  }
}
