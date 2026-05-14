#include "Cgi.hpp"

Cgi::Cgi( HttpRequest &req, std::map<std::string, std::string> &initCgiMap ) : cgiMap(initCgiMap), req(req), ev(NULL), av(NULL){
    inPipe[0] = -1;
    inPipe[1] = -1;
    outPipe[0] = -1;
    outPipe[1] = -1;
}

std::string Cgi::eMethod( HttpMethod mtd ) const{
    switch (mtd){
        case GET:
            return "GET";
        case POST:
            return "POST";
        case DELETE:
            return "DELETE";
        default:
            return "INVALID";
    }
    return "";
}

int      Cgi::getCONTENT_LENGTH( ) const{
    int res = atoi(ev[1] + 16);
    return res;
}

void    Cgi::closePipe( void ){
    if (inPipe[0] > 2) close(inPipe[0]);
    if (inPipe[1] > 2) close(inPipe[1]);
    if (outPipe[0] > 2) close(outPipe[0]);
    if (inPipe[1] > 2) close(outPipe[1]);
}

void    Cgi::buildEnv( ){
    std::vector<std::string> tEnv;
    std::string             arr[4] = {"REQUEST_METHOD=", "QUERY_STRING=", "CONTENT_LENGTH=", "SCRIPT_NAME="};

    for (int iX = 0; iX < 4; iX++){
        tEnv.push_back(arr[iX]);
        if (arr[iX] == "REQUEST_METHOD=")

            tEnv[iX] += eMethod(req.method);
        else if (arr[iX] == "QUERY_STRING=")
            tEnv[iX] += req.query;
        else if (arr[iX] == "CONTENT_LENGTH="){
            std::ostringstream           sInt;
            sInt << req.body.length();
            tEnv[iX] += sInt.str();
        }
        else
            tEnv[iX] += req.path;
    }
    ev = new char*[tEnv.size() + 1]();
    if (!ev)
        throw std::runtime_error("Run-time Error: alloc memory dynamic failed");
    for (int iX = 0; iX < static_cast<int>(tEnv.size()); iX++){
        ev[iX] = strdup(tEnv[iX].c_str());
        if (!ev[iX])
            throw std::runtime_error("Run-time Error: alloc memory dynamic failed");
    }
    ev[tEnv.size()] = NULL;
}

void    Cgi::buildCmd( ){
    std::string ext, tmp;

    for (int iX = static_cast<int>(req.path.length()) - 1; iX > -1; iX--){
        if (req.path[iX] == '.'){
            for (int i = iX + 1; i < static_cast<int>(req.path.length()); i++){
                ext.push_back(req.path[i]);
            }
            break;
        }
    }
    tmp = cgiMap[ext];
    std::cout << ext << std::endl;
    std::cout << tmp << std::endl;
    av = new char*[3];
    if (!av)
        throw std::runtime_error("Run-time Error: alloc memory dynamic failed");
    *av =  const_cast<char *>(tmp.c_str());
    *(av + 1) = const_cast<char *>(req.path.c_str());
    *(av + 2) = NULL;
}

std::string Cgi::executeCGI(){
    std::string aPPend;
    pid_t       piD;

    buildEnv();
    buildCmd();
    if (pipe(outPipe) == -1 || pipe(inPipe) == -1) throw std::runtime_error("Run-time Error: pipe() failed");
    piD = fork();
    if (piD == -1)
        throw std::runtime_error("Run-time Error: fork() failed");
    else if (piD == 0){
        if (dup2(outPipe[1], STDOUT_FILENO) == -1 || dup2(inPipe[0], STDIN_FILENO) == -1)
            throw std::runtime_error("Run-time Error: dup2() failed");
        closePipe();
        execve(*(av + 0), av, ev);
        exit (1);
    }
    else{
        close(inPipe[0]); close(outPipe[1]);
        if (req.body.empty() == false)
            write(inPipe[1], req.body.c_str(), req.body.length());
        char    buf[1024];
        ssize_t n;
        while ((n = read(outPipe[0], buf, sizeof(buf))) > 0)
            aPPend.append(buf, n);
        close(outPipe[0]); close(inPipe[1]);
        waitpid(piD, NULL, 0);
    }
    return (aPPend);
}

Cgi::~Cgi( void ){
    closePipe();
    if (ev != NULL)
    {
        for (int iX = 0; ev[iX] != NULL; iX++){
            free(ev[iX]);
        }
        delete[] ev;
    }
    if (av != NULL){
        for (int iX = 0; av[iX] != NULL; iX++){
            free(av[iX]);
        }
        delete[] av;
    }
}
