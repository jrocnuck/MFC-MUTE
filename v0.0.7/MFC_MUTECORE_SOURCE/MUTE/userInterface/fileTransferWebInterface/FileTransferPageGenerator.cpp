/*
 * Modification History
 *
 * 2003-September-5   Jason Rohrer
 * Created.
 *
 * 2003-September-7   Jason Rohrer
 * Fixed bugs in path parsing and directory list generation.
 * Fixed memory leaks.
 *
 * 2003-September-25  Jason Rohrer
 * Changed to handle unknown contacts properly.
 * Added default page with contact list.
 */



#include "FileTransferPageGenerator.h"

#include "MUTE/layers/fileTransfer/fileTransmitter.h"
#include "MUTE/layers/pointToPoint/pointToPointCommunicator.h"




#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/StringBufferOutputStream.h"

#include "minorGems/network/web/URLUtils.h"
#include "minorGems/formats/html/HTMLUtils.h"

#include <string.h>
#include <time.h>
#include <stdio.h>





FileTransferPageGenerator::FileTransferPageGenerator() {
    
    }



FileTransferPageGenerator::~FileTransferPageGenerator() {
    }



char FileTransferPageGenerator::extractContactAndFilePath(
    char *inGetRequestPath,
    char **outContactName,
    char **outFilePath ) {

    char returnValue = false;
    
    int numParts;
    char **pathParts = split( inGetRequestPath, "/", &numParts );
    
    if( numParts >= 2 ) {
        *outContactName = stringDuplicate( pathParts[1] );

        if( numParts >= 3 ) {
            // join remaining parts
            char **partsToJoin = &( pathParts[2] );
            char *filePath = join( partsToJoin, numParts - 2, "/" );
            
            if( strlen( filePath ) == 0 ) {
                // empty path is root
                delete [] filePath;
                filePath = stringDuplicate( "/" );
                }

            *outFilePath = filePath;
            }
        else {
            // no path, just user name, so use root path
            *outFilePath = stringDuplicate( "/" );
            }
        returnValue = true;
        }

    
    for( int i=0; i<numParts; i++ ) {
        delete [] pathParts[i];
        }
    delete [] pathParts;

    
    return returnValue;
    }



// callback for incoming files
void fileTransferPageGenerator_internalFileHandler( unsigned char *inChunk,
                                                    int inChunkLengthInBytes,
                                                    void *inExtraParam ) {

    // extra param is output stream where file should be written
    OutputStream *outputStream = (OutputStream *)inExtraParam;

    outputStream->write( inChunk, inChunkLengthInBytes );
    }



