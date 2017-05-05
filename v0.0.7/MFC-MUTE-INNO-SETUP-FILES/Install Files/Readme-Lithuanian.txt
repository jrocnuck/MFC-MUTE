MFC MUTE VERSIONS....

----------------------------------------------------------------------------------------------------
--- version 0.0.6 -- 08-13-2005 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  Incorporated Jason Rohrer's DROP TREE security patch code.
2.  Updated About screen with sourceforge address http://www.sourceforge.net/projects/mfc-mute-net
3.  The searching of wildcards '*' now limits the return of results.  The results are 
    randomized, to increase security and to reduce wildcard network search attacks.
4.  Files such as desktop.ini and Thumbs.db will not be included ever in search results.
    There is no real reason to be able to download these files, so they've been removed from
    search results.
5.  Dynamic tab icons at startup based on the files in the icons directory under the main program
    directory.  Below is a list of the files.  If a user wants to change the icon displayed at startup,
    just copy a new icon file in the directory and name it accordingly.
          icons\searchTab.ico
          icons\downloadsTab.ico
          icons\uploadsTab.ico
          icons\connectionsTab.ico
          icons\sharedfilesTab.ico
          icons\settingsTab.ico
6.  Network traffic statistics added on various screens (ENGLISH only currently)
7.  Added Port to Settings dialog. 


----------------------------------------------------------------------------------------------------
--- version 0.0.5 -- 03-04-2005 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  Added colored bar across bottom of GUI that shows connection status.
2.  Put the elapsed time in the caption bar format DAYS:HOURS:MINUTES:SECONDS
3.  Added some code from Nate that saves out of order downloaded chunks to help speed up D/Ls.

----------------------------------------------------------------------------------------------------
--- version 0.0.4 -- 02-25-2005 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  Changed tray icon tooltip from "MFC_MUTE_0.3" to "MFC MUTE"
2.  Fixed 2 bugs in the Download Queue code.
    -1-->   Fixed a bug when loading in the queued downloads from Downloadqueue.ini.
            In version 0.0.3 not all of the queued downloads in Downloadqueue.ini
            would properly be added on restart.
    -2-->   Fixed a bug where a queued download was automatically searched for and found, but
            in the process of attempting to resume the download, there were already 4 downloads
            active from the remote source.  Since only 4 active downloads are allowed from 1 VIP
            the queued item must be placed back into the queue.  The BUG was that the item was
            placed back into the queue, but the status text of the item was not properly updated.
            This gave the (invalid) appearance of many queued items searching at once.            
3.  Increased semaphore timeout values in Download Queue code.            
4.  Fixed code that adds downloads from the Search window that were already listed in the
    download queue.  After further testing of version 0.0.3, I found that this code was broken
    and duplicate items would show up in the download list.
5.  On the Downloads screen, when files are selected, the DELETE key performs the cancel/clear function.
6.  Increased semaphore timeout values in Downloads Dialog code.
7.  In the Shared Files refresh code, added a 1 millisecond sleep call to help prevent there
    thread from starving other threads of CPU cycles.
8.  On the Uploads screen, when files are selected, the DELETE key performs the cancel/clear function.    
9.  Fixed underlying code bugs related to the internal file chunk request timeout/retries.  This
    change should also help the overall network by preventing too many file chunk requests going out
    before allowing enough time for the responses to come back to the client.
10. Danish fixes by Tobias.
11. Turkish from Defnax.



----------------------------------------------------------------------------------------------------
--- version 0.0.3 -- 02-09-2005 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  Added some new icon resources and made some changes (--NGLWARCRY--) to icons. 
2.  Upgraded project files to be easily compiled under Visual Studio .NET.
3.  Added code to save host IPs in seedHosts.ini file at program close. (--Nate--/TSAFA)
4.  Fixed status sorting in downloads dialog.
5.  Added RIGHT-CLICK item in downloads screen that allows a user to make a "queued"
    item be the next item that will be automatically searched.(--Tony--)
6.  Increased amount of time between automated download queue searches to reduce flooding
    of network with search requests.
7.  Adjusted the download queue code so that each time a file is canceled or completed, the
    Downloadqueue.ini file is updated to reflect the state of the downloads.  This improves
    persistance and integrity of the Downloadqueue in cases of portential crashes or quick
    shutdowns.
8.  If a user searches for a file that is queued and finds it in the search window, the user,
    at the search dialog, can now double click the file or click the download and this will
    force the queued item to start downloading again (resume).(--Tony--)
9.  Added a Shared Files Tab to the GUI.  This will show the file and the HASH for the file.
    -- you can export the list to a comma delimited file for external usage
10. Users can now shrink the GUI down to a smaller size with the resizer-gripbar. (--Tony--)
11. Added some of the core changes/fixes from Jason Rohrer's MUTE-0.4 core code.
    -- Host Catcher fixes
    -- Time fixes
    -- Inbound/Outbound Channel fixes
    -- StringBufferOutputStream fixes
    -- crytpo++ compiler related fixes
    -- ChannelReceivingThreadManager fixes
    -- ConnectionMaintainer fixes
    -- Log rollover fixes
    -- formatUtils.cpp fix (added -GiB code)
