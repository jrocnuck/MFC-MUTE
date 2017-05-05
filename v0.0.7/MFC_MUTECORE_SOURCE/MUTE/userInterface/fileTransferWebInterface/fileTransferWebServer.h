/*
 * Modification History
 *
 * 2003-September-5   Jason Rohrer
 * Created.
 */



/**
 * The API for a web-base interface to MUTE file transfer.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * Before calling this API, application should call
 * muteSeedRandomGenerator and muteStart
 * from the messageRouting layer, then call
 * mutePointToPointStart
 * from the pointToPoint layer, and then call
 * muteFileTransferStart
 * from the fileTransfer layer.
 *
 * After using this API, application should call
 * muteFileTransferStop
 * from the fileTransfer layer, then call
 * mutePointToPointStop
 * from the pointToPoint layer, and then call
 * muteStop
 * from the messageRouting layer.
 * Application may also optionally call
 * muteGetRandomGeneratorState
 * from the messageRouting layer to save the state of the random generator.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_FILE_TRANSFER_WEB_SERVER_API
#define MUTE_FILE_TRANSFER_WEB_SERVER_API



/**
 * Starts the file transfer web server of this node.
 *
 * Must be called before using the file transfer API.
 *
 * Should be called after calling muteSeedRandomGenerator and muteStart
 * from the messageRouting layer, then mutePointToPointStart from the
 * pointToPoint layer, and then muteFileTransferStart from the fileTransfer
 * layer.
 *
 * @param inWebPort the port to listen for web connections on.
 */
void muteFileTransferWebServerStart( int inWebPort);



/**
 * Stops the file transfer web server of this node.
 *
 * Must be called after using the file transfer API.
 *
 * Should be called before calling muteFileTransferStop from the fileTransfer
 * layer, then mutePointToPointStop from the
 * pointToPoint layer, and then muteStop from the messageRouting layer.
 */
void muteFileTransferWebServerStop();



#endif
