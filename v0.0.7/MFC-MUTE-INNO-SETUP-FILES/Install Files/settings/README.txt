This file contains documentation for the various settings files used by MUTE 
File sharing.



-- broadcastProbability.ini
-- Range:  decimal values from 0.0 to 1.0
The probability that a routed message is broadcast to all neighbors.
By randomly choosing to broadcast instead of route, MUTE can search for 
alternate routes even when established routes exist.  However, spurious 
broadcast can also erode established routes and cause a burden on the network.
This is an experimental feature and is turned of (probability of 0) by default.



-- continueForwardProbability.ini
-- Range:  decimal values from 0.0 to 1.0
The probability that a search request in forward mode is switched out of 
forward mode and broadcast.  Forward mode, during which each node passes the
search request on to one neighbor, is used to protect the anonymity of the
search's sender.



-- downloadChunkRetries.ini
-- Range:  integer values that are at least 0
The number of retries to perform for a given download chunk before giving up
completely and counting the download as failed. 



-- downloadFileInfoRetries.ini
-- Range:  integer values that are at least 0
The number of retries to perform when fetching information about a file (the
first step in the download process) before giving up completely and counting 
the download as failed. 



-- downloadRetryFreshRouteProbability.ini
-- Range:  decimal values from 0.0 to 1.0
The probability of marking a chunk request with a FRESH_ROUTE flag when 
sending out a retry request for the chunk.
Messages marked with FRESH_ROUTE clear relevant routing tables as they
travel through the network, allowing fresh routes to be discovered.
 


-- downloadTimeoutCurrentTimeoutWeight.ini
-- Range:  decimal values that are at least 0.0
The weight of the current timeout value in the formula that is used to 
compute a new download chunk timeout.  Larger values cause the timeout to 
change more slowly as transfer speeds change.



-- downloadTimeoutMilliseconds.ini
-- Range:  integer values that are at least 0
The initial download chunk timeout in milliseconds.


 
-- downloadTimeoutRecentChunkWeight.ini
-- Range:  decimal values that are at least 0.0
The weight of the most recent chunk time in the formula that is used to 
compute a new download chunk timeout.  Larger values cause the timeout to 
change more rapidly as transfer speeds change.



-- haveGoodSharePath.ini
-- Range: integer values of 0 (no) or 1 (yes)
Disables a GUI dialog that asks the user to select a shared folder at startup.
MUTE creates this file after it asks the user to select a sharing path at 
first startup, ensuring that it only asks the user for this information once.



-- inboundMessageLimit.ini
-- Range:  decimal values
The maximum number of messages that can be received each second.  -1 specifies
no limit.



-- logConnectionContents.ini
-- Range:  integer values that are at least 0
The number of bytes to log from the beginning of each new connection, or 0
to disable connection contents logging.  The contents will be stored in files
named "seconds.mseconds.in" and "seconds.mseconds.out", for the inbound 
and outbound contents respectively, where "seconds" and "mseconds" are the
time the connection was formed in seconds and milliseconds.


 
-- logConnections.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables logging of information about each new connection, which will be stored
in the file "connectionHistory.log".



-- logDownloadTimeoutChanges.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables logging of information about timeout changes, which will be stored
in a separate file for each download.  For example, if we are downloading 
"myFile.mp3", the timeout log will be stored in a file called 
"timeoutLog_myFile.mp3".



-- logLevel.ini
-- Range:  string values of DEACTIVATE_LEVEL , CRITICAL_ERROR_LEVEL , 
                            ERROR_LEVEL , WARNING_LEVEL , INFO_LEVEL ,
                            DETAIL_LEVEL , or TRACE_LEVEL .
The amount of detail to include in the general log, which is stored in the file
"MUTE.log".  DEACTIVATE_LEVEL disables logging, CRITICAL_ERROR_LEVEL shows
the least detail, and TRACE_LEVEL shows the most detail.



-- logMessageHistory.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables logging of information about each message, which will be stored
in the file "messageHistory.log".



-- logRoutingHistory.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables logging of information about where each message is routed, which will 
be stored in the file "routingHistory.log".



-- maxConnectionCount.ini
-- Range:  integer values that are at least 0
The maximum number of neighbor connections to allow.



