//
//  main.m
//  A main module for starting Python projects under iOS.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Python/Python.h>
#include <dlfcn.h>

int main(int argc, char *argv[]) {
    int ret = 0;
    unsigned int i;
    NSString *tmp_path;
    NSString *exe;
    NSString *python_home;
    wchar_t *wpython_home;
    const char* main_script;
    wchar_t** python_argv;
    @autoreleasepool {

        NSString * resourcePath = [[NSBundle mainBundle] resourcePath];

        // Special environment to avoid writing bytecode because
        // the process will not have write attribute on the device.
        putenv("PYTHONDONTWRITEBYTECODE=1");

        python_home = [NSString stringWithFormat:@"%@/Library/Python.framework/Resources", resourcePath, nil];
        NSLog(@"PythonHome is: %@", python_home);
        wpython_home = Py_DecodeLocale([python_home UTF8String], NULL);
        Py_SetPythonHome(wpython_home);

        // iOS provides a specific directory for temp files.
        tmp_path = [NSString stringWithFormat:@"TMP=%@/tmp", resourcePath, nil];
        putenv((char *)[tmp_path UTF8String]);

        // Since iOS doesn't allow dynamic linking, we have to know
        // the name of the executable so that we can find the ctypes
        // test objects. However, sys.argv[0] will be updated to
        // reflect the script name; the TEST_EXECUTABLE environment
        // variable provides the mechanism for specifying the filename.
        exe = [NSString stringWithFormat:@"TEST_EXECUTABLE=%s", argv[0], nil];
        putenv((char *)[exe UTF8String]);

        NSLog(@"Initializing Python runtime...");
        Py_Initialize();

        /*******************************************************
         To tell lldb not to stop on signals, use the following commands:
           process handle SIGPIPE -n true -p true -s false
           process handle SIGINT -n true -p true -s false
           process handle SIGXFSZ -n true -p true -s false
           process handle SIGUSR1 -n true -p true -s false
           process handle SIGUSR2 -n true -p true -s false
         *******************************************************/

        // Arguments to pass to test runner
        char *test_args[] = {
            "-j", "1",
            "-u", "all,-audio,-curses,-largefile,-subprocess,-gui",
//            "-v",                  // Verbose test output
            "-W",                  // Display test output on failure
            
            "-x",                  // Arguments are tests to *exclude*
//            Simulator failures
//            "test_coroutines",       // docstring not being populated
//            "test_module",           // docstring not being populated

//            ARM64 failures
//            "test_coroutines",     // docstring not being populated
//            "test_ctypes",         // DL loading?
//            "test_module"          // docstring not being populated
//            "test_threading",      // ctypes related; missing symbol PyThreadState_SetAsyncExc
//            "test_unicode",        // encoding problem

//            ARMv7 failures
//            "test_cmath",          // math domain error
//            "test_ctypes",         // DL loading?
//            "test_float",          // rounding?
//            "test_math",           // math domain error
//            "test_numeric_tower",  //
//            "test_strtod",         //
//            "test_importlib",      // Thread locking problem
//            "test_threading",      // ctypes related; missing symbol PyThreadState_SetAsyncExc

//            COMMON FAILURES
            "test_bytes"           // HARD CRASH ctypes related; PyBytes_FromFormat

        };

        // Set the name of the main script
        main_script = [
            [[NSBundle mainBundle] pathForResource:@"Library/Application Support/org.python.iOS-test/app/iOS-test/main"
                                            ofType:@"py"] cStringUsingEncoding:NSUTF8StringEncoding];

        if (main_script == NULL) {
            NSLog(@"Unable to locate app/iOS-test/main.py file");
            exit(-1);
        }

        // Construct argv for the interpreter
        int n_test_args = sizeof(test_args) / sizeof (*test_args) + 1;

        python_argv = PyMem_RawMalloc(sizeof(wchar_t*) * n_test_args);
        python_argv[0] = Py_DecodeLocale(main_script, NULL);
        for (i = 1; i < n_test_args; i++) {
            python_argv[i] = Py_DecodeLocale(test_args[i-1], NULL);
        }

        PySys_SetArgv(n_test_args, python_argv);

        // If other modules are using thread, we need to initialize them before.
        PyEval_InitThreads();

        // Start the main.py script
        NSLog(@"Running %s", main_script);

        @try {
            FILE* fd = fopen(main_script, "r");
            if (fd == NULL) {
                ret = 1;
                NSLog(@"Unable to open main.py, abort.");
            } else {
                ret = PyRun_SimpleFileEx(fd, main_script, 1);
                if (ret != 0) {
                    NSLog(@"Application quit abnormally!");
                }
            }
        }
        @catch (NSException *exception) {
            NSLog(@"Python runtime error: %@", [exception reason]);
        }
        @finally {
            Py_Finalize();
        }

        PyMem_RawFree(wpython_home);
        if (python_argv) {
            for (i = 0; i < argc; i++) {
                PyMem_RawFree(python_argv[i]);
            }
            PyMem_RawFree(python_argv);
        }
        NSLog(@"Leaving");
    }

    exit(ret);
    return ret;
}