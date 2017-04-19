# slqlite3-python
An sqlite3 extension that allows calling of python functions

Based on the embedding code at https://docs.python.org/2/extending/embedding.html

## Compiling and Installing

Below, ```module-path``` refers to a path in your PYTHONPATH at the top of
your sqlite3 extensions. You can leave it off, but then users of sqlite3 will
have access to ***every callable in your project including imports***. This would
be ***very unsafe!***

Also, for safety, ***put all imports inside your function modules*** or sqlite3
users will have access to anything you import!

For example:

Do this:

```
def regexp(regex, value):
  import re
  ... do stuff ...
  return result
```

Do ***NOT*** do this:

```
import re
def regexp(regex, value):
  ... do stuff ...
  return result
```

***YOU HAVE BEEN WARNED!!!***

### Linux

On Linux compile with:

```
gcc -DPYTHONPATH='"<module-path>"' -g -I/usr/include/python2.7 -fPIC -Wall \
    -Werror -shared -lpcre -lsqlite3 -lpython2.7 \
    -olibsqlite3python.so sqlite3python.c
```

and copy libsqlite3python.so to a shared library directory (/usr/lib64).

### Mac

On Mac compile with:

```
gcc -DPYTHONPATH='"<module-path>"' -g -I/usr/include/python2.7 -fPIC -Wall \
    -Werror -shared -lpcre -lsqlite3 -lpython2.7 \
    -olibsqlite3python.dylib sqlite3python.c
```

and copy libsqlite3python.dylib to a shared library directory (/usr/lib).

## Usage

To use:

```
$ env PYTHONPATH=<project path> sqlite3

## Limitations

Currently you can only pass a single string value to Python from sqlite3
and you can only return a single string value back to sqlite3 from Python.

If you need to pass more values or different types, you will have to
encode them in your strings and decode them at the other end.
