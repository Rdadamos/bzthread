#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void go_through_all(char *dir_name_origin, char *path, int indent);

DIR *dir_destiny;

int main(int argc, char *argv[])
{
  // PASSO 1 - cp -ax DIR_ORIGEM DESTINO.bz2
  // PASSO 2 - cd DESTINO.bz2
  mkdir(argv[2], ACCESSPERMS);
  go_through_all(argv[1], argv[2], 0);
  // PASSO 4 - cd ..
  // PASSO 5 - tar cf DESTINO.bz2.tar DESTINO.bz2
  // PASSO 6 - rm -rf DESTINO.bz2
  return 0;
}

void go_through_all(char *dir_name_origin, char *path, int indent)
{
  DIR *dir_origin;
  struct dirent *entry_origin;

  if (!(dir_origin = opendir(dir_name_origin)))
    return;

  while ((entry_origin = readdir(dir_origin)) != NULL)
  {
    if (entry_origin->d_type == DT_DIR) // directory
    {
      if (strcmp(entry_origin->d_name, ".") == 0 || strcmp(entry_origin->d_name, "..") == 0)
        continue;
      char path_origin[1024], path_destiny[1024];
      snprintf(path_origin, sizeof(path_origin), "%s/%s", dir_name_origin, entry_origin->d_name);
      snprintf(path_destiny, sizeof(path_destiny), "%s/%s", path, entry_origin->d_name);
      // debug
      printf("%*sDIR : %s\n", indent, "", entry_origin->d_name);
      // end debug
      mkdir(path_destiny, ACCESSPERMS);
      go_through_all(path_origin, path_destiny, indent + 2);
    }
    else // file
      // debug
      printf("%*sFILE: %s\n", indent, "", entry_origin->d_name);
      // end debug
      // PASSO 3 - find . -type f -exec bzip2 "{}" \;
  }
  closedir(dir_origin);
}
