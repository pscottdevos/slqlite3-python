/*
 * Written by P. Scott DeVos
 *
 * Based on embedding code at https://docs.python.org/2/extending/embedding.html
 */

#include <assert.h>
#include <Python.h>
#include <sqlite3ext.h>
#include <string.h>

static void call_python(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    PyObject *pName, *pModule, *pDict, *pFunc;
    PyObject *pArgs, *pValue;
    int i;
    
    assert(argc == 3);

    if (argc < 3) {
        fprintf(stderr,"Usage: call pythonfile funcname string_arg\n");
        return 1;
    }

    Py_Initialize();
    pName = PyString_FromString(argv[0]);
    /* Error checking of pName left out */

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, sqlite3_value_text(argv[1]));
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(argc - 3);
            for (i = 2; i < argc; ++i) {
                pValue = PyString_FromString(sqlite3_value_text(argv[i])));
                if (!pValue) {
                    Py_DECREF(pArgs);
                    Py_DECREF(pModule);
                    fprintf(stderr, "Cannot convert argument\n");
                    return 2;
                }
                /* pValue reference stolen here: */
                PyTuple_SetItem(pArgs, i - 2, pValue);
            }
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
                printf("Result of call: %s\n", PyString_Asstring(pValue));
                Py_DECREF(pValue);
            }
            else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
                return 3;
            }
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
        return 4;
    }
    Py_Finalize();
    return 0;
}

int sqlite3_sqlite3python_init(
sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi
) {
  SQLITE_EXTENSION_INIT2(pApi)
  sqlite3_create_function(
    db, "CALL_PYTHON", 3, SQLITE_UTF8, NULL, call_python, NULL, NULL
  );
  return 0;
}
