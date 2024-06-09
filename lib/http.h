#pragma once

#ifndef _HTTP_H
#define _HTTP_H

#include "util.h"
#include "file_system.h"
#include <stdlib.h>

char *default_index = "/index.html";
char *web_dir = "./static";

int LOGGING = 0;
int BODY_CHUNK_THRESHOLD = 1024;

typedef struct {
    char *method;
    char *url;
    char *get_params;
    char *http_version;
} http_request_t;

typedef struct {
    int status_code;
    int content_length;
    char *content_type;
    char *filepath;
    char *body;
} http_response_t;

http_request_t *parse_request(char *req_str) {
    if(LOGGING) printf("RECEIVED REQUEST STR: %ld\n", strlen(req_str));
    
    int lines_count;
    char **lines = split(req_str, "\r\n", &lines_count);

    if(lines_count < 1) 
        // invalid request
        return NULL;
    

    int req_line_count;
    char **req_line = split(lines[0], " ", &req_line_count);

    if(LOGGING) printf("FOUND REQUEST LINE: %s\n", lines[0]);

    if(req_line_count != 3) 
        // invalid request
        return NULL;

    http_request_t *req = (http_request_t*)malloc(sizeof(http_request_t));

    req->method = (char*)malloc(strlen(req_line[0]));
    memset(req->method, 0, strlen(req_line[0]));
    strncpy(req->method, req_line[0], strlen(req_line[0]) + 1);

    if(LOGGING) printf("IDENTIFIED METHOD: %s\n", req->method);

    // url needs to be processed because it might contain get params
    char *question_mark = strrchr(req_line[1], '?');

    if(question_mark != 0) {
        if(LOGGING) printf("FOUND QUERY PARAMS\n");

        // copying query params
        req->get_params = (char*)malloc(strlen(question_mark + 1));
        strcpy(req->get_params, question_mark + 1);
        if(LOGGING) printf("%s\n", req->get_params);

        // terminating the url just before query params
        *question_mark = '\0';
    }

    req->url = (char*)malloc(strlen(req_line[1]));
    memset(req->url, 0, strlen(req_line[1]));
    strncpy(req->url, req_line[1], strlen(req_line[1]) + 1);
    
    if(LOGGING) printf("IDENTIFIED URL: %s\n", req->url);

    req->http_version = (char*)malloc(strlen(req_line[2]));
    memset(req->http_version, 0, strlen(req_line[2]));
    strncpy(req->http_version, req_line[2], strlen(req_line[2]) + 1);

    if(LOGGING) printf("IDENTIFIED HTTP VERSION: %s\n=== END OF FUNCTION ===\n\n", req->http_version);

    return req;
}

void add_body(http_response_t *resp, char *filepath) {
    if(resp->status_code == 200){
        // getting file size
        FILE *f = fopen(filepath, "r");
        fseek(f, 0, SEEK_END);
        long int file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *buf = (char*)malloc(file_size);
        fread(buf, 1, file_size, f);

        resp->body = buf;
        resp->content_length = file_size;

        fclose(f);
    }
}

void send_body(http_response_t *resp, char *filepath, int client_fd) {
    if(resp->status_code == 200){
        // getting file size
        FILE *f = fopen(filepath, "r");
        fseek(f, 0, SEEK_END);
        long int file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if(file_size > BODY_CHUNK_THRESHOLD) {
            char *file_buf = (char*)malloc(BODY_CHUNK_THRESHOLD);
            memset(file_buf, 0, BODY_CHUNK_THRESHOLD);

            int bytes_read = 0;
            int chunk_count = 0;
            while((bytes_read = fread(file_buf, 1, BODY_CHUNK_THRESHOLD, f)) > 0) {
                printf("%d CHUNK SENT\n", ++chunk_count);
                send(client_fd, file_buf, bytes_read, 0);
            }
        } else {
            char *file_buf = (char*)malloc(file_size);
            fread(file_buf, 1, file_size, f);
            send(client_fd, file_buf, file_size, 0);
        }

        fclose(f);
    }
}

char *content_type(char *path) {
    char *last_dot = strrchr(path, '.');

    if(last_dot == 0)
        return "text/plain";

    else if(strcmp(last_dot, ".html") == 0) 
        return "text/html";

    else if(strcmp(last_dot, ".css") == 0) 
        return "text/css";

    else if(strcmp(last_dot, ".js") == 0) 
        return "text/javascript";

    else if(strcmp(last_dot, ".json") == 0) 
        return "application/json";

    else if(strcmp(last_dot, ".png") == 0) 
        return "image/png";

    else if(strcmp(last_dot, ".jpg") == 0) 
        return "image/jpg";

    else if(strcmp(last_dot, ".webp") == 0) 
        return "image/webp";

    else if(strcmp(last_dot, ".ttf") == 0) 
        return "font/ttf";

    return "text/plain";
}

http_response_t *handle_request(http_request_t *req) {
    if(LOGGING) printf("HANDLING REQUEST\n");
    http_response_t *resp = (http_response_t*)malloc(sizeof(http_response_t));
    resp->content_length = 0;
    resp->content_type = "text/plain";

    // validating request
    if(strcmp(req->http_version, "HTTP/1.1")) {
        if(LOGGING) printf("INVALID HTTP VERSION\n");
        resp->status_code = 400;
        return resp;
    }
    
    if(strcmp(req->method, "GET")) {
        if(LOGGING) printf("METHOD NOT ALLOWED: %s\n", req->method);
        resp->status_code = 405;
        return resp;
    }


    if(strcmp(req->url, "/") == 0) {
        if(LOGGING) printf("FOUND / AS URL\n");
        free(req->url);
        req->url = (char*)malloc(strlen(default_index));
        strncpy(req->url, default_index, strlen(default_index));

        if(LOGGING) printf("URL REPLACED WITH: %s\n", req->url);
    }

    if(strstr(req->url, "..") != NULL) {
        // client is trying to be sneaky, adding .. to access parent directory

        resp->status_code = 403; // accessing parent dir is forbidden
        return resp;
    }

    char *filepath = (char*)malloc(strlen(web_dir) + strlen(req->url));
    strcpy(filepath, web_dir);
    strcat(filepath, req->url);
    resp->filepath = filepath;
    if(LOGGING) printf("FILEPATH: %s\n", filepath);

    if(!file_exists(filepath)) {
        if(LOGGING) printf("FILE DOES NOT EXIST\n");
        resp->status_code = 404;
        return resp;
    }

    resp->status_code = 200;
    resp->content_type = content_type(filepath);
    resp->content_length = get_file_size(filepath);
    if(LOGGING) printf("DETERMINED CONTENT TYPE: %s\n", resp->content_type);

    // add_body(resp, filepath);
    // if(LOGGING) printf("ADDED BODY\n");

    return resp;
}

#endif