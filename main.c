#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <bzlib.h>

#define BUFFERSIZE      4096
#define COPYMODE        0644

void go_through_all(char *dir_name_origin, char *path_destiny, int indent);
void compress(char *name, char *path_origin, char *path_destiny);

DIR *dir_destiny;

int main(int argc, char *argv[])
{
  mkdir(argv[2], ACCESSPERMS);
  go_through_all(argv[1], argv[2], 0);
  return 0;
}

void go_through_all(char *dir_name_origin, char *path_destiny, int indent)
{
  DIR *dir_origin;
  struct dirent *entry_origin;

  if (!(dir_origin = opendir(dir_name_origin)))
    return;

  while ((entry_origin = readdir(dir_origin)) != NULL)
  {
    char dir_origin[1024], dir_destiny[1024];
    snprintf(dir_origin, sizeof(dir_origin), "%s/%s", dir_name_origin, entry_origin->d_name);
    snprintf(dir_destiny, sizeof(dir_destiny), "%s/%s", path_destiny, entry_origin->d_name);
    if (entry_origin->d_type == DT_DIR) // is directory
    {
      if (strcmp(entry_origin->d_name, ".") == 0 || strcmp(entry_origin->d_name, "..") == 0)
        continue;
      // debug
        printf("%*sDIR : %s\n", indent, "", entry_origin->d_name);
      // end debug
      mkdir(dir_destiny, ACCESSPERMS);
      go_through_all(dir_origin, dir_destiny, indent + 2);
    }
    else // is file
    {
      // PASSO 3 - find . -type f -exec bzip2 "{}" \;
      compress(entry_origin->d_name, dir_origin, path_destiny);
      // debug
        printf("%*sFILE: %s\n", indent, "", entry_origin->d_name);
      // end debug
    }
  }
  closedir(dir_origin);
}

void compress(char *name, char *path_origin, char *path_destiny)
{
  char file_destiny[1024];
  FILE *destiny;
  snprintf(file_destiny, sizeof(file_destiny), "%s/%s.bz2", path_destiny, name);
  int origin = open(path_origin, O_RDONLY);
  destiny = fopen(file_destiny, "wb");
  int bzError;
  const int BLOCK_MULTIPLIER = 7;
  BZFILE *pBz = BZ2_bzWriteOpen(&bzError, destiny, BLOCK_MULTIPLIER, 0, 0);
  const int BUF_SIZE = 10000;
  char* buf[BUF_SIZE];
  ssize_t bytesRead;
  while((bytesRead = read(origin, buf, BUF_SIZE)) > 0) {
      BZ2_bzWrite(&bzError, pBz, buf, bytesRead);
  }
  BZ2_bzWriteClose(&bzError, pBz, 0, NULL, NULL);
  close(origin);
  fclose(destiny);
}
