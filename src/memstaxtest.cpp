/*!
 * \author  Manoel McCadden
 * \date    3-9-25
 * \file    memstaxtest.cpp
 *
 * \details
 *    Holds a series of different unit tests for memstax functionality 
 *    created in order to ensure that individual components are working
 *    as inteded and can be tested in case of modificatinos in order to
 *    more easily detect sources of issues and complications
 */

#include <cstring>
#include <assert.h>

#include "MemStax.h"

using namespace std;
using namespace Stax;

static void UnitTest_MemTrace_Console(); 
static void UnitTest_MemTrace_File();
static void UnitTest_MemTrace_CustomTrace();
static void UnitTest_MemTrace_FileNoClear();
static void UnitTest_MemTrace_FileDifferent();
static void UnitTest_MemTrace_CheckGetFile();

static MEMERR CustomMemTrace(const string &, fstream *);

int main(int argc, char** argv)
{
  // Init's a bool which determines wether or not to run all tests
  bool runAllTests = false;

  // If no arguments are given then we will run all tests
  if(argc < 1) {
    runAllTests = true;
  }

  // Find out if trace tests were called or if we are running all unit tests.
  // If we are then run mem trace tests :)
  if(strncmp(argv[0], "MemTrace", sizeof("MemTrace")) || runAllTests)
  {
    // Test for console iostream trace
    UnitTest_MemTrace_Console();
    // Test for file iostream trace
    UnitTest_MemTrace_File();
    // Test for file iostream into custom trace log
    UnitTest_MemTrace_CustomTrace();
    // Reopen the same trace file without clearing the trace log
    UnitTest_MemTrace_FileNoClear();
    // Write to a different file with a default trace message
    UnitTest_MemTrace_FileDifferent();
    // Rewrite to previous file
    UnitTest_MemTrace_FileNoClear();
    // Test for getting a file, directly writting to it
    // , and writting with log again with seperate asserts. Since they relate
    // closly and may cause odd interactions with one another test all 3.
    UnitTest_MemTrace_CheckGetFile();
  }

  return 0;
}

using namespace Stax;

// Trace unit tests
void UnitTest_MemTrace_Console()
{
  // Create a trace log with default parameters
  // (no file path, default trace func, no error tracking)
  MemTrace trace;

  // Log a trace message
  MEMERR error = trace.LogMessage("Test Default Trace Log");

  // Make sure that there is no error and stop the application by throwing
  // a runtime error if there is.
  assert(error == MEMERR_NO_ERR);
}

void UnitTest_MemTrace_File()
{
  MEMERR error;

  // Create a trace log with a trace file. Check for errors
  MemTrace trace("./log/Trace.txt", true, &error);

  // Check for an error in txt creation
  assert(error == MEMERR_NO_ERR);

  // Clear the file to ensure no execess data is present
  trace.ClearFile();

  // Log a trace message
  error = trace.LogMessage("Test Default Trace Log");

  // Make sure that there is no error and stop the application by throwing
  // a runtime error if there is.
  assert(error == MEMERR_NO_ERR);
}

void UnitTest_MemTrace_CustomTrace()
{
  MEMERR error;

  // Create a trace log with a trace file. Check for errors
  MemTrace trace("", false, &error, CustomMemTrace);

  // Check for an error in txt creation
  assert(error == MEMERR_NO_ERR);

  // Log a trace message
  error = trace.LogMessage("Test Default Trace Log");

  // Make sure that there is no error and stop the application by throwing
  // a runtime error if there is.
  assert(error == MEMERR_NO_ERR);
}

void UnitTest_MemTrace_FileNoClear()
{
  MEMERR error;

  // Create a trace log with a trace file. Check for errors. DO NOT CLEAR
  MemTrace trace("./log/Trace.txt", false, &error);

  // Check for an error in txt creation
  assert(error == MEMERR_NO_ERR);

  // Log a trace message (notated as 2 for difference)
  error = trace.LogMessage("Test Default Trace Log (2)");

  // Make sure that there is no error and stop the application by throwing
  // a runtime error if there is.
  assert(error == MEMERR_NO_ERR);
}

void UnitTest_MemTrace_FileDifferent()
{
  MEMERR error;

  // Create a trace log with a trace file. Check for errors. 
  MemTrace trace("./log/Trace2.txt", true, &error);

  // Check for an error in txt creation
  assert(error == MEMERR_NO_ERR);

  // Log a trace message (notated as 2 for difference)
  error = trace.LogMessage("Test Default Trace Log");

  // Make sure that there is no error and stop the application by throwing
  // a runtime error if there is.
  assert(error == MEMERR_NO_ERR);
}

void UnitTest_MemTrace_CheckGetFile()
{
  MEMERR error;

  // Create a trace log without clearing it and check for errors.
  MemTrace trace("./log/Trace.txt", false, &error);

  // Make sure no errors occured.
  assert(error == MEMERR_NO_ERR);

  // Get a pointer to the file
  std::fstream *file = trace.GetFile(&error);

  // Make sure no errors occured and the file is open and valid
  assert(error == MEMERR_NO_ERR);
  assert(file->is_open());

  // Write directly to file skipping trace log
  *file << "Testing Get File Direct Write" << endl;

  // Log to file
  error = trace.LogMessage("Testing Get File Log Write");

  // Make sure no errors occured
  assert(error == MEMERR_NO_ERR);
}

MEMERR CustomMemTrace(const string &msg, fstream *)
{
  // Ignores wether or not there is a file and print to the console
  cout << "Trace: " << msg << endl;

  return MEMERR_NO_ERR;
}
