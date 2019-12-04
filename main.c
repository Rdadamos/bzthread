#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <fcntl.h>

#define NAME_SIZE 1024
#define BUFFER_SIZE 10000
#define BLOCK_MULTIPLIER 7
#define TAR_COMMAND_SIZE 2048

char initial_destiny_name[NAME_SIZE];
// int indent = 0; // debug

void bzthread(char *path_origin, char *path);
void compress(char *name, char *path_origin, char *path);

int main(int argc, char *argv[])
{
  strcpy(initial_destiny_name, argv[2]);
  char *path = strtok(argv[2], ".");
  bzthread(argv[1], path);
  remove("tempbz2");
  return 0;
}

void bzthread(char *path_origin, char *path)
{
  DIR *dir_origin;
  struct dirent *entry_origin;
  dir_origin = opendir(path_origin);
  while ((entry_origin = readdir(dir_origin)) != NULL)
  {
    if (entry_origin->d_type == DT_DIR) // is directory
    {
      if (strcmp(entry_origin->d_name, ".") == 0 || strcmp(entry_origin->d_name, "..") == 0)
        continue;
      // printf("%*sDIR : %s\n", indent, "", entry_origin->d_name); // debug
      char path_dir[NAME_SIZE], path_destiny[NAME_SIZE];
      snprintf(path_dir, sizeof(path_dir), "%s/%s", path_origin, entry_origin->d_name);
      snprintf(path_destiny, sizeof(path_destiny), "%s/%s", path, entry_origin->d_name);
      // indent += 2; // debug
      bzthread(path_dir, path_destiny);
    }
    else // is file
    {
      compress(entry_origin->d_name, path_origin, path);
      // printf("%*sFILE: %s\n", indent, "", entry_origin->d_name); // debug
    }
  }
  closedir(dir_origin);
}

void compress(char *name, char *path_origin, char *path)
{
  // bz2
  FILE *file_temp;
  ssize_t bytesRead;
  char path_name[NAME_SIZE], *buf[BUFFER_SIZE];
  snprintf(path_name, sizeof(path_name), "%s/%s", path_origin, name);
  int file_origin = open(path_name, O_RDONLY), bzError;
  file_temp = fopen("tempbz2", "wb");
  BZFILE *pBz = BZ2_bzWriteOpen(&bzError, file_temp, BLOCK_MULTIPLIER, 0, 0);
  while((bytesRead = read(file_origin, buf, BUFFER_SIZE)) > 0)
    BZ2_bzWrite(&bzError, pBz, buf, bytesRead);
  BZ2_bzWriteClose(&bzError, pBz, 0, NULL, NULL);
  close(file_origin);
  fclose(file_temp);
  //tar
  char tar_command[TAR_COMMAND_SIZE];
  snprintf(tar_command, sizeof(tar_command), "tar rvf %s tempbz2 --transform 's,tempbz2,%s/%s.bz2,'", initial_destiny_name, path, name);
  FILE *file = popen(tar_command, "r");
  char buffer[100];
  if (fscanf(file, "%100s", buffer));
  pclose(file);
}