12. Added changes to connections code for handling time out/dropping.. (--Nate--)
13. Made SIGNIFICANT routing enhancements / routing code cleanup changes as discovered and
    researched by (--NATE--).
    -- these changes should really improve the network
14. Added duplicate file chunk request mitigation code provided by (--Nate--) -->previously, 
    our nodes were receiving duplicate file chunk requests, cluttering the network without
    unnecessary packets.
15. Added a cache mechanism to the Search Result sending code that DRAMATICALLY reduces the CPU
    usage of the application.  The previous version repeatedly searched the shared folders and
    recreated the list of shared files upon each search request.  The new version periodically
    updates a cache that contains the filename, filehash, and filesize.  At first boot up, the
    application updates the cache on the first search request.  On successive search requests
    the application checks for the last time the cache was updated.  Starting with 10 minutes
    as the first delay between cache updates, each successive cache update causes the time between
    cache updates to increase in 50 second intervals until the time between cache updates maxes
    out at 30 minutes.  -- This not only decreases CPU usage, but it also increases overall network
    performance because the application now has much more time to process network messages rather
    than being stuck searching through directories and file names.
16. Increased Hashbuilder dwell time. -- This means after the complete shared files directory has 
    been polled for new files that have not been hashed, the polling timeout is increased.  This
    makes the Hashbuilder use less CPU resources.
17. Uploads Dialog will automatically clear list after 100 items have shown up.  This prevents the
    program from consuming large amounts of ram, and also speeds things up, because each time file
    chunks are sent out, this list has to be iterated to update specific items.
18. The window location and size is now persistant on restarts.  When closing the application, the
    settings file "mfcWindowPlacement.ini" is created.  This file is a comma delimited file with 5 
    integers.  The format is:  
    
    field 1:    0 == show window in tray
                1 == show window normal
    field 2: x coordinate top left of window
    field 3: y coordinate top left of window
    field 4: x coordinate bottom right of window
    field 5: y coordinate bottom right of window
19. Added # Uploads Showing and Total Chunks Uploaded statistics to the uploads dialog.

----------------------------------------------------------------------------------------------------
--- version 0.0.2 -- 11-24-2004 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  *** New DOWNLOAD QUEUE *** that will actively search for new sources for downloads after restarting the application.
    The queue will also handle resuming of downloads much better than in the past.  Now you can start downloads, and 
    leave the program running for some time and the downloads will automatically resume downloading when sources become
    available.

    The download queue data is stored in the file Downloadqueue.ini in the MUTE \settings directory.
    The format is a text file with lines in the format: FILENAME * HASH, where the '*' is the delimiter,
    and FILENAME is the ascii version of the local filename in the MUTE \incoming directory	and the HASH is the ascii 
    HASH of the remote file as recorded upon the initial download of a file.  The Downloadqueue.ini file is loaded into
    the program at startup, and when the program is closed, the Downloadqueue.ini file will be overwritten with the new
    queue information that synchronizes the file with the queue as it was before closing the application.
	
    If a user has files in incoming that he/she wants to add to the Downloadqueue.ini, that is possible even without
    the HASH value.  With MUTE closed, just open the file Downloadqueue.ini with a standard text editor and add the full
    filename of the files in your incoming directory followed by a '*'.  For example if I want to add "supercoolmusic.mp3" 
    to my queue, I can open the Downloadqueue.ini file and add this line to the file:
	
    supercoolmusic.mp3 *

2.  Franc revamed the settings screen to now have an options pane on the left with ability to
    switch between Settings, HTML help tied to the current MUTE WIKI page (that needs to be updated),
    and an About Screen.
	
3.  Franc overhauled the code to allow for dynamic switching of languages without having to restart
    the application.
	
4.  Franc added support for external language files that can be created and placed in the settings directory
    to offer other languages than the current internal ones (ENGLISH, ITALIAN, GERMAN, & FRENCH)
    -- described in document X?? on website ??? 
	
5.  Added a new, cool About Screen with rolling credits.

6.  Fixed a problem with opening two instances of MUTE when both are using the same 
    local port... previously it would crash.
	
7.  "hash_" searches work properly for this version

8.  Faster shutdowns, due to cleaned up code.

9.  Thanks to Franc -- French language translation

10. New shutdown window to let you know when the program is really closed.

11. Internal bug fixes related to providing search results to others.

12. Improved efficiency in download processing, now once inbound messages are handled by the
    appropriate message handlers, the messages are not blindly passed to other message handlers.

13. Usage of pthreads32 library to fix frozen downloads.  You will never NOT be able to cancel a download
    with this version.

14. Quicker search cancelling.

15. Fixed a bug in the HASHING code than can cause a crash if the user starts up with an invalid hash
    directory selected.

