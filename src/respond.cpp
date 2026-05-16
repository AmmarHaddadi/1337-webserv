#include "main.hpp"



// suggeston for reponse builder
void respond(SocketMeta &sMeta, HttpRequest &req) { 
    // do route/path matching and find the correspondant route/path in sc
    // if none is found err
    // if not supported method err
    // 
	sMeta.responseBuf = Utils::responseFile(req);

    
    // check final resolved path is in allowed route (/../../abc)
    // if is file and exists
    //   if ends with cgi extension run cgi
    //   else serve file (if GET and file_serve is true)
    // else if path
    //   if default file is defined and exists serve it
    //   else if file browser is true serve the file browser
}