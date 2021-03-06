#ifndef VRPYTEXTUREGENERATOR_H_INCLUDED
#define VRPYTEXTUREGENERATOR_H_INCLUDED

#include "VRPyObject.h"
#include "core/objects/material/VRTextureGenerator.h"

struct VRPyTextureGenerator : VRPyBaseT<OSG::VRTextureGenerator> {
    static PyMethodDef methods[];

    static PyObject* add(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* drawFill(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* drawPixel(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* drawLine(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* drawPath(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* setSize(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* getSize(VRPyTextureGenerator* self);
    static PyObject* compose(VRPyTextureGenerator* self, PyObject* args);
    static PyObject* readSharedMemory(VRPyTextureGenerator* self, PyObject* args);
};

#endif // VRPYTEXTUREGENERATOR_H_INCLUDED
