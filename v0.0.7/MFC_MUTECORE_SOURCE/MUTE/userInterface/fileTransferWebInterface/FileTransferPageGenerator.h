/*
 * Modification History
 *
 * 2003-September-5   Jason Rohrer
 * Created.
 */



#ifndef FILE_TRANSFER_PAGE_GENERATOR_INCLUDED
#define FILE_TRANSFER_GENERATOR_INCLUDED



#include "minorGems/network/web/server/PageGenerator.h"



#include "minorGems/io/OutputStream.h"
#include "minorGems/io/file/File.h"




/**
 * A MUTE file transfer implementation of an HTML page generator.
 *
 * @author Jason Rohrer
 */
class FileTransferPageGenerator : public PageGenerator {

    public:


        
        /**
         * Constructs a generator.
         */
        FileTransferPageGenerator();


        
        ~FileTransferPageGenerator();
        
                
        
        // implements the PageGenerator interface
        void generatePage( char *inGetRequestPath,
                           OutputStream *inOutputStream );

        char *getMimeType( char *inGetRequestPath );


        
    private:


        
        /**
         * Extracts a contact name and file path from a GET URL path.
         *
         * @param inGetRequestPath the path to extract from.
         *   Must be destroyed by caller.
         * @param pointer to where the contact name should be returned.
         *   Must be destroyed by caller if extraction succeeds.
         * @param pointer to where the file path should be returned.
         *   Must be destroyed by caller if extraction succeeds.
         *
         * @return true if extraction succeeds, or false otherwise.
         */
        char extractContactAndFilePath( char *inGetRequestPath,
                                        char **outContactName,
                                        char **outFilePath );


    };



#endif
