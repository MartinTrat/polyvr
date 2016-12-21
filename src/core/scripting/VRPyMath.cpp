#include "VRPyMath.h"
#include "VRPyBaseT.h"
#include "core/utils/toString.h"

using namespace OSG;

template<> PyTypeObject VRPyBaseT<Vec3f>::type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "VR.Math.Vec3",             /*tp_name*/
    sizeof(VRPyVec3f),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    VRPyVec3f::Print,                         /*tp_repr*/
    &VRPyVec3f::nMethods,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
    "Vec3 binding",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VRPyVec3f::methods,             /* tp_methods */
    0,                      /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)init,      /* tp_init */
    0,                         /* tp_alloc */
    VRPyVec3f::New,                 /* tp_new */
};

PyNumberMethods VRPyVec3f::nMethods = {
    (binaryfunc)VRPyVec3f::add,
    (binaryfunc)VRPyVec3f::sub,
    (binaryfunc)VRPyVec3f::mul,
    (binaryfunc)VRPyVec3f::div,
    0,               /* binaryfunc nb_remainder;    __mod__ */
    0,            /* binaryfunc nb_divmod;       __divmod__ */
    0,               /* ternaryfunc nb_power;       __pow__ */
    (unaryfunc)VRPyVec3f::neg,
    0,               /* unaryfunc nb_positive;      __pos__ */
    (unaryfunc)VRPyVec3f::abs,
    0,           /* inquiry nb_nonzero;         __nonzero__ */
    0,            /* unaryfunc nb_invert;        __invert__ */
    0,            /* binaryfunc nb_lshift;       __lshift__ */
    0,            /* binaryfunc nb_rshift;       __rshift__ */
    0,               /* binaryfunc nb_and;          __and__ */
    0,               /* binaryfunc nb_xor;          __xor__ */
    0,                /* binaryfunc nb_or;           __or__ */
    0,            /* coercion nb_coerce;         __coerce__ */
    0,               /* unaryfunc nb_int;           __int__ */
    0,              /* unaryfunc nb_long;          __long__ */
    0,             /* unaryfunc nb_float;         __float__ */
    0,               /* unaryfunc nb_oct;           __oct__ */
    0,               /* unaryfunc nb_hex;           __hex__ */
};

PyMethodDef VRPyVec3f::methods[] = {
    {"normalize", (PyCFunction)VRPyVec3f::normalize, METH_NOARGS, "Normalize the vector - normalize()" },
    {"length", (PyCFunction)VRPyVec3f::length, METH_NOARGS, "Compute the length - float length()" },
    {"dot", (PyCFunction)VRPyVec3f::dot, METH_VARARGS, "Compute the dot product - float dot( Vec3f )" },
    {"cross", (PyCFunction)VRPyVec3f::cross, METH_VARARGS, "Compute the cross product - Vec3f cross( Vec3f )" },
    {NULL}  /* Sentinel */
};

VRPyVec3f* toPyVec3f(const Vec3f& v) {
    VRPyVec3f* pv = (VRPyVec3f*)VRPyVec3f::typeRef->tp_alloc(VRPyVec3f::typeRef, 0);
    pv->owner = false;
    pv->v = v;
    return pv;
}

PyObject* toPyObject(const Vec3f& v) { return (PyObject*)toPyVec3f(v); }

PyObject* VRPyVec3f::New(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    float a,b,c;
    if (! PyArg_ParseTuple(args, "fff", &a, &b, &c)) return NULL;
    VRPyVec3f* pv = (VRPyVec3f*)alloc( type, 0 );
    pv->v = Vec3f(a,b,c);
    return (PyObject*)pv;
}

PyObject* VRPyVec3f::Print(PyObject* self) {
    string s = "[" + toString(((VRPyVec3f*)self)->v) + "]";
    return PyString_FromString( s.c_str() );
}

PyObject* VRPyVec3f::normalize(VRPyVec3f* self) {
    self->v.normalize();
    Py_RETURN_TRUE;
}

PyObject* VRPyVec3f::length(VRPyVec3f* self) {
    return PyFloat_FromDouble( self->v.length() );
}

PyObject* VRPyVec3f::dot(VRPyVec3f* self, PyObject* args) {
    VRPyVec3f* v;
    if (!PyArg_ParseTuple(args, "O", &v)) return NULL;
    return PyFloat_FromDouble( self->v.dot(v->v) );
}

PyObject* VRPyVec3f::cross(VRPyVec3f* self, PyObject* args) {
    VRPyVec3f* v;
    if (!PyArg_ParseTuple(args, "O", &v)) return NULL;
    return ::toPyObject( self->v.cross(v->v) );
}


PyObject* VRPyVec3f::add(PyObject* self, PyObject* v) {
    return ::toPyObject( ((VRPyVec3f*)self)->v + ((VRPyVec3f*)v)->v );
}

PyObject* VRPyVec3f::sub(PyObject* self, PyObject* v) {
    return ::toPyObject( ((VRPyVec3f*)self)->v - ((VRPyVec3f*)v)->v );
}

PyObject* VRPyVec3f::mul(PyObject* self, PyObject* F) {
    float f = PyFloat_AsDouble(F);
    return ::toPyObject( ((VRPyVec3f*)self)->v * f );
}

PyObject* VRPyVec3f::div(PyObject* self, PyObject* F) {
    float f = PyFloat_AsDouble(F);
    return ::toPyObject( ((VRPyVec3f*)self)->v * (1.0/f) );
}

PyObject* VRPyVec3f::neg(PyObject* self) {
    return ::toPyObject( -((VRPyVec3f*)self)->v);
}

PyObject* VRPyVec3f::abs(PyObject* self) {
    Vec3f v = ((VRPyVec3f*)self)->v;
    return ::toPyObject( Vec3f(::abs(v[0]), ::abs(v[1]), ::abs(v[2])) );
}