void FileTransferPageGenerator::generatePage( char *inGetRequestPath,
                                              OutputStream *inOutputStream ) {

    if( strcmp( inGetRequestPath, "/" ) == 0 ) {
        // a request for the root path
        // display the default page (contact list)

        int numContacts;
        char **contacts = muteGetContactList( &numContacts );

        inOutputStream->writeString(
            "<HTML>"
            "<HEAD><TITLE>MUTE Web Interface</TITLE></HEAD>"
            "<BODY BGCOLOR=#FFFFFF TEXT=#000000"
            "LINK=#0000FF ALINK=#FF00FF>"
            "<H1>Known Contacts:</H1>" );

        for( int i=0; i<numContacts; i++ ) {

            char *contactLink = autoSprintf(
                "<A HREF=\"/%s\">%s</A><BR>\n",
                contacts[i], contacts[i] );

            inOutputStream->writeString( contactLink );

            delete [] contactLink;
            

            delete [] contacts[i];
            }
        delete [] contacts;

        inOutputStream->writeString( "</BODY></HTML>" ); 
        
        return;
        }
    
    char *contactName;
    char *filePath;
    char partsFound = extractContactAndFilePath( inGetRequestPath,
                                                 &contactName,
                                                 &filePath );

    if( !partsFound ) {
        inOutputStream->writeString( "Badly formatted URL: " );
        inOutputStream->writeString( inGetRequestPath );

        return;
        }
    
    char isDirectory;
    int length;
    int chunkCount;
    char *mimeType;
    
    int status = muteGetFileInfo( contactName, filePath,
                                  &isDirectory, &length,
                                  &chunkCount,
                                  &mimeType );

    if( status == MUTE_FILE_CONTACT_NOT_REACHABLE ) {
        inOutputStream->writeString( "Could not reach contact: " );
        inOutputStream->writeString( contactName );

        delete [] mimeType;
        delete [] filePath;
        delete [] contactName;

        return;
        }
    else if( status == MUTE_FILE_NOT_FOUND ) {
        inOutputStream->writeString( "File not found: " );
        inOutputStream->writeString( filePath );

        delete [] mimeType;
        delete [] filePath;
        delete [] contactName;

        return;
        }
    else if( status == MUTE_FILE_CONTACT_UNKNOWN ) {
        inOutputStream->writeString( "Contact name unknown: " );
        inOutputStream->writeString( contactName );

        delete [] mimeType;
        delete [] filePath;
        delete [] contactName;

        return;
        }
    
    // else file found

    // ignore mime type
    delete [] mimeType;


    if( isDirectory ) {
        char *absoluteDirPath;
        
        if( inGetRequestPath[ strlen( inGetRequestPath ) - 1 ] != '/' ) {
            // path does not end with /
            // fix it
            absoluteDirPath = autoSprintf( "%s/", inGetRequestPath );
            }
        else {
            absoluteDirPath = stringDuplicate( inGetRequestPath );
            }

        
        int numEntries;
        char **dirList = muteGetDirectoryListing( contactName,
                                                  filePath,
                                                  &numEntries );

        if( dirList == NULL ) {
            inOutputStream->writeString(
                "Failed to fetch file list from contact." );
            }
        else {
            // generate HTML for the directory list

            char *pageHeadHTML = autoSprintf(
                "<HTML>"
                "<HEAD><TITLE>File List For %s : %s</TITLE></HEAD>"
                "<BODY BGCOLOR=#FFFFFF TEXT=#000000"
                "LINK=#0000FF ALINK=#FF00FF>"
                "<H1>Contact: %s<BR>Folder: %s</H1>",
                contactName, filePath, contactName, filePath );
            
            
            inOutputStream->writeString( pageHeadHTML );
            delete [] pageHeadHTML;

            for( int i=0; i<numEntries; i++ ) {
                char *entryHTML = autoSprintf(
                    "<A HREF=\"%s%s\">%s</A><BR>",
                    absoluteDirPath, dirList[i], dirList[i] );
                inOutputStream->writeString( entryHTML );
                delete [] entryHTML;

                delete [] dirList[i];
                }

            inOutputStream->writeString( "</BODY></HTML>" );                

            delete [] dirList;
            }

        delete [] absoluteDirPath;
        }
    else {
        // fetch and send data file

        char fetched = muteGetFile(
            contactName, filePath,
            fileTransferPageGenerator_internalFileHandler,
            (void *)inOutputStream );

        if( !fetched ) {
            inOutputStream->writeString(
                "Failed to fetch file from contact." );
            }
        }

    delete [] contactName;
    delete [] filePath;
    }



char *FileTransferPageGenerator::getMimeType( char *inGetRequestPath ) {

    char *contactName;
    char *filePath;
    char partsFound = extractContactAndFilePath( inGetRequestPath,
                                                 &contactName,
                                                 &filePath );

    if( !partsFound ) {

        return stringDuplicate( "text/html" );
        }
    
    char isDirectory;
    int length;
    int chunkCount;
    char *mimeType;
    
    int status = muteGetFileInfo( contactName, filePath,
                                  &isDirectory, &length,
                                  &chunkCount,
                                  &mimeType );
    delete [] filePath;
    delete [] contactName;

    if( status == MUTE_FILE_CONTACT_NOT_REACHABLE ) {
        delete [] mimeType;

        return stringDuplicate( "text/html" );
        }
    else if( status == MUTE_FILE_NOT_FOUND ) {
        delete [] mimeType;
    
        return stringDuplicate( "text/html" );
        }
    else if( status == MUTE_FILE_CONTACT_UNKNOWN ) {
        delete [] mimeType;

        return stringDuplicate( "text/html" );
        }
    
    // else found
    return mimeType;
    }


