
// \date    3-6-25
// \file    MemStax.h
//
// \details
//    A memory manager that attempts to keep memory in the stack
//      to minimize heap allocation for memory/program saftey.
//    Additionally, we still allow for heap allocaiton but it must
//      be exlicetly requested and is not recommened.

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>

namespace Stax
{
  /*!
   * An enum used to report different errors that occur during different
   * memory related functions.
   */
  enum MEMERR 
  {
    //! No memory errors have occured
    MEMERR_NO_ERR = 0
    //! Unknown memory error has occured
    , MEMERR_UNKNOWN
    //! Memory limit has been reached
    , MEMERR_OUT_OF_MEM
    //! An invalid pointer or a pointer to invalid memory has been provided
    , MEMERR_INVALID_MEM
    //! Memory has been corrupted by overwriting beyond designated object space
    , MEMERR_CORRUPT_MEM
    //! A pointer that already has data is attempted to be reallocated
    , MEMERR_DOUBLE_ALLOC
    //! Either hasn't been initalized or failed to initalize
    , MEMERR_UNINITALIZED
    //! A class hasn't been properly initalized
    , MEMERR_INVALID_FUNCTION_PARAMETER 
    //! An invalid file or filepath has been provided
    , MEMERR_INVALID_FILE
  };

  /*!
   * An enum used to send enable/disable certain funcitonalities within
   * the heapmem, static, and dynamic managers.
   */
  enum MEMFLAGS
  {
    MEMFLAGS_NONE = 0x00
    , MEMFLAGS_DISABLE_DEBUG_MSG = 0x01
    , MEMFLAGS_OVERRIDE_DOUBLE_ALLOC = 0x02
  };

  /*!
   * An enum used to send messages to the memory callback.
   */
  enum MEMCALL
  {
    MEMCALL_ALLOC = 0 
    , MEMCALL_DEALLOC
  };

  //! A definition used for callback functions
  using memcallbackfunc = MEMERR (*)(const MEMCALL &, const size_t &);
  //! A definition used for trace functions
  using memtracefunc = MEMERR (*)(const std::string &, std::fstream *);

  /*!
   * \class MemTrace
   * \brief
   *    The memtrace class implements tracing functionality for memory 
   *    allocation, deallocation, and other messages sent by the callback
   *    class.
   *
   *    Operations:
   *    - Printing a trace message to the file or console
   *    - Getting the most recent trace message
   *    - Clearing a trace file
   *
   * \deprecated
   *    N/A
   *
   * \bug
   *    N/A
   */
  class MemTrace
  {
    public:
      /*!
       * Ctor function for memory trace class.
       *
       * \param tracePath
       *    The file path for the trace file that will be written to.
       *    Defaults as an empty string which means that trace is logged
       *    to the console.
       *
       * \param newTraceFunc
       *    A pointer to a trace function as defined by memtracefunc
       *    which takes in a message and prints it to either the console
       *    or the desired file as the givne file path on construction.
       */
      MemTrace(const std::string &tracePath = "", const bool &clearContents = false
          , MEMERR *error = nullptr, memtracefunc newTraceFunc = PrintMessage)
        : traceLog(false), traceFunc(newTraceFunc), filePath(tracePath)
      {
        // Checks if a valid trace function has been given in order to enable
        // trace functionality.
        if(!newTraceFunc)
        {
          // If the trace function given is not valid then report an error to the user
          // if they have given an address for error checking
          if(error)
          {
            *error = MEMERR_INVALID_FUNCTION_PARAMETER;
          }
        }
        
        // Enable trace logging functinatlty
        traceLog = true;
        
        // Check if trace path is empty if it is then we are done
        if(tracePath.empty())
        {
          if(error)
          {
            *error = MEMERR_NO_ERR;
          }
          return;
        }

        // Check if clearing of contetns was requested by the user
        if(clearContents)
        {
          ClearFile();
        }
        // If the contents arn't being deleted open like normal
        else
        {
          traceFile.open(tracePath
              , std::ios::out | std::ios::app);
        }

        // Check to make sure that the trace file is opened properly
        if(!traceFile.is_open())
        {
          // Disable trace if the file is invalid
          traceLog = false;
          if(error)
          {
            *error = MEMERR_INVALID_FILE;
          }
        }

        // Set error to no error if everything has passed
        if(error)
        {
          *error = MEMERR_NO_ERR;
        }
      }
      /*!
       * The dtor for memtrace which disables logging to the trace funciton 
       * and closes any open files.
       */
      ~MemTrace()
      {
        // If the trace function is valid then disable tracing on destruction
        if(traceFunc)
        {
          traceLog = false;
        }

        // Close the trace file if it exists and is open
        if(traceFile.is_open())
        {
          traceFile.close();
        }
      }

