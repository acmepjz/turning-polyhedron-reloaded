#pragma once

/** \file
*/

#include "UTF8-16.h"
#include <vector>

/** Initialize and create paths. */
void initPaths();

/** External storage path for user preferences, etc. */
extern u8string externalStoragePath;

//Copied from Me and My Shadow, licensed under GPLv3 or above

/** Method that returns a list of all the files in a given directory.
\param path The path to list the files of.
\param extension The extension the files must have.
\param containsPath Specifies if the return file name should contains path.
\return A vector containing the names of the files.
*/
std::vector<u8string> enumAllFiles(u8string path,const char* extension=NULL,bool containsPath=false);

/** Method that returns a list of all the directories in a given directory.
\param path The path to list the directory of.
\param containsPath Specifies if the return file name should contains path.
\return A vector containing the names of the directories.
*/
std::vector<u8string> enumAllDirs(u8string path,bool containsPath=false);

/** Method that will create a directory.
\param path The directory to create.
\return True if it succeeds.
*/
bool createDirectory(const u8string& path);
