#include "VRPyWorldGenerator.h"
#include "addons/WorldGenerator/nature/VRPyTree.h"
#include "core/scripting/VRPyBaseT.h"
#include "core/scripting/VRPyPath.h"
#include "core/scripting/VRPyGeometry.h"
#include "core/scripting/VRPyPolygon.h"
#include "addons/Semantics/Reasoning/VRPyOntology.h"

using namespace OSG;

simpleVRPyType(Asphalt, New_ptr);
simpleVRPyType(RoadNetwork, New_ptr);


PyMethodDef VRPyAsphalt::methods[] = {
    {"addMarking", (PyCFunction)VRPyAsphalt::addMarking, METH_VARARGS, "Add marking - addMarking( rID, path, width | dashN, offset )" },
    {"addTrack", (PyCFunction)VRPyAsphalt::addTrack, METH_VARARGS, "Add track - addTrack( rID, path, width | dashN, offset )" },
    {"clearTexture", (PyCFunction)VRPyAsphalt::clearTexture, METH_NOARGS, "Clear internal textures - clearTexture()" },
    {"updateTexture", (PyCFunction)VRPyAsphalt::updateTexture, METH_NOARGS, "Update internal textures - updateTexture()" },
    {NULL}  /* Sentinel */
};

PyObject* VRPyAsphalt::addMarking(VRPyAsphalt* self, PyObject *args) {
    if (!self->valid()) return NULL;
    int rID = 0;
    int dashN = 0;
    float width = 0.1;
    float offset = 0;
    VRPyPath* path = 0;
    if (!PyArg_ParseTuple(args, "iOf|if", &rID, &path, &width, &dashN, &offset)) return NULL;
    self->objPtr->addMarking( rID, path->objPtr, width, dashN, offset );
    Py_RETURN_TRUE;
}

PyObject* VRPyAsphalt::addTrack(VRPyAsphalt* self, PyObject *args) {
    if (!self->valid()) return NULL;
    int rID = 0;
    int dashN = 0;
    float width = 0.1;
    float offset = 0;
    VRPyPath* path = 0;
    if (!PyArg_ParseTuple(args, "iOf|if", &rID, &path, &width, &dashN, &offset)) return NULL;
    self->objPtr->addTrack( rID, path->objPtr, width, dashN, offset );
    Py_RETURN_TRUE;
}

PyObject* VRPyAsphalt::clearTexture(VRPyAsphalt* self) {
    if (!self->valid()) return NULL;
    self->objPtr->clearTexture();
    Py_RETURN_TRUE;
}

PyObject* VRPyAsphalt::updateTexture(VRPyAsphalt* self) {
    if (!self->valid()) return NULL;
    self->objPtr->updateTexture();
    Py_RETURN_TRUE;
}


// ------------------------------------------------------

