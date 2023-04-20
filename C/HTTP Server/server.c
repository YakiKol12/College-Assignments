#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#define MAX_LEN 500
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

int isNumber(const char *);
void* handleClient(void *);
char* errorHandle(int, char*);
char* handleDirectory(char*);
char* handleFile(char*, int);
char* get_mime_type(char*);
int isPermitted(char*);

int main(int argc, char const *argv[]) {
    int port = 0;
    int pool_size = 0;
    int max_number_of_request = 0;

    if(argc != 4) {
        printf("Usage: serrver <port> <pool-size> <max-number-of-request>\n");
        exit(0);
    }

    // validating input
    if (isNumber(argv[1]) && isNumber(argv[2]) && isNumber(argv[3])) {
        port = atoi(argv[1]);
        pool_size = atoi(argv[2]);
        max_number_of_request = atoi(argv[3]);
    }
    else {
        printf("Usage: serrver <port> <pool-size> <max-number-of-request>\n");
        exit(0);
    }
    if (port <= 0 || port > 65536 || pool_size <= 0 || max_number_of_request <= 0) {
        printf("Usage: serrver <port> <pool-size> <max-number-of-request>\n");
        exit(0);
    }

    threadpool *pool = create_threadpool(pool_size);

    // creating a TCP socket
    int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        perror("socket() failed");
        exit(1);
    }
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to a port
    if (bind(sd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("bind() failed");
        exit(1);
    }
    // creating the "welcome socket" queue length
    if (listen(sd, 5) < 0) {
        perror("listen() failed");
        exit(1);
    }
    // starting to recieve requests
    int new_sd;
    for (int i = 0; i < max_number_of_request; i++) {
        new_sd = accept(sd, NULL, NULL);
        if (new_sd < 0) {
            perror("accept() failed");
            exit(1);
        }
        // dispatch a new work to the queue
        int *new_socket = (int*)malloc(sizeof(int));
        *new_socket = new_sd;
        dispatch(pool, (dispatch_fn)handleClient, new_socket);
    }
    destroy_threadpool(pool);
    close(sd);
    return 0;
}

// This method checks if a given string represent a number
int isNumber(const char *str) {
    int i = strlen(str);

    if(i == 0)
        return 0;

    while (i--) {
        if (str[i] >= '0' && str[i] <= '9')
            continue;
        return 0;
    }
    return 1;
}

// This method handles an HTTP client request
// thread function called after accpet gets a new request
void *handleClient(void *arg) {
    int new_sd = *((int *)arg);
    free(arg);

    char request[MAX_LEN] = "";
    char *res = NULL;
    if (read(new_sd, request, MAX_LEN) < 0) {
        perror("read() falied");
        res = errorHandle(500, NULL);
        write(new_sd, res, strlen(res));
        exit(1);
    }

    // if anyone asks this is for null requests from the browser
    if(request == NULL || strlen(request) == 0) {
        return NULL;
    }

    char *tmp = strstr(request, "\r\n");
    tmp[0] = '\0';

    // validating request
    char method[MAX_LEN] = "";
    char path[MAX_LEN] = ".";
    char protocol[MAX_LEN] = "";
    const char del[2] = " ";

    char *token = strtok(request, del);
    if (token == NULL) {
        res = errorHandle(400, path);
    }
    strncpy(method, token, strlen(token));

    token = strtok(NULL, del);
    if (token == NULL) {
        res = errorHandle(400, path);
    }
    strcat(path, token);

    token = strtok(NULL, del);
    if (token == NULL || (strcmp(token, "HTTP/1.0") != 0 && strcmp(token, "HTTP/1.1") != 0)) {
        res = errorHandle(400, path);
    }
    if(token != NULL) {
        strncpy(protocol, token, strlen(token));
    }

    if ((token = strtok(NULL, del)) != NULL) {
        res = errorHandle(400, path);
    }
    if (strcmp(method, "GET") != 0) {
        res = errorHandle(501, path);
    }
    // if the was a usage error printing response now
    if(res != NULL) {
        write(new_sd, res, strlen(res));
        return NULL;   
    }

    // check if path leads to existing file/directory
    struct stat s;
    int result = stat(path, &s);
    if (result == 0) {
        if (S_ISREG(s.st_mode)) { // handle file
            if((res = handleFile(path, new_sd)) == NULL) {
                return NULL;
            }
        }
        else if (S_ISDIR(s.st_mode)) {
            if(path[strlen(path) - 1] != '/') {
                res = errorHandle(302, path);
            }
            else { // handle directory
                char *new_path = (char*)malloc(sizeof(char) * (strlen(path) + strlen("index.html") + 1));
                sprintf(new_path, "%sindex.html", path);
                if(stat(new_path, &s) == 0) {
                    res = handleFile(new_path, new_sd);
                    free(new_path);
                }
                else {
                    free(new_path);
                    res = handleDirectory(path);
                }
            }
        }
    }
    else {
        res = errorHandle(404, path);
    }
    
    if(res != NULL) {
        write(new_sd, res, strlen(res));
        free(res);
    }
    close(new_sd);
    return NULL;
}

