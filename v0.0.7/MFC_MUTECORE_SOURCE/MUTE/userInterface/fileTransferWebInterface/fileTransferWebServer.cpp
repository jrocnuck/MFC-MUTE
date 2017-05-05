/*
 * Modification History
 *
 * 2003-September-5   Jason Rohrer
 * Created.
 */


#include "fileTransferWebServer.h"

#include "FileTransferPageGenerator.h"

#include "minorGems/network/web/server/WebServer.h"



// globals
FileTransferPageGenerator *muteFileTransferPageGenerator = NULL;
WebServer *muteFileTransferWebServer = NULL;



void muteFileTransferWebServerStart( int inWebPort ) {
    if( muteFileTransferPageGenerator == NULL &&
        muteFileTransferWebServer == NULL ) {

        muteFileTransferPageGenerator = new FileTransferPageGenerator();

        muteFileTransferWebServer = new WebServer(
            inWebPort,
            muteFileTransferPageGenerator );                                       
        }    
    }



void muteFileTransferWebServerStop() {

    if( muteFileTransferWebServer != NULL ) {
        // this deletes the page generator too
        delete muteFileTransferWebServer;
        
        muteFileTransferWebServer = NULL;
        muteFileTransferPageGenerator = NULL;
        }
    }
