/*
 * Modification History
 *
 * 2002-May-5   Jason Rohrer
 * Created.
 * Added a utility function for receiving data.
 *
 * 2002-May-12   Jason Rohrer
 * Added support for fetching the final (after redirects) URL.
 *
 * 2002-May-26   Jason Rohrer
 * Added support for fetching mime types and content length.
 * Added a function for fetching MIME types alone.
 */

#include "minorGems/common.h"

 
#ifndef WEB_CLIENT_INCLUDED
#define WEB_CLIENT_INCLUDED 



#include "minorGems/network/HostAddress.h"
#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/SocketStream.h"


#include <string.h>
#include <stdio.h>


/**
 * A class that implements a basic web client.
 *
 * @author Jason Rohrer.
 */
class WebClient {



    public:


        
        /**
         * Gets a web page.
         *
         * @param inURL the URL to get as a \0-terminated string.
         *   Must be destroyed by caller if non-const.
         * @param outContentLength pointer to where the length of the content,
         *   in bytes, should be returned.
         *   Useful for binary content which cannot be reliably terminated
         *   by \0.
         * @param outFinalURL pointer to where the actual
         *   URL of the content (after following redirects) should be returned,
         *   or NULL to ignore the final URL.  Defaults to NULL.
         *   The string returned in this location must be destroyed
         *   by caller.
         * @param outMimeType pointer to where the MIME type
         *   of the content should be returned,
         *   or NULL to ignore the MIME type.  Defaults to NULL.
         *   The string returned in this location must be destroyed
         *   by caller.
         *
         * @return the fetched web page as a \0-terminated string,
         *   or NULL if fetching the page fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *getWebPage( char *inURL,
                                 int *outContentLength,
                                 char **outFinalURL = NULL,
                                 char **outMimeType = NULL );

        

        /**
         * Gets the MIME type for a web page without fetching the content.
         *
         * @param inURL the URL to get as a \0-terminated string.
         *   Must be destroyed by caller if non-const.
         *
         * @return the fetched MIME type as a \0-terminated string,
         *   or NULL if fetching the page fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *getMimeType( char *inURL );

        

    protected:


        
        /**
         * Receives data on a connection until the connection is closed.
         *
         * @param inSocketStream the stream to read from.
         *   Must be destroyed by caller.
         * @param outContentLength pointer to where the length of the content,
         *   in bytes, should be returned.
         *   Useful for binary content which cannot be reliably terminated
         *   by \0.
         *
         * @return the received data as a \0-terminated string.
         *   Must be destroyed by caller.
         */
        static char *receiveData( SocketStream *inSocketStream,
                                  int *outContentLength );



        /**
         * Executes a web method.
         *
         * @param inMethod the method to execute (for example, GET or HEAD).
         *   Must be destroyed by caller if non-const.
         * @param inURL the URL to get as a \0-terminated string.
         *   Must be destroyed by caller if non-const.
         * @param outContentLength pointer to where the length of the content,
         *   in bytes, should be returned.
         *   Useful for binary content which cannot be reliably terminated
         *   by \0.
         * @param outFinalURL pointer to where the actual
         *   URL of the content (after following redirects) should be returned,
         *   or NULL to ignore the final URL.  Defaults to NULL.
         *   The string returned in this location must be destroyed
         *   by caller.
         * @param outMimeType pointer to where the MIME type
         *   of the content should be returned,
         *   or NULL to ignore the MIME type.  Defaults to NULL.
         *   The string returned in this location must be destroyed
         *   by caller.
         *
         * @return the fetched web page as a \0-terminated string,
         *   or NULL if fetching the page fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *executeWebMethod( char *inMethod, char *inURL,
                                       int *outContentLength,
                                       char **outFinalURL = NULL,
                                       char **outMimeType = NULL );
        
        
    };




#endif