16. For MFC MUTE clients only, added code to respond to searches for '*'.  This should return all files 
    in the shared folder.  Use it sparingly, because it will hog up bandwidth, but if you are "feeling lucky"
    like GOOGLE asks you, then try it out.

17. Modified search results sending code so that it is not limited to a string of 28kbytes.  Therefore, people
    will receive results of all files that meet their search criteria.
	
18. Added ICONS to the right click menus on all screens.


----------------------------------------------------------------------------------------------------
--- 08-21-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
1.  Made a large alteration to the way incoming searches work.  Now, when an incoming search
    comes through, if no hash exists for a file, no hash is created.  A separate worker thread
    handles generating hash files based of the user's Shared directory.
    
    The new hashing thread starts when the program starts.  This thread iterates through all the 
    files in the shared directory and as it goes through the files there it checks to see if a hash
    for each file exists.  If the hash doesn't exist yet, it creates the hash and waits 10 seconds 
    until it can create the next hash file.  Additionally, the function that actually hashes the files
    now has a limiter inside of it so that large files do not bogg down the application and render it 
    impossible to use.  Inside this function, as the hashing code loops through, every three seconds, the
    function delays the hashing for a second.  This makes sure that the hashing doesn't starve any other 
    application threads.
    
2.  Created a complete install package with Inno Setup 4.2.7 ISPP 1.2.1.295 and ISTool 4.2.7.
    -- The new install provides for German, Italian, and English installs.
    -- The new install puts an icon on the desktop and creates a program group.
    -- The new install puts the new Seed Node Updater in the install settings directory.
    -- The new install puts batch files on the desktop for the seed node updater that will update
       the user's settings for seed nodes and for web caches.
       
----------------------------------------------------------------------------------------------------       
--- 08-07-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
1.  Support for multi-lingual added via Resource Editor in MS Visual C++.  Now MUTE MFC supports English, German,
    and Italian.

--- 7-29-2004 debug release: binary only currently (source available upon request) -------
1.  7-28 release had a bug regarding downloading while sharing read only directory...
    The program would crash when attempting to move files from incoming directory
	to a read only directory (i.e. CD DRIVE) because file pointers were not validated
	in underlying mute core code.  This has been fixed quickly...  sorry for the quick 
	releases back to back... 

	--no new features in this release
  
----------------------------------------------------------------------------------------------------
--- 7-28 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------
1. settings dialog has changed and offers new settings and web links to important
   MUTE sites.
2. now users can choose individual share, incoming, and hash directories
   - this allows users to share files from read only sources like CDs.. however... currently
     incoming files (once completed) will not be moved to the shared directory (this will be fixed
	 in a future release after people give me some ideas how to combat this)
3. new button on downloads page to explore incoming directory

----------------------------------------------------------------------------------------------------
--- 7-24 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------
1. upload controls settable on settings screen
2. purge hashes button on search dialog
3. if download is already present and active in downloads, won't add same file to download twice
4. fixed the Alt-XXX key problem on search dialog so that if you are searching and click
   Alt-T it will now actually be equivalent to clicking the Stop button, whereas before it wasn't working.

----------------------------------------------------------------------------------------------------
--- 7-14 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------
1. settings dialog, new button for "Delete canceled Files" -- allows you to force canceled/failed downloads to not be deleted from mute's incoming folder.
2. settings dialog, no longer get the messagebox telling you to restart the program after saving settings.
3. search dialog, new filter edit boxes.  Now you can do a bit more serious searching without having to weed through all of the extra garbage from before.
4. downloads dialog, now we are doing resumes on downloads
5. downloads dialog, button to purge mute's incoming directory so you don't have to open it and delete the files yourself. -- may even add a button to purge the hash directory

----------------------------------------------------------------------------------------------------
--- 6-15 release: available with source code -------
----------------------------------------------------------------------------------------------------
0. COLOR icons (let's spice this life up a bit)
1. program opens to connections tab first
2. connection screen invalid host bug (when typing bad ip address) fixed
3. sortable list box on connections tab with headers movable
4. search window color codes files listed (based roughly on type.. aka red for audio, green for video, blue for documents, black for unknown
5. search window adds system icon and file type info to search list (when available)
6. search window, all columns are sortable, headers movable
7. search window, right click download menu
8. search window files added to downloads are highlighted yellow and noted that they are in downloads
9. search window, ability to export search results in comma delimited file
10. search window, button to clear search results
11. search window, can maneuver through search list with cursor keys and press enter key to download selected files.
12. download window, right click menu clear completed, cancel, clear stalled etc. 
13. download window all columns are sortable, headers movable
14. download window, explore shared files button
15. download window, buttons for clearing completed, stalled and for cancelling...
16. download window, color coded items... blue == complete, red == stalled/failed/cancelling/cancelled, green == active download
17. upload window right click menu clear selections
18. upload window all columns are sortable, headers movable
19. upload window remove and clear buttons (only clear finished or stalled) current uploads will reappear on next file chunk request.
20. upload window,color coded items  blue == complete, red == stalled/failed, green == active
21. TOOL tips with extra easy to read info