// This method recieves an error code and creates the apropriate response to return
char* errorHandle(int code, char *path) {
    char *response = (char*)malloc(sizeof(char) * MAX_LEN);
    response[0] = '\0';
    char headers[MAX_LEN] = "HTTP/1.0 ";
    char body[MAX_LEN] = "";
    char location[MAX_LEN] = "";
    switch(code) {
        case 302:
            strcat(headers, "302 Found\r\n");
            strcat(location, "\r\nLocation: ");
            strcat(location, path);
            strcat(location, "/");
            strcat(body, "<HTML><HEAD><TITLE>302 Found</TITLE></HEAD>\r\n"
                            "<BODY><H4>302 Found</H4>\r\n"
                            "Directories must end with a slash.\r\n"
                            "</BODY></HTML>");
            break;
        case 400:
            strcat(headers, "400 Bad Request\r\n");
            strcat(body, "<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\r\n"
                            "<BODY><H4>400 Bad request</H4>\r\n"
                            "Bad Request.\r\n"
                            "</BODY></HTML>");            
            break;
        case 403:
            strcat(headers, "403 Forbidden\r\n");
            strcat(body, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\r\n"
                            "<BODY><H4>403 Forbidden</H4>\r\n"
                            "Access denied.\r\n"
                            "</BODY></HTML>");
            break;
        case 404:
            strcat(headers, "404 Not Found\r\n");
            strcat(body, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n"
                            "<BODY><H4>404 Not Found</H4>\r\n"
                            "File not found.\r\n"
                            "</BODY></HTML>");
            break;
        case 500:
            strcat(headers, "500 Internal Server Error\r\n");
            strcat(body, "<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\r\n"
                            "<BODY><H4>500 Internal Server Error</H4>\r\n"
                            "Some server side error.\r\n"
                            "</BODY></HTML>");
            break;
        case 501:
            strcat(headers, "501 Not Supported\r\n");
            strcat(body, "<HTML><HEAD><TITLE>501 Not supported</TITLE></HEAD>\r\n"
                            "<BODY><H4>501 Not supported</H4>\r\n"
                            "Method is not supported.\r\n"
                            "</BODY></HTML>");
            break;
    }
    strcat(headers, "Server: webserver/1.0\r\n");
    strcat(headers, "Date: ");
    time_t now = time(NULL);
    char timebuf[128];
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    strcat(headers, timebuf);
    strcat(headers, location); // empty if not 302
    strcat(headers, "\r\nContent-Type: text/html");
    strcat(headers, "\r\nContent-Length: ");
    char str[MAX_LEN] = "";
    sprintf(str, "%ld", strlen(body));
    strcat(headers, str);
    strcat(headers, "\r\nConnection: close\r\n\r\n");

    strcat(response, headers);
    strcat(response, body);
    return response;
}

