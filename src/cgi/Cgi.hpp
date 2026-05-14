#pragma once

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include "../http_parser/Parser.hpp"

/*
If your form has METHOD="POST" in its FORM tag, your CGI program will receive the encoded form input on stdin. The server will NOT send you an EOF on the end of the data,
instead you should use the environment variable CONTENT_LENGTH to determine how much data you should read from stdin.
*/

class   Cgi {
    private:
        std::map<std::string, std::string>  cgiMap;
        HttpRequest                         &req;
        char                                **ev;
        char                                **av;
        int                                 outPipe[2];
        int                                 inPipe[2];

        std::string     eMethod( HttpMethod mtd ) const;
        int             getCONTENT_LENGTH( ) const;
        void            closePipe( );
        void            buildEnv( );
        void            buildCmd( );

    public:
        Cgi( HttpRequest &req, std::map<std::string, std::string> &initCgiMap );
        ~Cgi( );
        std::string executeCGI( );
};