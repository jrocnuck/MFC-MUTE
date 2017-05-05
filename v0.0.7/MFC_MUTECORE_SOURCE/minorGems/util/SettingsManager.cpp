/*
 * Modification History
 *
 * 2002-September-16    Jason Rohrer
 * Created.
 * Fixed a memory leak.
 *
 * 2002-September-25    Jason Rohrer
 * Added a setSetting function that takes a string value.
 *
 * 2002-September-26    Jason Rohrer
 * Added functions for integer values.
 *
 * 2003-January-11    Jason Rohrer
 * Added default values for float and int settings.
 *
 * 2003-August-24    Jason Rohrer
 * Fixed to remove 499-character limit for a setting value.
 */



#include "SettingsManager.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"



// will be destroyed automatically at program termination
SettingsManagerStaticMembers SettingsManager::mStaticMembers;



void SettingsManager::setDirectoryName( char *inName ) {
    delete [] mStaticMembers.mDirectoryName;
    mStaticMembers.mDirectoryName = stringDuplicate( inName );
    }



char *SettingsManager::getDirectoryName() {
    return stringDuplicate( mStaticMembers.mDirectoryName );
    }



SimpleVector<char *> *SettingsManager::getSetting( char *inSettingName ) {

    char *fileName = getSettingsFileName( inSettingName );
    File *settingsFile = new File( NULL, fileName );

    delete [] fileName;
    
    char *fileContents = settingsFile->readFileContents();

    delete settingsFile;


    
    if( fileContents == NULL ) {
        // return empty vector
        return new SimpleVector<char *>();
        }

    // else tokenize the file contents
    SimpleVector<char *> *returnVector = tokenizeString( fileContents );

    delete [] fileContents;
    
    return returnVector;
    }



char *SettingsManager::getStringSetting( char *inSettingName ) {
    char *value = NULL;
    
    SimpleVector<char *> *settingsVector = getSetting( inSettingName );

    int numStrings = settingsVector->size(); 
    if( numStrings >= 1 ) {

        char *firstString = *( settingsVector->getElement( 0 ) );

        value = stringDuplicate( firstString );
        }

    for( int i=0; i<numStrings; i++ ) {
        char *nextString = *( settingsVector->getElement( i ) );

        delete [] nextString;
        }
    
    delete settingsVector;

    return value;
    }



float SettingsManager::getFloatSetting( char *inSettingName,
                                        char *outValueFound ) {

    char valueFound = false;
    float value = 0;


    char *stringValue = getStringSetting( inSettingName );

    if( stringValue != NULL ) {

        int numRead = sscanf( stringValue, "%f",
                              &value );

        if( numRead == 1 ) {
            valueFound = true;
            }

        delete [] stringValue;
        }

    *outValueFound = valueFound;

    return value;
    }



int SettingsManager::getIntSetting( char *inSettingName,
                                    char *outValueFound ) {

    char valueFound = false;
    int value = 0;


    char *stringValue = getStringSetting( inSettingName );

    if( stringValue != NULL ) {

        int numRead = sscanf( stringValue, "%d",
                              &value );

        if( numRead == 1 ) {
            valueFound = true;
            }

        delete [] stringValue;
        }

    *outValueFound = valueFound;

    return value;
    }



void SettingsManager::setSetting( char *inSettingName,
                                  SimpleVector<char *> *inSettingVector ) {

    FILE *file = getSettingsFile( inSettingName, "w" );

    if( file != NULL ) {

        int numSettings = inSettingVector->size();

        for( int i=0; i<numSettings; i++ ) {

            char *setting = *( inSettingVector->getElement( i ) );

            fprintf( file, "%s\n", setting );
            }

        fclose( file );
        }
    // else do nothing
    }



void SettingsManager::setSetting( char *inSettingName,
                                  float inSettingValue ) {

    char *valueString = new char[ 15 ];
    sprintf( valueString, "%f", inSettingValue );

    setSetting( inSettingName, valueString );
    
    delete [] valueString;
    }



void SettingsManager::setSetting( char *inSettingName,
                                  int inSettingValue ) {

    char *valueString = new char[ 15 ];
    sprintf( valueString, "%d", inSettingValue );

    setSetting( inSettingName, valueString );
    
    delete [] valueString;
    }



void SettingsManager::setSetting( char *inSettingName,
                                  char *inSettingValue ) {
    SimpleVector<char *> *settingsVector = new SimpleVector<char *>( 1 );

    settingsVector->push_back( inSettingValue );

    setSetting( inSettingName, settingsVector );

    delete settingsVector; 
    }



FILE *SettingsManager::getSettingsFile( char *inSettingName,
                                        const char *inReadWriteFlags ) {
    char *fullFileName = getSettingsFileName( inSettingName );
    
    FILE *file = fopen( fullFileName, inReadWriteFlags );

    delete [] fullFileName;
    
    return file;
    }



char *SettingsManager::getSettingsFileName( char *inSettingName ) {
    char **pathSteps = new char*[1];

    pathSteps[0] = mStaticMembers.mDirectoryName;

    char *fileName = new char[ strlen( inSettingName ) + 10 ];

    sprintf( fileName, "%s.ini", inSettingName );
    
    File *settingsFile = new File( new Path( pathSteps, 1, false ),
                                   fileName );

    delete [] fileName;

    // pathSteps copied internally by Path constructor
    delete [] pathSteps;
    

    char *fullFileName = settingsFile->getFullFileName();
    
    delete settingsFile;

    return fullFileName;
    }



SettingsManagerStaticMembers::SettingsManagerStaticMembers()
    : mDirectoryName( stringDuplicate( "settings" ) ) {
    
    }



SettingsManagerStaticMembers::~SettingsManagerStaticMembers() {
    delete [] mDirectoryName;
    }

