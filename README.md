# slqlite3-python
An sqlite3 extension that allows calling of python functions

Based on the embedding code at
https://docs.python.org/2/extending/embedding.html

## Build and Install

Below, ```module-name``` refers to a module in your PYTHONPATH at the top of
your sqlite3 extensions. If you leave it off, the default value of
"sqlite3_extensions" will be used.

If you need to break your module into a package, provide the name of the
package for ```module-name``` and import all of your functions into
```<module-name>/__init__.py```.

Do ***NOT*** put any imports in the top level of your module
(or ```__init__.py``` when using a package) or those imports will be
callable from sqlite3 which would be ***very unsafe!!!***

Either import your names explicitly into your module (or ```__init__.py```)
or ***place all imports inside your function defitions***. Failure to heed
this advice will give sqlite3 users access to anything you import!

Example:

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

Do this:

from my_package.my_module import a, b, c

Do ***NOT*** do this:

from my_package.my_module import *

***YOU HAVE BEEN WARNED!!!***

### Linux

On Linux compile with:

```
gcc -DEXT_MODULE='"<module-path>"' -g -I/usr/include/python2.7 -fPIC -Wall \
    -Werror -shared -lpcre -lsqlite3 -lpython2.7 \
    -olibsqlite3python.so sqlite3python.c
```

and copy libsqlite3python.so to a shared library directory (/usr/lib64).

### Mac

On Mac compile with:

```
gcc -DEXT_MODULE='"<module-path>"' -g -I/usr/include/python2.7 -fPIC -Wall \
    -Werror -shared -lpcre -lsqlite3 -lpython2.7 \
    -olibsqlite3python.dylib sqlite3python.c
```

and copy libsqlite3python.dylib to a shared library directory (/usr/lib).

## Usage

To use:

```
$ env PYTHONPATH=<project path> sqlite3
SQLite version 3.16.2 2017-01-06 16:32:41
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
sqlite> select load_extension('libsqlite3python');

sqlite> select call_python('<function_name>, 'value');
```

Example using the sample functions:

$ env PYTHONPATH=. sqlite3
SQLite version 3.16.2 2017-01-06 16:32:41
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
sqlite> select load_extension('libsqlite3python');

sqlite> select call_python('string_length', 'abc');
The string is abc
3
sqlite> select call_python('double_string', 'abc');
The string is abc
abcabc


## Limitations

Currently you can only pass a single string value to Python from sqlite3
and you can only return a single string value back to sqlite3 from Python.

If you need to pass more values or different types, you will have to
encode them in your strings and decode them at the other end.