PyMethodDef VRPyRoadNetwork::methods[] = {
    {"setOntology", (PyCFunction)VRPyRoadNetwork::setOntology, METH_VARARGS, "Set the ontology - setOntology( ontology )" },
    {"setNatureManager", (PyCFunction)VRPyRoadNetwork::setNatureManager, METH_VARARGS, "Set the nature manager - setNatureManager( woods )" },
    {"addNode", (PyCFunction)VRPyRoadNetwork::addNode, METH_VARARGS, "Add a new node - node addNode( [x,y,z] )" },
    {"addLane", (PyCFunction)VRPyRoadNetwork::addLane, METH_VARARGS, "Add a new lane - lane addLane( int direction, road, float width )" },
    {"addGreenBelt", (PyCFunction)VRPyRoadNetwork::addGreenBelt, METH_VARARGS, "Add a green lane - lane addGreenBelt( road, float width )" },
    {"addWay", (PyCFunction)VRPyRoadNetwork::addWay, METH_VARARGS, "Add a new way - way addWay( str name, [paths], int rID, str type )" },
    {"addRoad", (PyCFunction)VRPyRoadNetwork::addRoad, METH_VARARGS, "Add a new road - road addRoad( str name, str type, node1, node2, norm1, norm2, lanesN )" },
    {"addPath", (PyCFunction)VRPyRoadNetwork::addPath, METH_VARARGS, "Add a new path - path addPath( str type, str name, [nodes], [normals] )" },
    {"addArrows", (PyCFunction)VRPyRoadNetwork::addArrows, METH_VARARGS, "Add a new path - arrows addArrows( lane, float t, [float] dirs )" },
    {"addAsset", PyWrap(RoadNetwork, addAsset, "Add an asset", void, string, VRTransformPtr) },
    {"computeIntersectionLanes", (PyCFunction)VRPyRoadNetwork::computeIntersectionLanes, METH_VARARGS, "Compute the lanes of an intersection - computeIntersectionLanes( intersection )" },
    {"computeLanePaths", (PyCFunction)VRPyRoadNetwork::computeLanePaths, METH_VARARGS, "Compute the path of each lane of a road - computeLanePaths( road )" },
    {"computeIntersections", (PyCFunction)VRPyRoadNetwork::computeIntersections, METH_NOARGS, "Compute the intersections - computeIntersections( )" },
    {"computeLanes", (PyCFunction)VRPyRoadNetwork::computeLanes, METH_NOARGS, "Compute the lanes - computeLanes( )" },
    {"computeSurfaces", (PyCFunction)VRPyRoadNetwork::computeSurfaces, METH_NOARGS, "Compute the surfaces - computeSurfaces( )" },
    {"computeMarkings", (PyCFunction)VRPyRoadNetwork::computeMarkings, METH_NOARGS, "Compute the markings - computeMarkings( )" },
    {"compute", (PyCFunction)VRPyRoadNetwork::compute, METH_NOARGS, "Compute everything - compute( )" },
    {"computeMarkingsRoad2", (PyCFunction)VRPyRoadNetwork::computeMarkingsRoad2, METH_VARARGS, "Compute markings of a type 2 road - computeMarkingsRoad2( road )" },
    {"computeMarkingsIntersection", (PyCFunction)VRPyRoadNetwork::computeMarkingsIntersection, METH_VARARGS, "Compute markings of an intersection - computeMarkingsIntersection( intersection )" },
    {"createRoadGeometry", (PyCFunction)VRPyRoadNetwork::createRoadGeometry, METH_VARARGS, "Create a geometry for a road - geo createRoadGeometry( road )" },
    {"createIntersectionGeometry", (PyCFunction)VRPyRoadNetwork::createIntersectionGeometry, METH_VARARGS, "Create a geometry for an intersection - geo createIntersectionGeometry( intersection )" },
    {"getRoadID", (PyCFunction)VRPyRoadNetwork::getRoadID, METH_NOARGS, "Get a road ID - int getRoadID()" },
    {"getMaterial", (PyCFunction)VRPyRoadNetwork::getMaterial, METH_NOARGS, "Get road material - material getMaterial()" },
    {"updateAsphaltTexture", (PyCFunction)VRPyRoadNetwork::updateAsphaltTexture, METH_NOARGS, "Update markings and tracks on asphalt texture - updateAsphaltTexture()" },
    {"computeGreenBelts", (PyCFunction)VRPyRoadNetwork::computeGreenBelts, METH_NOARGS, "Compute green belt areas - areas computeGreenBelts()" },
    {"clear", (PyCFunction)VRPyRoadNetwork::clear, METH_NOARGS, "Clear all data - clear()" },
    {NULL}  /* Sentinel */
};

PyObject* VRPyRoadNetwork::computeGreenBelts(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    auto areas = self->objPtr->computeGreenBelts();
    PyObject* pyAreas = PyList_New(0);
    for (auto a : areas) PyList_Append( pyAreas, VRPyPolygon::fromSharedPtr(a) );
    return pyAreas;
}