// This method recieves a path to a directory and retruns its content in html format
char* handleDirectory(char *path) {
    char *response = NULL;

    int perms = isPermitted(path);
    if(perms == -1) {
        response = errorHandle(500, path);
        return response;
    }

    if(perms == 0) {
        response = errorHandle(403, path);
        return response;
    }

    response = (char*)malloc(sizeof(char) * MAX_LEN);
    response[0] = '\0';
    // building body with dir content
    char *body = (char*)malloc(sizeof(char) * MAX_LEN);
    body[0] = '\0';
    strcat(body, "<HTML>\r\n<HEAD><TITLE>Index of ");
    strcat(body, path);
    strcat(body, "</TITLE></HEAD>\r\n\r\n");
    strcat(body, "<BODY>\r\n<H4>Index of ");
    strcat(body, path);
    strcat(body, "</H4>\r\n\r\n");
    strcat(body, "<table CELLSPACING=8>\r\n<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n");

    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    
    if ((dir = opendir(path)) == NULL) {
        perror("opendir() failed");
        free(response);
        response = errorHandle(500, NULL);
        return response;
    }
    // iterate through the directory
    while ((entry = readdir(dir)) != NULL) {
        body = (char*)realloc(body, sizeof(char) * (strlen(body) + MAX_LEN));
        char dir_path[MAX_LEN] = "";
        strncpy(dir_path, path, strlen(path));
        strcat(dir_path, "/");
        strcat(dir_path, entry->d_name);
        if(stat(dir_path, &statbuf) == -1) {
            perror("stat() failed");
            free(response);
            response = errorHandle(500, NULL);
            return response;
        }
        strcat(body, "<tr>\r\n\t<td><A HREF=\"");
        strcat(body, entry->d_name);
        // ------------------------------------------------------ This is for dir content test
        // if(S_ISDIR(statbuf.st_mode)) {
        //     strcat(body, "/");
        // }
        // ------------------------------------------------------
        strcat(body, "\">");
        strcat(body, entry->d_name);
        strcat(body, "</A></td>\r\n\t<td>");
        char mod_time[MAX_LEN] = "";
        strftime(mod_time, sizeof(mod_time), RFC1123FMT, localtime(&statbuf.st_mtime));
        strcat(body, mod_time);
        strcat(body, "</td>\r\n\t");
        if(S_ISREG(statbuf.st_mode)) {
            sprintf(body + strlen(body), "<td>%ld</td>\r\n", statbuf.st_size);
        }
        strcat(body, "</tr>\r\n");
    }
    body = (char*)realloc(body, sizeof(char) * (strlen(body) + MAX_LEN));
    strcat(body, "</table>\r\n<HR>\r\n<ADDRESS>webserver/1.0</ADDRESS>\r\n</BODY></HTML>\r\n");

    // building response
    strcat(response, "HTTP/1.0 200 OK\r\n");
    strcat(response, "Server: webserver/1.0\r\n");

    strcat(response, "Date: ");
    time_t now = time(NULL);
    char date[MAX_LEN] = "";
    strftime(date, sizeof(date), RFC1123FMT, gmtime(&now));
    strcat(response, date);
    
    strcat(response, "\r\nContent-Type: text/html\r\n");

    strcat(response, "Content-Length: ");
    char str[MAX_LEN] = "";
    sprintf(str, "%ld", strlen(body));
    strcat(response, str);

    strcat(response, "\r\nLast-Modified: ");
    struct stat s;
    if(stat(path, &s) == -1) {
        perror("stat() failed");
        free(response);
        response = errorHandle(500, NULL);
        return response;
    }
    char last_mod[MAX_LEN] = "";
    strftime(last_mod, sizeof(last_mod), RFC1123FMT, localtime(&s.st_mtime));
    strcat(response, last_mod);
    strcat(response, "\r\nConnection: close\r\n\r\n");

    response = (char*)realloc(response, sizeof(char) * (strlen(response) + strlen(body) + 1));
    strcat(response, body);
    free(body);
    closedir(dir);
    return response;
}