      /*!
       * A method which tells the trace instance to send a message to
       * it's trace function. Sends the file address if it exists and is open.
       *
       * \param msg
       *    The message we wanted to be traced.
       *
       * \returns
       *    Returns a memory error.
       */
      MEMERR LogMessage(const std::string &msg)
      {
        // Check that the error has been inited
        if(!traceLog)
        {
          return MEMERR_UNINITALIZED;
        }

        // Make sure that the trace file is open...
        // if it is not then set the file to nullptr 
        if(traceFile.is_open())
        {
          return traceFunc(msg, &traceFile);
        }
        else
        {
          return traceFunc(msg, nullptr); 
        }
      }

      /*!
       * Clears the current trace file of the trace instance.
       * If the file is open then closes it and then clears it, if not
       * then simply clears and reopens for appending.
       *
       * \returns
       *    A memory error result.
       */
      MEMERR ClearFile()
      {
        // Make sure there is a valid path
        if(filePath.empty())
        {
          return MEMERR_INVALID_FILE;
        }

        // Close the file if it is currently open
        if(!traceFile.is_open())
        {
          traceFile.close();
        }
        // Reopen the file for truncation to delete contents
        traceFile.open(filePath, std::ios::out | std::ios::trunc);
        if(!traceFile.is_open())
        {
          return MEMERR_INVALID_FILE;
        }
        // Close the file and reopen for appending as normal
        traceFile.close();
        traceFile.open(filePath, std::ios::out | std::ios::app);
        if(!traceFile.is_open())
        {
          return MEMERR_INVALID_FILE;
        }

        return MEMERR_NO_ERR;
      }
 
      /*!
       * Gets a pointer to the file being used within the 
       * current trace instace.
       *  
       * \param error
       *    A pointer or address that gets updated with error results
       *    if given.
       *
       * \returns
       *    A pointer or address to the file in use if any. 
       *    Returns nullptr if there is no file in use.
       */
      std::fstream *GetFile(MEMERR *error = nullptr)
      {
        // Check for a valid trace file being open 
        if(!traceFile.is_open())
        {
          if(error)
          {
            *error = MEMERR_INVALID_FILE;
          }

          return nullptr;
        }
  
        if(error)
        {
          *error = MEMERR_NO_ERR;
        }

        return &traceFile;
      }

    private:
      /*!
       * A static method of memtrace which is the default trace function.
       * This function prints a message either to a file or to console while
       * checking to make sure the file is valid if requested.
       *
       * \param msg
       *    The message being printed to the desired locaiton
       *
       * \param file
       *    The file we are using (if any). If nullptr then print to console
       *
       * \returns
       *    A memory error result telling the user wether or not an error has
       *    occured.
       */
      static MEMERR PrintMessage(const std::string &msg
          , std::fstream *file = nullptr)
      {
        // Check if a valid trace file address is given
        // if not then print to console
        if(!file)
        {
          std::cout << msg << std::endl;
        }
        // If the file is valid then we print to the file 
        else
        {
          *file << msg << std::endl;
        }

        return MEMERR_NO_ERR;
      }

      //! A bool for telling the log system wether or no logging is active.
      bool traceLog;
      //! A pointer to a functino which matches the trace func definition
      memtracefunc traceFunc;
      //! A file that can be used for writting trace messages
      std::fstream traceFile;
      //! A string that holds the file path of the trace file.
      std::string filePath;
  };

  class MemCallback
  {
    public:
      /*!
       * Creates a new callback for memory allocation and deallocation.
       * This can be either a general callback for all MemStax memory types
       * or a specialized callback for a single type. Additionally the user
       * may replace the default callback with their own callback funciton
       * with different internal functionality (for example: if they have
       * a sort of custom event or message handler that needs to be notified.)
       *
       * \param callbackFunc
       *    A callback funciton as defined by memcallbackfunc that will recieve
       *    messages from allocs and deallocations
       */
      MemCallback(const bool &logging = false
          , memcallbackfunc newCallback = CallbackFunc, MEMERR *error = nullptr)
        : callback(newCallback), callbackLog(logging) 
      {
        // Check for a valid callback function enabling callback funcitonality
        // if it is
        if(callback)
        {
          callbackInit = true;
        }
        // If not then reports an error if an error address has been given
        else if (error)
        {
          *error = MEMERR_INVALID_FUNCTION_PARAMETER;
        }
      }
      ~MemCallback()
      {
        if(callback)
        {
          callbackInit = false;
        }
      }
  
      /*!
       * Returns a pointer to the callback function
       */
      memcallbackfunc GetMemCallback()
      {
        if(!callback)
        {
          // TODO: Add and replace with memory exceptions
          return nullptr;
        }

        return callback;
      }



