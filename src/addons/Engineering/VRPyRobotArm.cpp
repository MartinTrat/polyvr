#include "VRPyRobotArm.h"
#include "core/scripting/VRPyBaseT.h"
#include "core/scripting/VRPyGeometry.h"


template<> PyTypeObject VRPyBaseT<OSG::VRRobotArm>::type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "VR.RobotArm",             /*tp_name*/
    sizeof(VRPyRobotArm),             /*tp_basicsize*/
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
    "RobotArm binding",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VRPyRobotArm::methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)init,      /* tp_init */
    0,                         /* tp_alloc */
    New,                 /* tp_new */
};

PyMethodDef VRPyRobotArm::methods[] = {
    {"setParts", (PyCFunction)VRPyRobotArm::setParts, METH_VARARGS, "Set robot parts - setParts([base, upper_arm, forearm, wrist, grab, jaw1, jaw2])" },
    {"setAngleOffsets", (PyCFunction)VRPyRobotArm::setAngleOffsets, METH_VARARGS, "Set angle offset for each part - setAngleOffsets([float rad])" },
    {"setAxis", (PyCFunction)VRPyRobotArm::setAxis, METH_VARARGS, "Set rotation axis for each part - setAxis([int a])\n a: 0 = 'x', 1 = 'y', 2 = 'z'" },
    {"setLengths", (PyCFunction)VRPyRobotArm::setLengths, METH_VARARGS, "Set kinematic lengths between joints - setLengths([base_height, upper_arm length, forearm length, grab position])" },
    {"moveTo", (PyCFunction)VRPyRobotArm::moveTo, METH_VARARGS, "Move the end effector to a certain position - moveTo([x,y,z])" },
    {"setGrab", (PyCFunction)VRPyRobotArm::setGrab, METH_VARARGS, "Set grab state - setGrab(float d)\n d: 0 is closed, 1 is open" },
    {NULL}  /* Sentinel */
};

PyObject* VRPyRobotArm::moveTo(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::moveTo - Object is invalid"); return NULL; }
    PyObject *v1, *v2;
    if (! PyArg_ParseTuple(args, "OO", &v1, &v2)) return NULL;
    self->obj->moveTo( parseVec3fList(v1), parseVec3fList(v2) );
    Py_RETURN_TRUE;
}

PyObject* VRPyRobotArm::setGrab(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::setGrab - Object is invalid"); return NULL; }
    self->obj->setGrab( parseFloat(args) );
    Py_RETURN_TRUE;
}

PyObject* VRPyRobotArm::setParts(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::setParts - Object is invalid"); return NULL; }
    auto prts = parseList(args);
    vector<OSG::VRTransform*> res;
    for (auto p : prts) res.push_back( ((VRPyTransform*)p)->obj );
    self->obj->setParts( res );
    Py_RETURN_TRUE;
}

PyObject* VRPyRobotArm::setAngleOffsets(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::setAngleOffsets - Object is invalid"); return NULL; }
    auto prts = parseList(args);
    vector<float> res;
    for (auto p : prts) res.push_back( PyFloat_AsDouble(p) );
    self->obj->setAngleOffsets( res );
    Py_RETURN_TRUE;
}

PyObject* VRPyRobotArm::setLengths(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::setLengths - Object is invalid"); return NULL; }
    auto prts = parseList(args);
    vector<float> res;
    for (auto p : prts) res.push_back( PyFloat_AsDouble(p) );
    self->obj->setLengths( res );
    Py_RETURN_TRUE;
}

PyObject* VRPyRobotArm::setAxis(VRPyRobotArm* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyRobotArm::setAxis - Object is invalid"); return NULL; }
    auto prts = parseList(args);
    vector<int> res;
    for (auto p : prts) res.push_back( PyInt_AsLong(p) );
    self->obj->setAxis( res );
    Py_RETURN_TRUE;
}