// This method recieves a path and fd, create an http response and write it to the client
char* handleFile(char *path, int sd) {
    char *response = (char*)malloc(sizeof(char) * MAX_LEN);
    response[0] = '\0';

    int perms = isPermitted(path);
    if(perms == -1) {
        response = errorHandle(500, path);
        return response;
    }

    if(perms == 0) {
        response = errorHandle(403, path);
        return response;
    }

    time_t now = time(NULL);
    char date[MAX_LEN] = "";
    strftime(date, sizeof(date), RFC1123FMT, gmtime(&now));
    char last_mod[MAX_LEN] = "";
    struct stat s;
    if(stat(path, &s) == -1) {
        perror("stat() failed");
        free(response);
        response = errorHandle(500, NULL);
        return response;
    }
    strftime(last_mod, sizeof(last_mod), RFC1123FMT, localtime(&s.st_mtime));

    // creating headers
    strcat(response, "HTTP/1.0 200 OK\r\n");
    strcat(response, "Server: webserver/1.0\r\n");
    strcat(response, "Date: ");
    strcat(response, date);
    if(get_mime_type(path)) {
        strcat(response, "\r\nContent-Type: ");
        strcat(response, get_mime_type(path));
    }
    strcat(response, "\r\nContent-Length: ");
    char str[MAX_LEN] = "";
    sprintf(str, "%ld", s.st_size);
    strcat(response, str);
    strcat(response, "\r\nLast-Modified: ");
    strcat(response, last_mod);
    strcat(response, "\r\nConnection: close\r\n\r\n");

    write(sd, response, strlen(response));

    int fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("open() failed");
        free(response);
        response = errorHandle(500, NULL);
        return response;
    }

    // send the file to socket
    unsigned char buff[MAX_LEN] = "";
    int bytes_read = 0;
    while((bytes_read = read(fd, buff, sizeof(buff))) > 0) {
        write(sd, buff, bytes_read); 
    }

    free(response);
    close(fd);
    close(sd);
    return NULL;
}

// This method checks if there are reading/execute permissions to a path
// if access is ok return 1, else return -1
int isPermitted(char *path) {
    char *tmp = (char*)malloc(sizeof(char) * (strlen(path) + 1));
    if(tmp == NULL) {
        perror("malloc() failed");
        return -1;
    }
    strcpy(tmp, path);

    struct stat s;
    if(stat(tmp, &s) == -1) {
        perror("stat() failed");
        free(tmp);
        return -1;
    }

    // check if a file/directory has read permissions
    if(!(s.st_mode & S_IROTH)) {
        free(tmp);
        return 0;
    }

    // check if rout directories has execute permissions
    char *tmp_dir = (char*)malloc(sizeof(char) * (strlen(path) + 1));
    if(tmp_dir == NULL) {
        perror("malloc() failed");
        return -1;
    }
    strcpy(tmp_dir, dirname(tmp));
    strcpy(tmp, tmp_dir);

    while(strcmp(tmp, ".") != 0) {
        if(stat(tmp, &s) == -1) {
            perror("stat() failed");
            free(tmp);
            free(tmp_dir);
            return -1;
        }
        if(!(s.st_mode & S_IXOTH)) {
            free(tmp);
            free(tmp_dir);
            return 0;
        }
        strcpy(tmp, tmp_dir);
        strcpy(tmp_dir, dirname(tmp));
    }
    free(tmp);
    free(tmp_dir);
    return 1;
}

// This method returns the type of a file
char *get_mime_type(char *name) {
    char *ext = strrchr(name, '.');
    if (!ext) return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    return NULL;
}