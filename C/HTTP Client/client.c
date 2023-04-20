#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define MAX_LEN 2048

void print_error();
int get_arguments(const char*[], int, char*);
int get_text(const char**, int, char*);
void parseURL(const char**, int, int*, char*, char*);


int main(int argc, char const *argv[]) {
    int post_flag = 0;
    int text_index = -1;
    int port = 0;
    char arguments[MAX_LEN] = "?";
    char content[MAX_LEN] = "";
    char path[MAX_LEN] = "";
    char url_name[MAX_LEN] = "";
    char request[MAX_LEN] = "";

    for(int i = 1; i < argc; i++) {
        char tmp = ' ';
        if(argv[i][0] == '-') {
            if(argv[i][1] == '\0') {
                print_error();
            }
            tmp = argv[i][1];

            switch (tmp)
            {
            case 'r':
                i = get_arguments(argv, i, arguments);
                break;
            case 'p':
                text_index = get_text(argv, i, content);
                i = text_index;
                post_flag = TRUE;
                break;
            default:
                print_error();
                break;
            }
        }
        else {
            regex_t regex;
            regcomp(&regex, "http://", 0);
            if(regexec(&regex, argv[i], 0, NULL, 0) != 0) {
                print_error();
            }
            parseURL(argv, i, &port, url_name, path);
            regfree(&regex);
        }
    }

    if(url_name == NULL || url_name[0] == '\0') {
        print_error();
    }

    if(post_flag == TRUE) {
        strcat(request, "POST ");
    }
    else {
        strcat(request, "GET ");
    }
    strcat(request, path);
    if(strlen(arguments) > 1) {
        strcat(request, arguments);
    }
    strcat(request, " HTTP/1.0\r\nHost: ");
    strcat(request, url_name);
    strcat(request, "\r\n");
    if(post_flag == TRUE) {
        strcat(request, "Content-length:");
        strcat(request, argv[text_index - 1]);
        strcat(request, "\r\n");
    }
    strcat(request, "\r\n");
    if(post_flag == TRUE) {
        strcat(request, content);
    }
    printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request));

    // create new TCP socket
    int sd;
    if((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    // create the struct for the connect() syscall
    struct sockaddr_in peeraddr;

    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(port);
    
    // geeting host IP address
    struct hostent *hp = gethostbyname(url_name);
    if(hp == NULL) {
        herror("gethostbyname failed");
        exit(1);
    }
    peeraddr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;

    // connecting to the server
    if(connect(sd, (struct sockaddr*) &peeraddr, sizeof(peeraddr)) < 0) {
        perror("connect failed");
        exit(1);
    }

    // sending the request to server
    if(write(sd, request, sizeof(request)) < 0) {
        perror("write failed");
        exit(1);
    }

    // reading response from the server
    unsigned char response[MAX_LEN] = "";
    int size = 0;
    while(TRUE) {
        int tmp = read(sd, response, MAX_LEN - sizeof(char));
        if(tmp < 0) {
            perror("read failed");
            exit(1);
        }
        // --------------------------------- CHANGED THIS LINE ---------------------------------------
        write(STDOUT_FILENO, response, tmp);
        // -------------------------------------------------------------------------------------------
        size += tmp;
        if(tmp == 0) {
            break;
        }
    }    
    printf("\n Total received response bytes: %d\n",size); 
    close(sd);
    return 0;
}

// This method finds indexes of the arguments to the GET call putting it in argument_indexes
int get_arguments(const char *argv[], int i, char *arguments) {
    regex_t number_regex;
    regex_t string_args_regex;

    // validating input
    if(argv[i+1] == NULL) { print_error(); }
    regcomp(&number_regex, "[:0-9:]", 0);
    if(regexec(&number_regex, argv[i+1], 0, NULL, 0) != 0) {
        print_error();
    }
    int num_of_arguments = atoi(argv[i+1]);

    regcomp(&string_args_regex, "[a-zA-Z0-9_\\-\\.][=][a-zA-Z0-9_\\-\\.\\=\\!\\@\\#\\$\\^\\*\\/\\]", 0);
    i++;

    if(strlen(arguments) > 1) {
        strcat(arguments, "&");
    }
    
    for(int j = 0; j < num_of_arguments; j++) {
        if(argv[++i] == NULL) {
            print_error();
        }
        // validating arguments
        if(regexec(&string_args_regex, argv[i], 0, NULL, 0) != 0) {
            print_error();
        }
        strcat(arguments, argv[i]);
        if(j != num_of_arguments - 1) {
            strcat(arguments, "&");
        }
    }    
    regfree(&number_regex);
    regfree(&string_args_regex);
    return i;
}

// This method validates the use of -p argument and finds the index of the text to POST
int get_text(const char **argv, int i, char *content) {
    regex_t number_regex;

    // validating input
    if(argv[i+1] == NULL) { print_error(); }
    regcomp(&number_regex, "[:0-9:]", 0);
    if(regexec(&number_regex, argv[i+1], 0, NULL, 0) != 0) {
        print_error();
    }
    int size_of_text = atoi(argv[++i]);

    if(argv[++i] == NULL || strlen(argv[i]) < size_of_text) {
        print_error();
    }
    strncpy(content, argv[i], size_of_text);

    regfree(&number_regex);
    return i;
}

// This method reads the URL extracting the parts we need - URL, port number and path
void parseURL(const char **argv, int i, int *port, char *host, char *path) {
    if(argv == NULL || argv[i] == NULL || port == NULL || host == NULL || path == NULL) {
        return;
    }
    
    // remove http:// from host
    int offset = strlen("http://");
    strncpy(host, argv[i] + offset, strlen(argv[i]) - offset);

    // extract and remove the path from host
    int path_start_index = strcspn(host, "/");
    if(path_start_index != strlen(host)) {
        strcat(path, host + path_start_index);
    }
    else {
        strcat(path, "/");
    }
    host[path_start_index] = '\0';

    char port_str[10] = "";
    int port_start_index = strcspn(host, ":"); // search for port in the host
    if(port_start_index != strlen(host)) {
        // extract the port from host
        strncpy(port_str, host + port_start_index, strlen(host) - port_start_index);
        host[port_start_index] = '\0';

        // if there is no number in port_str
        if(strlen(port_str) <= 1) {
            print_error();
        }

        // ------------------------------------- CHANGED THIS FOR VALGRIND -----------------------------------------
        // fixing port_str - remove ':'
        char new_port_str[MAX_LEN] = "";
        strcpy(new_port_str, port_str + 1);
        strcpy(port_str, new_port_str);
        // strncpy(port_str, port_str + 1, strlen(port_str) - 1);
        // port_str[strlen(port_str) - 1] = '\0';
        // ---------------------------------------------------------------------------------------------------------


        // validating the port_str - "isNumber"
        regex_t number_regex;
        int ret = regcomp(&number_regex, "[0-9]", 0);
        if(ret != 0) {
            printf("regcomp failed\n");
            print_error();
        }
        ret = regexec(&number_regex, port_str, 0, NULL, 0);
        if(ret != 0) {      // port is not a number
            print_error();
        }
        *port = atoi(port_str); // setting the port
        if(*port > 65536) { // port is not a valid port number
            print_error();
        }
        regfree(&number_regex);
    }
    else {
        *port = 80;
    }
}

// This method prints error message to stdout and exit program
void print_error() {
    printf("Usage: client [-p n <text>] [-r n <pr1=value1 pr2=value2 ...>] <URL>\n");
    exit(1);
}