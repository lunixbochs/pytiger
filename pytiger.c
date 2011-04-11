/*
Copyright 2006 Mark Lee

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include <Python.h>
#include <string.h>
#include <stdint.h>
#include "tigertree.h"

word64 endianFlip(word64 val)
{
   return 
   ((val &   UINT64_C(0x00000000000000FF)) << 56) |
   ((val &   UINT64_C(0x000000000000FF00)) << 40) |
   ((val &   UINT64_C(0x0000000000FF0000)) << 24) |
   ((val &   UINT64_C(0x00000000FF000000)) << 8)  |
   ((val &   UINT64_C(0x000000FF00000000)) >> 8)  | 
   ((val &   UINT64_C(0x0000FF0000000000)) >> 24) |
   ((val &   UINT64_C(0x00FF000000000000)) >> 40) |
   ((val &   UINT64_C(0xFF00000000000000)) >> 56);
}

static PyObject *tiger_hash(PyObject*, PyObject*);
static PyObject *tiger_treehash(PyObject*, PyObject*);
static PyMethodDef tiger_methods[3] = {
    {"hash", tiger_hash, METH_VARARGS, "Creates a tiger hash from the specified string."},
    {"treehash", tiger_treehash, METH_VARARGS | METH_KEYWORDS, "Creates a tiger tree hash from the specified file object."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC inittiger(void)
{
    (void) Py_InitModule("tiger", tiger_methods);
}

static PyObject *tiger_hash(PyObject *self, PyObject *args)
{
    const char *str;
    word64 result[3];
    if (!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    tiger((word64*)str, strlen(str), result);

    int i;
    for (i=0; i<3; i++) {
        result[i] = endianFlip(result[i]);
    }

    char buffer[49];
    sprintf(buffer, "%.16llx%.16llx%.16llx", result[0], result[1], result[2]);
    return Py_BuildValue("s#", buffer, 48);
}

static PyObject *tiger_treehash(PyObject *self, PyObject *args)
{
    PyObject *fileobj;
    FILE* f;
    TT_CONTEXT tt_ctx;
    unsigned char result[TIGERSIZE];
    const char *buffer;
    int buffer_len;
    if (!PyArg_ParseTuple(args, "s#", &buffer, &buffer_len)) {
        if (PyArg_ParseTuple(args, "O", &fileobj)) {
            if (!PyFile_Check(fileobj)) {
                PyErr_SetString(PyExc_TypeError, "First argument MUST be a string or a file object!");
                return NULL;
            }
            f = PyFile_AsFile(fileobj);
            (void)fseek(f, 0L, SEEK_END);
            buffer_len = ftell(f);
            rewind(f);
            buffer = (char*)calloc(buffer_len, sizeof(char));
            fread((char*)buffer, sizeof(char), buffer_len, f);
            /* Don't fclose(f) or else it will segfault - Python does it for us */
        } else {
            return NULL;
        }
    }
    tt_init(&tt_ctx);
    tt_update(&tt_ctx, (unsigned char*)buffer, buffer_len);
    tt_digest(&tt_ctx, result);

    /* For some reason the last character is always 0x01, which is not part of the hash */
    return Py_BuildValue("s#", (char*)result, 24);
}
