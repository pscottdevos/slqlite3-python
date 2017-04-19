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
    PyObject *pArgs, *pFormat, *pName, *pPath, *pModule, *pFunc;
    PyObject *pValue;
    
    if (argc != 3) {
        sqlite3_result_error(
          context, "Usage: call pythonfile funcname string_arg\n", -1
        );
        return;
    }

    pName = PyString_FromString((char *)(void *)sqlite3_value_text(argv[0]));
    #ifdef PYTHONPATH
      pPath = PyString_FromString(PYTHONPATH);
      pFormat = PyString_FromString("%s.%s");
      pArgs = PyTuple_New(2);
      PyTuple_SetItem(pArgs, 0, pPath);
      PyTuple_SetItem(pArgs, 1, pName);
      pName = PyString_Format(PyString_FromString("%s.%s"), pArgs);
      Py_DECREF(pPath);
      Py_DECREF(pFormat);
      Py_DECREF(pArgs);
    #endif

    /* Error checking of pName left out */

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(
            pModule, (char *)(void *)sqlite3_value_text(argv[1])
        );
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(1);
            pValue = PyString_FromString(
              (char *)(void *)sqlite3_value_text(argv[2])
            );
            if (!pValue) {
                Py_DECREF(pArgs);
                Py_DECREF(pModule);
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
                Py_DECREF(pModule);
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
        Py_DECREF(pModule);
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
  sqlite3_create_function(
    db, "CALL_PYTHON", 3, SQLITE_UTF8, NULL, call_python, NULL, NULL
  );
  Py_Initialize();
  return 0;
}
