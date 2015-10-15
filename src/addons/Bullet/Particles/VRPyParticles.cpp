#include "VRPyParticles.h"
#include "core/scripting/VRPyGeometry.h"
#include "core/scripting/VRPyBaseT.h"

template<> PyTypeObject VRPyBaseT<OSG::VRParticles>::type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "VR.Particles",             /*tp_name*/
    sizeof(VRPyParticles),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "VRParticles binding",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VRPyParticles::methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)init,      /* tp_init */
    0,                         /* tp_alloc */
    New_VRObjects_unnamed_ptr,                 /* tp_new */
};

PyMethodDef VRPyParticles::methods[] = {
    {"getGeometry", (PyCFunction)VRPyParticles::getGeometry, METH_VARARGS, "Get geometry - Geometry getGeometry()" },
    {"spawnCuboid", (PyCFunction)VRPyParticles::spawnCuboid, METH_VARARGS, "spawnCuboid(x,y,z) \n\tspawnCuboid(x,y,z,'size',a,b,c) \n\tspawnCuboid(x,y,z,'liter',l)"},

    {"setRadius", (PyCFunction)VRPyParticles::setRadius, METH_VARARGS, "setRadius(radius, variation) \n\tsetRadius(0.05, 0.02)"},
    {"setMass", (PyCFunction)VRPyParticles::setMass, METH_VARARGS, "setMass(mass, variation) \n\tsetMass(0.05, 0.02)"},
    {"setMassByRadius", (PyCFunction)VRPyParticles::setMassByRadius, METH_VARARGS, "setMassByRadius(massOfOneMeterRadius) \n\tsetMass(1000.0*100)"},
    {"setMassForOneLiter", (PyCFunction)VRPyParticles::setMassForOneLiter, METH_VARARGS, "setMassForOneLiter(massOfOneLiter) \n\tsetMass(1000.0)"},
    {NULL}  /* Sentinel */
};

void checkObj(VRPyParticles* self) {
    if (self->objPtr == 0) self->objPtr = OSG::VRParticles::create();
}

PyObject* VRPyParticles::getGeometry(VRPyParticles* self) {
    Py_RETURN_TRUE;
}

PyObject* VRPyParticles::spawnCuboid(VRPyParticles* self, PyObject* args) {
    checkObj(self);
    OSG::Vec3f position;
    float x,y,z, a=1,b=1,c=1;
    char* mode = NULL; int modeLength=0;

    if (! PyArg_ParseTuple(args, "fff|s#fff", &x,&y,&z, &mode, &modeLength, &a,&b,&c)) {
        // ERROR!
        Py_RETURN_FALSE;
    }
    position.setValues(x, y, z);
    int num = 0;
    if (mode != NULL && strncmp(mode, "size", modeLength)==0) {
        num = self->objPtr->spawnCuboid(position, OSG::VRParticles::SIZE, a,b,c);
    } else if (mode != NULL && strncmp(mode, "liter", modeLength)==0) {
        num = self->objPtr->spawnCuboid(position, OSG::VRParticles::LITER, a);
    } else {
        num = self->objPtr->spawnCuboid(position);
    }

    // Py_RETURN_TRUE;
    return PyInt_FromLong((long) num);
}

PyObject* VRPyParticles::setRadius(VRPyParticles* self, PyObject* args) {
    checkObj(self);
    float radius, variation;
    radius = variation = 0.0;
    if (! PyArg_ParseTuple(args, "f|f", &radius, &variation)) { Py_RETURN_FALSE; }
    self->objPtr->setRadius(radius, variation);
    Py_RETURN_TRUE;
}

PyObject* VRPyParticles::setMass(VRPyParticles* self, PyObject* args) {
    checkObj(self);
    float mass, variation;
    mass = variation = 0.0;
    if (! PyArg_ParseTuple(args, "f|f", &mass, &variation)) { Py_RETURN_FALSE; }
    self->objPtr->setMass(mass, variation);
    Py_RETURN_TRUE;
}

PyObject* VRPyParticles::setMassByRadius(VRPyParticles* self, PyObject* args) {
    checkObj(self);
    float massFor1mRadius = 0.0;
    if (! PyArg_ParseTuple(args, "f", &massFor1mRadius)) { Py_RETURN_FALSE; }
    self->objPtr->setMassByRadius(massFor1mRadius);
    Py_RETURN_TRUE;
}

PyObject* VRPyParticles::setMassForOneLiter(VRPyParticles* self, PyObject* args) {
    checkObj(self);
    float massPerLiter = 0.0;
    if (! PyArg_ParseTuple(args, "f", &massPerLiter)) { Py_RETURN_FALSE; }
    self->objPtr->setMassForOneLiter(massPerLiter);
    Py_RETURN_TRUE;
}
