/*
 * Written by P. Scott DeVos
 *
 * Based on embedding code at https://docs.python.org/2/extending/embedding.html
 */

#include <Python.h>
#include <sqlite3ext.h>
#include <stdlib.h>
#include <string.h>
SQLITE_EXTENSION_INIT1

static void call_python(
  sqlite3_context *context, int argc, sqlite3_value **argv
) {
    PyObject *pArgs, *pModule, *pFunc, *pValue;
    
    if (argc != 2) {
        sqlite3_result_error(
          context, "Usage: call funcname string_arg\n", -1
        );
        return;
    }
    pModule = sqlite3_user_data(context);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(
            pModule, (char *)(void *)sqlite3_value_text(argv[0])
        );
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(1);
            pValue = PyString_FromString(
              (char *)(void *)sqlite3_value_text(argv[1])
            );
            if (!pValue) {
                Py_DECREF(pArgs);
                sqlite3_result_error(context, "Cannot convert argument\n", -1);
                return;
            }
            /* pValue reference stolen here: */
            PyTuple_SetItem(pArgs, 0, pValue);
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
                const char *cValue = PyString_AsString(pValue);
                sqlite3_result_text(context, cValue, -1, NULL);
                Py_DECREF(pValue);
            } else {
                Py_DECREF(pFunc);
                PyErr_Print();
                sqlite3_result_error(context, "Call Failed\n", -1);
                return;
            }
        } else {
            if (PyErr_Occurred())
                PyErr_Print();
            sqlite3_result_error(context, "Cannot find function\n", -1);
        }
        Py_XDECREF(pFunc);
    }
    else {
        PyErr_Print();
        sqlite3_result_error(context, "Failed to load file\n", -1);
        return;
    }
    return;
}

int sqlite3_sqlitepython_init(
sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi
) {
  SQLITE_EXTENSION_INIT2(pApi)
  Py_Initialize();
  #ifdef EXT_PACKAGE
    const char *cModule = EXT_MODULE;
  #else
    const char *cModule = "sqlite3_extensions";
  #endif
  PyObject *pModuleName = PyString_FromString(cModule);
  PyObject *pModule = PyImport_Import(pModuleName);
  sqlite3_create_function(
    db, "CALL_PYTHON", 2, SQLITE_UTF8, pModule, call_python, NULL, NULL
  );
  return 0;
}
