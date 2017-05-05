/*
 * Modification History
 *
 * 2004-January-1    Jason Rohrer
 * Created.
 *
 * 2004-March-28    Jason Rohrer
 * Changed postLocalAddress to post to all caches instead of to a random cache.
 *
 * 2004-March-30    Jason Rohrer
 * Changed to avoid re-seeding the random number generator for each query.
 */



#ifndef SIMPLE_WEB_HOST_CACHE_INCLUDED
#define SIMPLE_WEB_HOST_CACHE_INCLUDED



#include "minorGems/util/random/StdRandomSource.h"



/**
 * A simple implementation of a GWebCache client.
 *
 * @author Jason Rohrer.
 */
class SimpleWebHostCache {


        
    public:



        /**
         * Gets seed nodes from the web caches.
         *
         * @param outNodeAddresses pointer to where an array of node addresses,
         *   as \0-terminated strings, should be returned.
         *   The returned array and the addresses must be destroyed by
         *   caller if there is no error
         * @param outNodePorts pointer to where an array of node ports,
         *   should be returned.
         *   The returned array must be destroyed by caller if there
         *   is no error.
         *
         * @return the number of seed nodes being returned, or -1 if there
         *   was an error in getting seed nodes.
         */
        static int getSeedNodes( char ***outNodeAddresses,
                                 int **outNodePorts );



        /**
         * Posts our local address to all of the web caches.
         *
         * Should only be called if this node is not behind a firewall that
         * blocks inbound connections on inLocalPort.
         *
         * Many web caches have mechanisms in place that penalize nodes
         * for posting their address too frequently.
         * Thus, this function should not be called more than once per hour.
         *
         * @param inLocalAddress the address of the local node.
         *   Must be destroyed by caller.
         * @param inLocalPort the port of the local node.
         */
        static void postLocalAddress( char *inLocalAddress, int inLocalPort );



    private:

        static StdRandomSource mRandSource;

        
        
    };



#endif
