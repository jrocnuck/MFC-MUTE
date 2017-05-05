#
# Modification History
#
# 2003-August-14   Jason Rohrer
# Created.
#
# 2003-August-24   Jason Rohrer
# Added make of crypto library.
#
# 2003-August-26   Jason Rohrer
# Added make of point-to-point layer.
#
# 2003-September-4   Jason Rohrer
# Added make of file transfer layer.
#
# 2003-September-5   Jason Rohrer
# Added web server layer.
#
# 2003-September-8   Jason Rohrer
# Added text-based UI.
#
# 2003-October-14   Jason Rohrer
# Added file sharing app.
#



all:
	cd crypto; make; cd ../layers/messageRouting; make; cd ../pointToPoint; make; cd ../fileTransfer; make; cd ../../userInterface/fileTransferWebInterface; make; cd ../textBased; make; cd ../../otherApps/fileSharing; make; cd userInterface/textBased; make; cd ../../../..

clean:
	cd crypto; make clean; cd ../layers/messageRouting; make clean; cd ../pointToPoint; make clean; cd ../fileTransfer; make clean; cd ../../userInterface/fileTransferWebInterface; make clean; cd ../textBased; make clean; cd ../../otherApps/fileSharing; make clean; cd userInterface/textBased; make clean; cd ../../../..