-- maxDroppedMessageFraction.ini
-- Range:  decimal values that are at least 0.0
The maximum ratio of dropped message to sent messages that a neighbor node
can have before that neighbor is dropped and replaced with a new connection.
For example, if maxDroppedMessageFraction is 0.5, and a neighbor has been
sent 1000 messages, that neighbor will be replaced if it has dropped more than
500 messages.



-- maxMessageUtility.ini
-- Range:  integer values that are at least 0
The maximum utility counter (UC) value that a message can have before it is 
dropped. UCs are used to limit the effect of broadcast messages, such
as search requests.  As messages travel through the network, their UCs are 
increased as they pass through nodes and when they generate results (or
other benefits for the sending node).  When a message has a high UC, it has
passed through many nodes, generated many results, or some combination of 
both. 



-- maxSubfolderDepth.ini
-- Range:  integer values that are at least 0
The maximum folder depth to share files from in the shared folder.



-- mime.ini
-- Range:  series of extension/MIME type pairs.
The MIME types to send out for various files extensions.


-- muteVersion.ini
-- Range:  string values in integer-and-point notation
The version number of this MUTE node (for example, 0.2.1).

 
-- outboundMessageLimit.ini
-- Range:  decimal values
The maximum number of messages that can be sent each second.  -1 specifies
no limit.



-- port.ini
-- Range:  integer values that are at least 0
The port number that this node will use to accept connections.



-- printSearchSyncTrace.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables console printout of debugging information about search synchronization
mechanisms (semaphores).  Intended to help track down a search freeze bug
in v0.2.2.



-- randomSeed.ini
-- Range:  string values
The random seed that MUTE should use at the next startup.  MUTE creates this 
file when it shuts down.



-- seedHosts.ini
-- Range:  series of address/port pairs
A list of seed nodes to try connecting to on startup.  For example the list 
might look like this:
katcher.2y.net 4900
monolith.2y.net 4900



-- sharingPath.ini
-- Range:  platform dependent path that is URL-safe encoded
The path of the folder to share files from.  This path is platform dependent,
so it uses the platform's directory separation character (for example "/" on
Unix and "\" on Windows).  URL-safe encoding must be used to represent the
path as a string with no spaces.



-- showNiceQuit.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables the display of two Quit options in the wxWindows GUI.  The second 
Quit option is a "nice" quite that waits for all MUTE layers to shut down.  If
disabled, only a single Quit option is shown, and this option results in a 
forced quit.



-- tailChainDropProbability.ini
-- Range:  decimal values from 0.0 to 1.0
The probability that this node will decide at startup to drop all tail
chain messages that it receives. Drop chain mode, during which each node 
either passes the search request on to one neighbor or drops the message, is 
used to protect the anonymity of nodes returning search results.  Because
nodes decide at startup whether to pass or drop all tail chain messages,
a given tail chains is always the same length.  
   


-- targetConnectionCount.ini
-- Range:  integer values that are at least 0
The number of neighbor connections to try to maintain.



-- useMajorityRouting.ini
-- Range:  integer values of 0 (disable) or 1 (enable)
Enables an experimental routing option that always uses the trail with the
strongest "scent" instead of picking from the available trails using 
scent-weighted probabilities.



The following three settings are parameters in this utility counter update 
formula:
newUC = oldUC + 
        alpha * localResultCount + 
        beta * forwardNeighborCount + 
        gamma



-- utilityAlpha.ini
-- Range:  decimal values that are at least 0.0
The weight of generated results in the utility counter update formula.



-- utilityBeta.ini
-- Range:  decimal values that are at least 0.0
The weight of the local branching factor (how many nodes this node will send
the message on to) in the utility counter update formula.



-- utilityGamma.ini
-- Range:  decimal values that are at least 0.0
The constant factor in the utility counter update formula.



-- webHostCachePostIntervalSeconds.ini
-- Range:  integer values that are at least 0
The time to wait before re-posting our local address to the web caches.  Only
affects nodes that are not behind firewalls.  Since some web caches penalize
nodes for posting their address too frequently (for example, more than 
hourly), this value should not be set too low.



-- webHostCaches.ini
-- Range:  series of URLs
A list of URLs for web-based cache scripts that this node can use to learn 
about other nodes.



