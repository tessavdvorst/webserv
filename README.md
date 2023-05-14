# webserv
> An asynchronous http server

## Table of Contents
* [General Info](#general-information)
* [Features](#features)
* [Setup](#setup)
* [Usage](#usage)
* [Contributors](#contributors)

## General Information
Webserv is a project that requires building an asynchronous HTTP server that works like Nginx. The server is written in C++ and is designed to handle multiple connections simultaneously.

## Features

### Asynchronous I/O with select()
Webserv uses an event-driven architecture based on the select() function to handle incoming connections. This allows the server to handle a large number of connections efficiently and with minimal overhead. 
When a connection is established, the server adds the socket descriptor to the set of file descriptors monitored by select(). 
When a socket becomes available for read or write operations, select() returns the descriptor to the server, allowing it to handle the request.

### Dynamic Content
Webserv supports dynamic content generation through the use of CGI scripts (PHP and Perl). This allows developers to create dynamic web pages that can be customized based on user input.

### Configuration file
Webserv uses a configuration file to specify the server's settings and options. The configuration file is written in a simple syntax and includes options for the server's listening address, port number, virtual hosts, and other settings.

### Error handling
Webserv includes comprehensive error handling, with customizable error pages and support for HTTP error codes.

## Setup
To use Webserv, you must first clone this repository.

`$ git clone https://github.com/tessavdvorst/webserv.git`

Once you have the source code, you can compile it using the provided Makefile.
To compile the program, navigate to the directory containing the source code and type the following command:

`$ make`

This will create an executable file called webserv in the same directory.

## Usage
Once you have compiled the program, you can run it by typing the following command:

`$ ./webserv [config-file]`

This will start the Webserv program and provide you with a server that listens on a specified port and IP address. The [config-file] argument is optional and specifies the location of the configuration file for the server. 
If no configuration file is specified, Webserv will use a default configuration.

## Contributors
[![](https://github.com/PanoramixDeDruide.png?size=30)](https://github.com/PanoramixDeDruide)
    Robert Rinia