PyObject* VRPyRoadNetwork::updateAsphaltTexture(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->updateAsphaltTexture();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::getMaterial(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    return VRPyAsphalt::fromSharedPtr( self->objPtr->getMaterial() );
}

PyObject* VRPyRoadNetwork::getRoadID(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    return PyInt_FromLong( self->objPtr->getRoadID() );
}

PyObject* VRPyRoadNetwork::clear(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->clear();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeMarkingsRoad2(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* road = 0;
    if (!PyArg_ParseTuple(args, "O", &road)) return NULL;
    self->objPtr->computeMarkingsRoad2( road->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeMarkingsIntersection(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* intersection = 0;
    if (!PyArg_ParseTuple(args, "O", &intersection)) return NULL;
    self->objPtr->computeMarkingsIntersection( intersection->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::createRoadGeometry(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* road = 0;
    if (!PyArg_ParseTuple(args, "O", &road)) return NULL;
    return VRPyGeometry::fromSharedPtr( self->objPtr->createRoadGeometry( road->objPtr ) );
}

PyObject* VRPyRoadNetwork::createIntersectionGeometry(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* intersection = 0;
    if (!PyArg_ParseTuple(args, "O", &intersection)) return NULL;
    return VRPyGeometry::fromSharedPtr( self->objPtr->createIntersectionGeometry( intersection->objPtr ) );
}

PyObject* VRPyRoadNetwork::computeIntersections(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->computeIntersections();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeLanes(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->computeLanes();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeSurfaces(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->computeSurfaces();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeMarkings(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->computeMarkings();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::compute(VRPyRoadNetwork* self) {
    if (!self->valid()) return NULL;
    self->objPtr->compute();
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeLanePaths(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* road = 0;
    if (!PyArg_ParseTuple(args, "O", &road)) return NULL;
    self->objPtr->computeLanePaths( road->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::computeIntersectionLanes(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* intersection = 0;
    if (!PyArg_ParseTuple(args, "O", &intersection)) return NULL;
    self->objPtr->computeIntersectionLanes( intersection->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::setOntology(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyOntology* ontology = 0;
    if (!PyArg_ParseTuple(args, "O", &ontology)) return NULL;
    self->objPtr->setOntology( ontology->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::setNatureManager(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyWoods* woods = 0;
    if (!PyArg_ParseTuple(args, "O", &woods)) return NULL;
    self->objPtr->setNatureManager( woods->objPtr );
    Py_RETURN_TRUE;
}

PyObject* VRPyRoadNetwork::addNode(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    auto n = self->objPtr->addNode( parseVec3f( args ) );
    return VRPyEntity::fromSharedPtr( n );
}

PyObject* VRPyRoadNetwork::addLane(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    int dir = 0;
    VRPyEntity* road = 0;
    float width = 0;
    if (!PyArg_ParseTuple(args, "iOf", &dir, &road, &width)) return NULL;
    auto l = self->objPtr->addLane( dir, road->objPtr, width );
    return VRPyEntity::fromSharedPtr( l );
}

PyObject* VRPyRoadNetwork::addGreenBelt(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* road = 0;
    float width = 0;
    if (!PyArg_ParseTuple(args, "Of", &road, &width)) return NULL;
    auto l = self->objPtr->addGreenBelt( road->objPtr, width );
    return VRPyEntity::fromSharedPtr( l );
}

PyObject* VRPyRoadNetwork::addWay(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    const char* name;
    PyObject* paths = 0;
    int rID = 0;
    const char* type;
    if (!PyArg_ParseTuple(args, "sOis", &name, &paths, &rID, &type)) return NULL;
    vector<VREntityPtr> pathsV;
    for (int i=0; i<pySize(paths); i++) pathsV.push_back( ((VRPyEntity*)PyList_GetItem(paths, i))->objPtr );
    auto r = self->objPtr->addWay( name?name:"", pathsV, rID, type?type:"" );
    return VRPyEntity::fromSharedPtr( r );
}

PyObject* VRPyRoadNetwork::addRoad(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    const char* name;
    const char* type;
    VRPyEntity* node1 = 0;
    VRPyEntity* node2 = 0;
    PyObject* norm1 = 0;
    PyObject* norm2 = 0;
    int Nlanes = 1;
    if (!PyArg_ParseTuple(args, "ssOOOOi", &name, &type, &node1, &node2, &norm1, &norm2, &Nlanes)) return NULL;
    auto road = self->objPtr->addRoad( name?name:"", type?type:"", node1->objPtr, node2->objPtr, parseVec3fList(norm1), parseVec3fList(norm2), Nlanes );
    return VRPyEntity::fromSharedPtr( road );
}

PyObject* VRPyRoadNetwork::addPath(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    const char* type;
    const char* name;
    PyObject* nodes = 0;
    PyObject* norms = 0;
    if (!PyArg_ParseTuple(args, "ssOO", &type, &name, &nodes, &norms)) return NULL;
    vector<VREntityPtr> nodesV;
    vector<Vec3f> normsV;
    for (int i=0; i<pySize(nodes); i++) nodesV.push_back( ((VRPyEntity*)PyList_GetItem(nodes, i))->objPtr );
    for (int i=0; i<pySize(norms); i++) normsV.push_back( parseVec3fList(PyList_GetItem(norms, i)) );
    auto p = self->objPtr->addPath( type?type:"", name?name:"", nodesV, normsV );
    return VRPyEntity::fromSharedPtr( p );
}

PyObject* VRPyRoadNetwork::addArrows(VRPyRoadNetwork* self, PyObject *args) {
    if (!self->valid()) return NULL;
    VRPyEntity* lane = 0;
    float t = 0;
    PyObject* dirs = 0;
    if (!PyArg_ParseTuple(args, "OfO", &lane, &t, &dirs)) return NULL;
    vector<float> Dirs;
    for (int i=0; i<pySize(dirs); i++) Dirs.push_back( PyFloat_AsDouble(PyList_GetItem(dirs, i)) );
    auto a = self->objPtr->addArrows( lane->objPtr, t, Dirs );
    return VRPyEntity::fromSharedPtr( a );
}



