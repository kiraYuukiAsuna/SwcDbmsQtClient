// Pick your poison.
//
// On GNU/Linux, you have few choices to get the most out of your stack trace.
//
// By default you get:
//	- object filename
//	- function name
//
// In order to add:
//	- source filename
//	- line and column numbers
//	- source code snippet (assuming the file is accessible)

// Install one of the following libraries then uncomment one of the macro (or
// better, add the detection of the lib and the macro definition in your build
// system)

// - apt-get install libdw-dev ...
// - g++/clang++ -ldw ...
// #define BACKWARD_HAS_DW 1

// - apt-get install binutils-dev ...
// - g++/clang++ -lbfd ...
// #define BACKWARD_HAS_BFD 1

// - apt-get install libdwarf-dev ...
// - g++/clang++ -ldwarf ...
// #define BACKWARD_HAS_DWARF 1

// Regardless of the library you choose to read the debug information,
// for potentially more detailed stack traces you can use libunwind
// - apt-get install libunwind-dev
// - g++/clang++ -lunwind
// #define BACKWARD_HAS_LIBUNWIND 1

#include "backward.hpp"

#include "src/framework/core/log/EditorConsoleSink.h"
#include "src/framework/core/log/Log.h"

namespace backward {

// SignalHandling sh;

class MySignalHandling {
public:
  MySignalHandling(const std::vector<int> & = std::vector<int>())
      : reporter_thread_([]() {
          /* We handle crashes in a utility thread:
            backward structures and some Windows functions called here
            need stack space, which we do not have when we encounter a
            stack overflow.
            To support reporting stack traces during a stack overflow,
            we create a utility thread at startup, which waits until a
            crash happens or the program exits normally. */

          {
            std::unique_lock<std::mutex> lk(mtx());
            cv().wait(lk, [] { return crashed() != crash_status::running; });
          }
          if (crashed() == crash_status::crashed) {
            handle_stacktrace(skip_recs());
          }
          {
            std::unique_lock<std::mutex> lk(mtx());
            crashed() = crash_status::ending;
          }
          cv().notify_one();
        }) {
    SetUnhandledExceptionFilter(crash_handler);

    signal(SIGABRT, signal_handler);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    std::set_terminate(&terminator);
#ifndef BACKWARD_ATLEAST_CXX17
    std::set_unexpected(&terminator);
#endif
    _set_purecall_handler(&terminator);
    _set_invalid_parameter_handler(&invalid_parameter_handler);
  }
  bool loaded() const { return true; }

  ~MySignalHandling() {
    {
      std::unique_lock<std::mutex> lk(mtx());
      crashed() = crash_status::normal_exit;
    }

    cv().notify_one();

    reporter_thread_.join();
  }

private:
  static CONTEXT *ctx() {
    static CONTEXT data;
    return &data;
  }

  enum class crash_status { running, crashed, normal_exit, ending };

  static crash_status &crashed() {
    static crash_status data;
    return data;
  }

  static std::mutex &mtx() {
    static std::mutex data;
    return data;
  }

  static std::condition_variable &cv() {
    static std::condition_variable data;
    return data;
  }

  static HANDLE &thread_handle() {
    static HANDLE handle;
    return handle;
  }

  std::thread reporter_thread_;

  // TODO: how not to hardcode these?
  static const constexpr int signal_skip_recs =
#ifdef __clang__
      // With clang, RtlCaptureContext also captures the stack frame of the
      // current function Below that, there are 3 internal Windows functions
      4
#else
      // With MSVC cl, RtlCaptureContext misses the stack frame of the current
      // function The first entries during StackWalk are the 3 internal Windows
      // functions
      3
#endif
      ;

  static int &skip_recs() {
    static int data;
    return data;
  }

  static inline void terminator() {
    crash_handler(signal_skip_recs);
    abort();
  }

  static inline void signal_handler(int) {
    crash_handler(signal_skip_recs);
    abort();
  }

  static inline void __cdecl invalid_parameter_handler(const wchar_t *,
                                                       const wchar_t *,
                                                       const wchar_t *,
                                                       unsigned int,
                                                       uintptr_t) {
    crash_handler(signal_skip_recs);
    abort();
  }

  NOINLINE static LONG WINAPI crash_handler(EXCEPTION_POINTERS *info) {
    // The exception info supplies a trace from exactly where the issue was,
    // no need to skip records
    crash_handler(0, info->ContextRecord);
    return EXCEPTION_CONTINUE_SEARCH;
  }

  NOINLINE static void crash_handler(int skip, CONTEXT *ct = nullptr) {

    if (ct == nullptr) {
      RtlCaptureContext(ctx());
    } else {
      memcpy(ctx(), ct, sizeof(CONTEXT));
    }
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
                    GetCurrentProcess(), &thread_handle(), 0, FALSE,
                    DUPLICATE_SAME_ACCESS);

    skip_recs() = skip;

    {
      std::unique_lock<std::mutex> lk(mtx());
      crashed() = crash_status::crashed;
    }

    cv().notify_one();

    {
      std::unique_lock<std::mutex> lk(mtx());
      cv().wait(lk, [] { return crashed() != crash_status::crashed; });
    }
  }

  static void handle_stacktrace(int skip_frames = 0) {
    // printer creates the TraceResolver, which can supply us a machine type
    // for stack walking. Without this, StackTrace can only guess using some
    // macros.
    // StackTrace also requires that the PDBs are already loaded, which is done
    // in the constructor of TraceResolver
    Printer printer;

    StackTrace st;
    st.set_machine_type(printer.resolver().machine_type());
    st.set_thread_handle(thread_handle());
    st.load_here(32 + skip_frames, ctx());
    st.skip_n_firsts(skip_frames);

    printer.address = true;
    printer.print(st, std::cerr);

    if(Seele::Log::IsInitialized()) {
      std::stringstream ss;
      printer.print(st,ss);
      SEELE_ERROR_TAG("StaackTrace", "{}", ss.str());
      Seele::Log::Flush();
    }

  }
};

  MySignalHandling sh;

} // namespace backward
