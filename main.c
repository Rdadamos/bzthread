#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

void list_all(const char *dir_name, int indent);

int main(int argc, char const *argv[])
{
  // PASSO 1 - cp -ax DIR_ORIGEM DESTINO.bz2
  // PASSO 2 - cd DESTINO.bz2
  list_all(argv[1], 0);
  // PASSO 4 - cd ..
  // PASSO 5 - tar cf DESTINO.bz2.tar DESTINO.bz2
  // PASSO 6 - rm -rf DESTINO.bz2
  return 0;
}

void list_all(const char *dir_name, int indent)
{
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(dir_name)))
    return;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      char path[1024];
      snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
      printf("%*sDIR : %s\n", indent, "", entry->d_name);
      list_all(path, indent + 2);
    } else
      printf("%*sFILE: %s\n", indent, "", entry->d_name);
      // PASSO 3 - find . -type f -exec bzip2 "{}" \;
  }
  closedir(dir);
}
