#define _XOPEN_SOURCE 500

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
#include <ftw.h>

#define NAME_SIZE 4096
#define BUFFER_SIZE 10000
#define BLOCK_MULTIPLIER 7
#define TAR_COMMAND_SIZE 2048
#define THREADS 4
#define NOPENFD 4

struct entry
{
  char name[NAME_SIZE];
  char path[NAME_SIZE];
};
struct PARAM
{
  int start;
  int final;
  int num_thread;
};

struct entry buffer[BUFFER_SIZE];
char initial_destiny_name[NAME_SIZE];
char destiny_name[NAME_SIZE];
int orig_name_size;
int buffer_position = 0;
pthread_mutex_t lock;

int make_buffer(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
void *bzthread(void *param);

int main(int argc, char *argv[])
{
  int i;
  orig_name_size = strlen(argv[1]);
  strcpy(initial_destiny_name, argv[2]);
  char *path = strtok(argv[2], ".");
  strcpy(destiny_name, path);
  if (nftw(argv[1], make_buffer, NOPENFD, FTW_PHYS));
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
    pthread_create(&workers[i], &attrs[i], bzthread, &param[i]);
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

int make_buffer(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  if (typeflag == FTW_F) // is file
  {
    char *path = (char *)fpath + orig_name_size;
    strcpy(buffer[buffer_position].name, fpath);
    strcpy(buffer[buffer_position].path, path);
    buffer_position++;
  }
  return 0;
}

void *bzthread(void *arg)
{
  int i;
  struct PARAM *param = (struct PARAM *)arg;
  for (i = param->start; i < param->final; i++)
  {
    // bz2
    FILE *file_temp;
    ssize_t bytesRead;
    char *buf[BUFFER_SIZE], file_name[12];
    int file_origin = open(buffer[i].name, O_RDONLY), bzError;
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
      snprintf(tar_command, sizeof(tar_command), "tar rvf %s %s --transform 's,%s,%s%s.bz2,'", initial_destiny_name, file_name, file_name, destiny_name, buffer[i].path);
    pthread_mutex_lock(&lock);
    FILE *file = popen(tar_command, "r");
    char buffer_command[100];
    if (fscanf(file, "%100s", buffer_command));
    pclose(file);
    pthread_mutex_unlock(&lock);
  }
  pthread_exit(0);
}
