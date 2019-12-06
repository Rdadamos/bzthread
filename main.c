#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <fcntl.h>
#include <pthread.h>

#define NAME_SIZE 1024
#define BUFFER_SIZE 10000
#define BLOCK_MULTIPLIER 7
#define TAR_COMMAND_SIZE 2048
#define THREADS 4

struct entry
{
  char *name;
  char *path_origin;
  char *path;
};
struct PARAM
{
  int start;
  int final;
  int num_thread;
};

struct entry buffer[BUFFER_SIZE];
char initial_destiny_name[NAME_SIZE];
int indent = 0; // debug

void bzthread(char *path_origin, char *path, int *buffer_position);
void *compress(void *param);

int main(int argc, char *argv[])
{
  strcpy(initial_destiny_name, argv[2]);
  char *path = strtok(argv[2], ".");
  int buffer_position = 0;
  bzthread(argv[1], path, &buffer_position);
  printf("%d\n", buffer_position);
  int i;
  for (i = 0; i < buffer_position; i++)
    printf("f: %d :: %s - %s - %s \n", i, buffer[i].name, buffer[i].path_origin, buffer[i].path);
  pthread_t workers[THREADS];
  pthread_attr_t attrs[THREADS];
  int batch = buffer_position/THREADS;
  int rest = buffer_position%THREADS;
  struct PARAM param[THREADS];
  for (i = 0; i < THREADS; i++)
  {
    param[i].start = i * batch;
    param[i].final = ((i + 1) * batch) + (((THREADS % (i + 2))/THREADS) * rest);
    param[i].num_thread = i;
    pthread_attr_init(&attrs[i]);
    pthread_create(&workers[i], &attrs[i], compress, &param[i]);
  }
  for(i = 0; i < THREADS; i++)
    pthread_join(workers[i], NULL);
  for(i = 0; i < THREADS; i++)
  {
    char name[12];
    snprintf(name, 12, "tempbz2_%d", i);
    remove(name);
  }
  return 0;
}

void bzthread(char *path_origin, char *path, int *buffer_position)
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
      indent += 2; // debug
      bzthread(path_dir, path_destiny, buffer_position);
    }
    else // is file
    {
      buffer[*buffer_position].name = entry_origin->d_name;
      buffer[*buffer_position].path_origin = path_origin;
      buffer[*buffer_position].path = path;
      *buffer_position += 1;
      printf("\nBBBBB: %d :: %s - %s - %s \n", *buffer_position, entry_origin->d_name, path_origin, path);
      // printf("%*sFILE: %s\n", indent, "", entry_origin->d_name); // debug
      int i;
      for (i = 0; i < *buffer_position; i++)
        printf("c: %d :: %s - %s - %s \n", i, buffer[i].name, buffer[i].path_origin, buffer[i].path);
    }
  }
  closedir(dir_origin);
}

void *compress(void *arg)
{
  int i;
  struct PARAM *param = (struct PARAM *)arg;
  for (i = param->start; i < param->final; i++)
  {
    // bz2
    FILE *file_temp;
    ssize_t bytesRead;
    char path_name[NAME_SIZE], *buf[BUFFER_SIZE], file_name[12];
    snprintf(path_name, sizeof(path_name), "%s/%s", buffer[i].path_origin, buffer[i].name);
    int file_origin = open(path_name, O_RDONLY), bzError;
    snprintf(file_name, 12, "tempbz2_%d", param->num_thread);
    file_temp = fopen(file_name, "wb");
    BZFILE *pBz = BZ2_bzWriteOpen(&bzError, file_temp, BLOCK_MULTIPLIER, 0, 0);
    while((bytesRead = read(file_origin, buf, BUFFER_SIZE)) > 0)
      BZ2_bzWrite(&bzError, pBz, buf, bytesRead);
    BZ2_bzWriteClose(&bzError, pBz, 0, NULL, NULL);
    close(file_origin);
    fclose(file_temp);
    //tar
    char tar_command[TAR_COMMAND_SIZE];
    snprintf(tar_command, sizeof(tar_command), "tar rvf %s %s --transform 's,%s,%s/%s.bz2,'", initial_destiny_name, file_name, file_name, buffer[i].path, buffer[i].name);
    printf("%s\n", tar_command);
    FILE *file = popen(tar_command, "r");
    char buffer_command[100];
    if (fscanf(file, "%100s", buffer_command));
    pclose(file);
  }
  pthread_exit(0);
}