    private:
      // TODO: Create some sort of trace function that can take in a filepath
      //  and write to it if requested by user. If no filepath is given then
      //  the trace will simply write to the console using iostream if logging
      //  is enabled by the user. Additionally the user can add their own trace
      //  log to the callback function to handle messaging them self without
      //  handing the tracking of allocations, deallocations, and more!
      static MEMERR CallbackFunc(const MEMCALL &msg, const size_t &memSize)
      {
        switch(msg)
        {
          case MEMCALL_ALLOC:
            ++allocs;
            memInUse += memSize;
            break;
          case MEMCALL_DEALLOC:
            ++deallocs;
            memInUse -= memSize;
            break;
        }

        return MEMERR_NO_ERR;
      }
      static uint32_t allocs, deallocs;
      static size_t memInUse;
      static bool callbackInit;
      
      memcallbackfunc callback;
      bool callbackLog;
  };

  /*!
   * Creates raw memory in the heap that is not directly handled by the class
   */
  class HeapMem 
  {
    public:
      HeapMem()
        : allocCount(0), deallocCount(0), totalHeapSize(0)
        , heapInitalized(false), memFlags(0)
      {

      }
      ~HeapMem()
      {

      }

      // TODO: on initalization have it take in a callback funciton which 
      //  tracks allocations, deallocations, and object allocation size.
      //  Maybe a templatized class (based on Heap, Static, and Dynamic)
      //  This allows us to track allocations seperate from the memory
      //  allocation object and have it be override more easily by the
      //  user in case they wanted to do something else and minimize overhead.
      MEMERR InitalizeHeapMem()
      {
        // Turn on the heap
        heapInitalized = true;

        return MEMERR_NO_ERR;
      }
      MEMERR TerminateStackMem()
      {
        // Turn off the heap
        heapInitalized = false;

        return MEMERR_NO_ERR;
      }

      /*!
       * Takes in a pointer and creates a raw pointer that is returned
       * to the user to be handled. Additionally keeps track of allocs and
       * double allocation and returns errors properly in case an issue
       * occurs.
       *
       * TODO: Create a memory tracker that takes in strings and has it's own
       *  flags. With the possibilty to be polymorphized so that more
       *  custimization can be made.
       *
       * \param p_Obj
       *  A pointer that will be filled with a new obj allocated on success
       * \param callbackFun
       *  A pointer to a callback function that is not required but will
       *  send a message to the given callback func if provided. Callback
       *  may keep track of allcoations as well but class will continue
       *  to keep personal count.
       *
       * An example of the callback funciton as it will be taken by 
       * the function.
       *
       * void (*)(const std::string &string)
       *
       * \returns 
       *  A MEMERR indicating if any errors occured during allocation
       *  which helps the use avoid doing unnecessary error checking
       *  and bloating their own code/program.
       */
      template<typename T>
      MEMERR Allocate(T* p_Obj
          , void (*callback)(const std::string &) = nullptr)
      { 
        // Check for a double allocation to avoid allocating over an 
        //  already allocated object so that we don't risk floating memory
        //  unless the user has specifically disabled it
        if(!(memFlags & MEMFLAGS_OVERRIDE_DOUBLE_ALLOC) && p_Obj)
        {
          return MEMERR_DOUBLE_ALLOC;
        }

        // Attempt to allocate a new object catching any exceptions thrown
        try
        {
          p_Obj = new T();
        }
        catch(const std::bad_alloc &e)
        {
          std::cout << "Error alocating raw memory into the heap: " 
            << e.what() << std::endl;

          return MEMERR_OUT_OF_MEM;
        }
        catch(const std::exception &e)
        {
          return MEMERR_UNKNOWN;
        }
          
        // Check to see if debug messages are disabled... if not then notify the user
        //  we have allocated a new object.
        if(!(memFlags & MEMFLAGS_DISABLE_DEBUG_MSG))
        {
          if(callback)
          {
            callback("Allocated new object in heap");
          }
          else
          {
            std::cout << "Allocated new object in heap" << std::endl;
            ++allocCount;
          }
        }

        return MEMERR_NO_ERR;
      }
    private:
      uint32_t allocCount, deallocCount;
      size_t totalHeapSize;
      bool heapInitalized;
      uint8_t memFlags;
  };

  /*!
   * Creates memory in the heap that deletes automatically once it is out
   *  of scope.
   */
  template<typename T>
  class StaticMem 
  {
    public:
      StaticMem();
      ~StaticMem();

    private:

  };

  /*!
   * Creates memory in the heap that deletes automatically once it is out of
   *  scope but can be referenced by multiple instances requiring all 
   *  instances to dereference to be deleted
   */
  template<typename T>
  class DynamicMem
  {
    public:
      DynamicMem();
      ~DynamicMem();

    private:
  };
}
