#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <fcntl.h>
// #include <libtar.h>


#define NAME_SIZE 1024
#define BUFFER_SIZE 10000
#define BLOCK_MULTIPLIER 7
#define COPY_MODE ACCESSPERMS
#define INITIAL_INDENT_SIZE 0 // debug

void bzthread(char *dir_name_origin, char *path_destiny, int indent);
void compress(char *name, char *path_origin, char *path_destiny);
void tar(char *path_destiny);

int main(int argc, char *argv[])
{
  char *origin = argv[1], *destiny = argv[2];
  strcat(destiny, ".bz2");
  mkdir(destiny, COPY_MODE);
  bzthread(origin, destiny, INITIAL_INDENT_SIZE);
  tar(destiny);
  return 0;
}

void bzthread(char *dir_name_origin, char *path_destiny, int indent)
{
  DIR *dir_origin;
  struct dirent *entry_origin;
  dir_origin = opendir(dir_name_origin);
  while ((entry_origin = readdir(dir_origin)) != NULL)
  {
    char dir_origin[NAME_SIZE];
    snprintf(dir_origin, sizeof(dir_origin), "%s/%s", dir_name_origin, entry_origin->d_name);
    if (entry_origin->d_type == DT_DIR) // is directory
    {
      if (strcmp(entry_origin->d_name, ".") == 0 || strcmp(entry_origin->d_name, "..") == 0)
        continue;
      // debug
        printf("%*sDIR : %s\n", indent, "", entry_origin->d_name);
      // end debug
      char dir_destiny[NAME_SIZE];
      snprintf(dir_destiny, sizeof(dir_destiny), "%s/%s", path_destiny, entry_origin->d_name);
      mkdir(dir_destiny, ACCESSPERMS);
      bzthread(dir_origin, dir_destiny, indent + 2);
    }
    else // is file
    {
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
  FILE *destiny;
  ssize_t bytesRead;
  int origin = open(path_origin, O_RDONLY), bzError;
  char file_destiny[NAME_SIZE], *buf[BUFFER_SIZE];
  snprintf(file_destiny, sizeof(file_destiny), "%s/%s.bz2", path_destiny, name);
  destiny = fopen(file_destiny, "wb");
  BZFILE *pBz = BZ2_bzWriteOpen(&bzError, destiny, BLOCK_MULTIPLIER, 0, 0);
  while((bytesRead = read(origin, buf, BUFFER_SIZE)) > 0)
    BZ2_bzWrite(&bzError, pBz, buf, bytesRead);
  BZ2_bzWriteClose(&bzError, pBz, 0, NULL, NULL);
  close(origin);
  fclose(destiny);
}

void tar(char *path_destiny)
{
  // TAR *pTar;
  // char destiny_name[] = path_destiny;
  // strcat(destiny_name, ".bz2.tar")
  // tar_open(&pTar, destiny_name, NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);
  // tar_append_tree(pTar, path_destiny, ".");
  // close(tar_fd(pTar));
  char tar_comand[15 + (NAME_SIZE * 2)] = "tar cf ";
  strcat(tar_comand, path_destiny);
  strcat(tar_comand, ".tar ");
  strcat(tar_comand, path_destiny);
  system(tar_comand);
  char rm_comand[] = "rm -rf "; // PELIGRO
  strcat(rm_comand, path_destiny); // PELIGRO
  system(rm_comand);
}
