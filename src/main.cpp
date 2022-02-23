#include <iostream>
#include <csignal>
#include <csetjmp>
#include <stack>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <syncstream>
#include <iomanip>
#include <malloc.h>
#include <vector>

//#include <valgrind/valgrind.h>

#include "02_system/common/CommonUtilities.hpp"

#include "02_system/common/SystemUtilities.hpp"

#include "02_system/implementation/exception/CallStackFrame.hpp"

#include "02_system/implementation/exception/Exception.hpp"

#include "02_system/implementation/nstd/Vector.hpp"

void stack_trace() {

  NSTD::Vector<NSTD::CallStackFrame> call_stack_entries = SystemUtilities::get_call_stack_entries( 1 );

  for ( const NSTD::CallStackFrame &call_stack_entry: call_stack_entries ) {

    std::cout << call_stack_entry.get_function() << " => " << call_stack_entry.get_file() << ":" << call_stack_entry.get_line() << std::endl;

  }

  malloc_trim( 0 ); //Force to return the memory to Operating system

}

void valid_function() {

  try {

    __BEGIN_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

    std::osyncstream sync_out( std::cout );

    sync_out << "Valid function called" << std::endl;

    __END_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

  }
  catch ( const NSTD::Exception& exception )
  {

    __REPORT_EXCEPTION

  }

}

int invalid_memory_access_01()
{

  int result = 0;

  try
  {

    __BEGIN_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

    valid_function();

    int *badPtr = nullptr;

    result = *badPtr;

    __END_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

  }
  catch ( const NSTD::Exception& exception )
  {

    __REPORT_EXCEPTION

  }

  return result;

}

int invalid_memory_access_02()
{

  int result = 0;

  try
  {

    __BEGIN_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

    valid_function();

    invalid_memory_access_01();

    valid_function();

    int *badPtr = nullptr;

    result = *badPtr;

    invalid_memory_access_01(); //This code never executed, is breaked and send to the catch

    __END_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

  }
  catch ( const NSTD::Exception& exception )
  {

    __REPORT_EXCEPTION

  }

  return result;

}

//FLOATING_POINT_EXCEPTION
int floating_point_exception_01()
{

  int result = 0;

  try
  {

    __BEGIN_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

    int div = 0;

    result = 10 / div;

    __END_MAP_POSIX_SIGNAL_TO_CPP_EXCEPTION

  }
  catch ( const NSTD::Exception& exception )
  {

    __REPORT_EXCEPTION

  }

  return result;

}

void thread01_entry_point( const int repeat, const char *name, const int argc, const char *argv[] )
{

  CommonUtilities::init( name, argc, argv );

  SystemUtilities::init( name, argc, argv );

  for ( int i = 0; i < repeat; i++ )
  {

    std::osyncstream sync_out( std::cout );

    sync_out << "[" << name << "]: Calling to invalid_memory_access_01. Round: " << i << std::endl;

    invalid_memory_access_01();

    sync_out << "[" << name << "]: Sleeping by 1 seconds. Round: " << i << std::endl;

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

  }

  SystemUtilities::finish( name, argc, argv );

  CommonUtilities::finish( name, argc, argv );

}

void thread02_entry_point( const int repeat, const char *name, const int argc, const char *argv[] )
{

  CommonUtilities::init( name, argc, argv );

  SystemUtilities::init( name, argc, argv );

  for ( int i = 0; i < repeat; i++ )
  {

    std::osyncstream sync_out( std::cout );

    sync_out << "[" << name << "]: Calling to invalid_memory_access_02. Round: " << i << std::endl;

    invalid_memory_access_02();

    sync_out << "[" << name << "]: Sleeping by 1 seconds. Round: " << i << std::endl;

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

  }

  SystemUtilities::finish( name, argc, argv );

  CommonUtilities::finish( name, argc, argv );

}

int main( const int argc, const char *argv[] )
{

  CommonUtilities::init( "thread_main", argc, argv );

  SystemUtilities::init( "thread_main", argc, argv );

  std::cout << "Running from the path: " << SystemUtilities::full_program_path << std::endl;

  struct sigaction sa;

  sa.sa_flags = SA_SIGINFO;

  sigemptyset( &sa.sa_mask );

  sa.sa_sigaction = SystemUtilities::catch_signal; //(void (*)(int, siginfo_t *, void *))

  sigaction( SIGILL, &sa, NULL );
  sigaction( SIGFPE, &sa, NULL );
  sigaction( SIGSEGV, &sa, NULL );
  sigaction( SIGINT, &sa, NULL );

  //stack_trace();

  //valid_function();

  invalid_memory_access_01();
  invalid_memory_access_02();

  floating_point_exception_01();

  std::cout << "Press any key to continue the program" << std::endl;
  std::cin.ignore();

  std::thread thread01( thread01_entry_point, 10, "thread_01", argc, argv  );

  std::thread thread02( thread02_entry_point, 10, "thread_02", argc, argv );

  std::thread thread03( thread01_entry_point, 10, "thread_03", argc, argv );

  std::thread thread04( thread02_entry_point, 10, "thread_04", argc, argv );

  int capture = 0;

  //Lambda thread test05
  std::thread thread05( [&capture]( const int repeat, const char *name, const int argc, const char *argv[] )
  {

    CommonUtilities::init( name, argc, argv );

    SystemUtilities::init( name, argc, argv );

    for ( int i = 0; i < repeat; i++ )
    {

      capture += 1;

      std::osyncstream sync_out( std::cout );

      sync_out << "[" << name << "]: Calling to invalid_memory_access_02. Round: " << i << std::endl;

      invalid_memory_access_02();

      sync_out << "[" << name << "]: Sleeping by 1 seconds. Round: " << i << std::endl;

      std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

    }

    CommonUtilities::finish( name, argc, argv );

    SystemUtilities::finish( name, argc, argv );

  },
  10,
  "thread_05", argc, argv );

  thread01.join();

  thread02.join();

  thread03.join();

  thread04.join();

  thread05.join();

  std::cout << "Capture: " << capture << std::endl;

  //malloc_trim( 0 );

  std::cout << "Press any key to finish the program [key]" << std::endl;
  std::cin.ignore();

  CommonUtilities::finish( "thread_main", argc, argv );

  SystemUtilities::finish( "thread_main", argc, argv );

  return 0;

}
