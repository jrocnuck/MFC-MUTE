#include "MUTE/otherApps/fileSharing/fileShare.h"

#ifndef _MUTE_SEARCH_THREAD_
#define _MUTE_SEARCH_THREAD_

// callback for incoming search results
char searchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam );



/**
 * Thread that runs the MUTE search.
 */
class SearchThread : public Thread {

    public:
        /**
         * Constructs and starts a search thread
         * @param inSearchString must be destroyed by caller.
         * @param inExtraHandlerParam parameter to pass to results handler.
         */
        SearchThread( const char *inSearchString, void *inExtraHandlerParam )
            : mSearchString( stringDuplicate( inSearchString ) ),
              mExtraHandlerParam( inExtraHandlerParam ) {
            
            start();
            }

        /**
         * Joins and destroys this search thread.
         */
        ~SearchThread() {
            join();

            delete [] mSearchString;
            }

        // implements Thread interface
        void run() {
            muteShareSearch( mSearchString, searchResultHandler,
                             mExtraHandlerParam,
                             1000 );
            }
        
    private:
        char *mSearchString;
        void *mExtraHandlerParam;
        
    };

#endif //_MUTE_SEARCH_THREAD_