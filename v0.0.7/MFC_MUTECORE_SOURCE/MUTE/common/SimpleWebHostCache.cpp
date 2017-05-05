/*
 * Modification History
 *
 * 2004-January-1    Jason Rohrer
 * Created.
 *
 * 2004-January-2    Jason Rohrer
 * Fixed a memory leak.
 *
 * 2004-January-20    Jason Rohrer
 * Changed to send client version to make compatible with GWC2 caches.
 * Fixed bugs in parameter order.
 *
 * 2004-March-28    Jason Rohrer
 * Changed postLocalAddress to post to all caches instead of to a random cache.
 *
 * 2004-March-30    Jason Rohrer
 * Changed to avoid re-seeding the random number generator for each query.
 *
 * 2004-April-4    Jason Rohrer
 * Fixed redefs of variable i.  Thanks to jrocnuck.
 */



#include "SimpleWebHostCache.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"

#include "minorGems/util/random/StdRandomSource.h"

#include "minorGems/network/web/WebClient.h"

#include "minorGems/network/web/URLUtils.h"



StdRandomSource SimpleWebHostCache::mRandSource;



int SimpleWebHostCache::getSeedNodes( char ***outNodeAddresses,
                                      int **outNodePorts ) {

    const char *clientName = "MUTE";
    
    char *muteVersion = SettingsManager::getStringSetting( "muteVersion" );

    if( muteVersion == NULL ) {
        muteVersion = stringDuplicate( "unknown" );
        }
    if( strcmp( muteVersion, "" ) == 0 ) {
        delete [] muteVersion;
        muteVersion = stringDuplicate( "unknown" );
        }

    
    
    SimpleVector<char *> *webCacheURLs =
        SettingsManager::getSetting( "webHostCaches" );
    

    int numCaches = webCacheURLs->size();

    char seedsFound = false;
    int numSeedsFound = -1;
    char **seedAddresses;
    int *seedPorts;
    
    
    // pick a web cache at random from the list and fetch
    // seed hosts from it.
    
    // keep trying if we fail to fetch from a cache,
    // but try at most _numCaches_ times
    // (thus, if there is only one cache, we will try only once)
    int tryCount = 0;
    int maxTries = numCaches;
    
    
    while( !seedsFound && tryCount < maxTries ) {

        int cachePick = mRandSource.getRandomBoundedInt( 0, numCaches - 1 );

        // compose a URL that asks for a hostfile
        char *urlToGet =
            autoSprintf( "%s?client=%s&version=%s&hostfile=1",
                         *( webCacheURLs->getElement( cachePick ) ),
                         clientName, muteVersion ); 

        // request the URL
        int dataLength;
        char *dataFromWebCache =
            WebClient::getWebPage( urlToGet, &dataLength );

        
        if( dataFromWebCache != NULL ) {

            if( strstr( dataFromWebCache, ":" ) != NULL ) {
                // at least one host in cache

                // spilt up seed nodes into separate strings
                SimpleVector<char *> *hostsAndPorts =
                    tokenizeString( dataFromWebCache );

                SimpleVector<char *> *hostAddresses =
                    new SimpleVector<char*>();
                SimpleVector<long> *hostPorts =
                    new SimpleVector<long>();
                    
                
                numSeedsFound = hostsAndPorts->size();

                int i;
                for( i=0; i<numSeedsFound; i++ ) {
                    char *hostAndPort = *( hostsAndPorts->getElement( i ) );

                    char *pointerToColon = strstr( hostAndPort, ":" );

                    if( pointerToColon != NULL ) {
                        // remove colon
                        pointerToColon[0] = ' ';

                        // read the port
                        int port;
                        int numRead = sscanf( pointerToColon, "%d", &port );

                        if( numRead == 1 ) {
                            // terminate the string where the colon used
                            // to be
                            pointerToColon[0] = '\0';

                            // now extract the host address
                            char *address = stringDuplicate( hostAndPort );

                            // add it to the list of extracted addresses
                            hostAddresses->push_back( address );
                            hostPorts->push_back( (long)port );
                            }
                        
                        }
                    // else no colon found --- bad format
                        
                        
                    delete [] hostAndPort;
                    }
                delete hostsAndPorts;


                
                // if we got some addresses from this web cache, save them
                numSeedsFound = hostAddresses->size();

                if( numSeedsFound > 0 ) {
                    seedAddresses = new char*[ numSeedsFound ];
                    seedPorts = new int[ numSeedsFound ];

                    for( i=0; i<numSeedsFound; i++ ) {
                        seedAddresses[i] = *( hostAddresses->getElement( i ) );
                        seedPorts[i] =
                            (int)( *( hostPorts->getElement( i ) ) );
                        }
                    seedsFound = true;
                    
                    *outNodeAddresses = seedAddresses;
                    *outNodePorts = seedPorts;
                    }
                else {
                    for( i=0; i<numSeedsFound; i++ ) {
                        delete [] *( hostAddresses->getElement( i ) );
                        }
                    }
                
                delete hostAddresses;
                delete hostPorts;
                }
            

            delete [] dataFromWebCache;
            }
        

        delete [] urlToGet;
        tryCount++;
        }

    
    for( int i=0; i<numCaches; i++ ) {
        delete [] *( webCacheURLs->getElement( i ) );
        }
    delete webCacheURLs;


    delete [] muteVersion;

    
    if( seedsFound ) {
        return numSeedsFound;
        }
    else {
        return -1;
        }
    }



void SimpleWebHostCache::postLocalAddress( char *inLocalAddress,
                                           int inLocalPort ) {

    char *addressToPost = autoSprintf( "%s:%d", inLocalAddress, inLocalPort );

    // we must URL-safe encode our address
    char *encodedAddress = URLUtils::hexEncode( addressToPost );

    delete [] addressToPost;


    const char *clientName = "MUTE";
    
    char *muteVersion = SettingsManager::getStringSetting( "muteVersion" );

    if( muteVersion == NULL ) {
        muteVersion = stringDuplicate( "unknown" );
        }
    if( strcmp( muteVersion, "" ) == 0 ) {
        delete [] muteVersion;
        muteVersion = stringDuplicate( "unknown" );
        }
    

    SimpleVector<char *> *webCacheURLs =
        SettingsManager::getSetting( "webHostCaches" );

    int numCaches = webCacheURLs->size();

    
    // post to all of the caches
    int i;
    for( i=0; i<numCaches; i++ ) {

        // compose a URL that posts our address
        char *urlToGet =
            autoSprintf( "%s?client=%s&version=%s&ip=%s",
                         *( webCacheURLs->getElement( i ) ),
                         clientName, muteVersion,
                         encodedAddress ); 

        // request the URL
        int dataLength;
        char *dataFromWebCache =
            WebClient::getWebPage( urlToGet, &dataLength );

        
        if( dataFromWebCache != NULL ) {

            if( strstr( dataFromWebCache, "OK" ) != NULL ) {
                // got OK response

                // don't do anything special for now
                }
            
            delete [] dataFromWebCache;
            }
        

        delete [] urlToGet;
        }
    

    for( i=0; i<numCaches; i++ ) {
        delete [] *( webCacheURLs->getElement( i ) );
        }
    delete webCacheURLs;

    delete [] encodedAddress;

    delete [] muteVersion;
    }
