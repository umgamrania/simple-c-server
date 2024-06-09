# Simple C Server

## Introduction
I created this mini project as part of the process of learning networking. Web server is perhaps the most basic network project, as it involves using some of the core networking functionalities as well as the quirks of the language itself.

## Working
The server is very basic, it receives http requests on the defined port, and serves static files from the `./static` directory.

In case of big files, I have also implemented chunking.

As of now, I have only identified the http error codes in the case of HTTP 400 (Bad request), HTTP 404 (Not found), HTTP 405 (Method not allowed).

We can optionally change the following parameters in the server.config file:
* Server Port (default 8000)
* Serving directory (default ./static)
* Default index file (default index.html)
* Body chunk size (bytes) (default 1024)