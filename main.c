#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "lib/util.h"
#include "lib/http.h"

int SERVER_PORT = 8000;

void *handle_client(void *client_socket) {
    int client = *((int*)client_socket);

    char *req_buf = (char*)malloc(4096);
    memset(req_buf, 0, 4096);

    while(1) {
        int received = recv(client, req_buf, 4096, 0);
        if(str_ends_with(req_buf, "\r\n\r\n")) {
            // received whole GET request
            break;
        }
    }
    
    http_request_t *req = parse_request(req_buf);
    http_response_t *resp = handle_request(req);
    free(req->method);
    free(req->url);
    free(req->http_version);
    free(req_buf);

    char *resp_buf = (char*)malloc(512);
    sprintf(resp_buf, 
                    "HTTP/1.1 %d %s\r\n"
                    "Server: Generic C Server"
                    "Content-Type: %s\r\n"
                    "Content-Length: %d\r\n\r\n", resp->status_code, "OK", resp->content_type, resp->content_length);

    send(client, resp_buf, strlen(resp_buf), 0);
    send_body(resp, resp->filepath, client);
    close(client);

    free(resp_buf);
    free(resp->body);
}

void setup_server() {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    if(server == -1) {
        printf("Failed to create socket\n");
        return;
    }

    int reuseval = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuseval, sizeof(reuseval)) == -1) {
        printf("Failed to set socket option\n");
        return;
    }

    struct sockaddr_in addr = { AF_INET, htons(SERVER_PORT), INADDR_ANY };
    if(bind(server, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Failed to bind\n");
        return;
    }

    if(listen(server, 10) == -1) {
        printf("Failed to listen\n");
        return;
    }

    while(1) {
        int client = accept(server, NULL, NULL);
        pthread_t thread;
        pthread_create(&thread, NULL, &handle_client, &client);
    }
}

void config() {
    FILE *config_file = fopen("server.config", "r");

    if(config_file == NULL) {
        printf("server.config not found. Using default settings\n");
        return;
    }

    char *buf = (char*)malloc(512);
    int bytes_read;
    while(fgets(buf, 512, config_file) != NULL) {
        int bytes_read = strlen(buf);

        if(buf[bytes_read - 1] == '\n')
            buf[bytes_read - 1] = 0;

        int pair_count;
        char **pair = split(buf, "=", &pair_count);

        memset(buf, 0, 512);

        if(pair_count != 2) {
            printf("Invalid config\n%s\n\nKey-value pair required\n", buf);
            printf("%d\n", pair_count);
            continue;
        }

        if(strcmp(pair[0], "PORT") == 0) {
            SERVER_PORT = atoi(pair[1]);
            printf("SERVER PORT SET TO %d\n", SERVER_PORT);
        }

        else if(strcmp(pair[0], "DIR") == 0) {
            web_dir = (char*)malloc(strlen(pair[1]));
            strcpy(web_dir, pair[1]);
            printf("WEB DIR SET TO %s\n", web_dir);
        }

        else if(strcmp(pair[0], "DEFAULT_INDEX") == 0) {
            default_index = (char*)malloc(strlen(pair[1]));
            strcpy(default_index, pair[1]);
            printf("DEFAULT INDEX SET TO %s\n", default_index);
        }

        else if(strcmp(pair[0], "BODY_CHUNK_THRESHOLD") == 0) {
            BODY_CHUNK_THRESHOLD = atoi(pair[1]);
            printf("BODY_CHUNK_THRESHOLD SET TO %d\n", BODY_CHUNK_THRESHOLD);
        }

        else {
            printf("Invalid key: %s\n", pair[0]);
        }
    }

    fclose(config_file);
}

int main() {
    config();
    setup_server();
    return 0;
}