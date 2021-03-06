#ifndef VRGEOMETRY_H_INCLUDED
#define VRGEOMETRY_H_INCLUDED



#include "core/objects/VRObjectFwd.h"
#include "core/tools/selection/VRSelectionFwd.h"
#include "../VRTransform.h"

#include <OpenSG/OSGSField.h>

struct VRPrimitive;

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRMaterial;
class GeoVectorProperty;
class GeoIntegralProperty;
class Action;

class VRGeometry : public VRTransform {
    public:
        enum { CODE, SCRIPT, FILE, PRIMITIVE };

        struct Reference {
            int type;
            string parameter;
            Reference(int type = CODE, string params = "");
        };

    protected:
        VRMaterialPtr mat = 0;
        VRPrimitive* primitive = 0;
        OSGGeometryPtr mesh;
        OSGObjectPtr mesh_node;
        bool meshSet = false;
        int lastMeshChange = 0;

        map<string, VRGeometryPtr> dataLayer;

        Reference source;

        VRObjectPtr copy(vector<VRObjectPtr> children);

        void meshChanged();
        void setup();

        VRGeometry(string name, bool hidden);
        static VRGeometryPtr create(string name, bool hidden);

    public:
        VRGeometry(string name = "0");
        virtual ~VRGeometry();

        static VRGeometryPtr create(string name = "None");
        static VRGeometryPtr create(string name, string primitive, string params);
        VRGeometryPtr ptr();

        /** Set the geometry mesh (OSG geometry core) **/
        void setMesh(OSGGeometryPtr g = 0);
        void setMesh(OSGGeometryPtr g, Reference ref, bool keep_material = false);

        void setReference(Reference ref);
        Reference getReference();
        void makeUnique();
        void setMeshVisibility(bool b);

        virtual bool applyIntersectionAction(Action* ia);
        virtual void setPrimitive(string primitive, string args = "");

        /** Create a mesh using vectors with positions, normals, indices && optionaly texture coordinates **/
        void create(int type, vector<Vec3d> pos, vector<Vec3d> norms, vector<int> inds, vector<Vec2d> texs = vector<Vec2d>());
        void create(int type, GeoVectorProperty* pos, GeoVectorProperty* norms, GeoIntegralProperty* inds, GeoVectorProperty* texs);

        /** Overwrites the vertex positions of the mesh **/
        void setType(int t);
        void setTypes(GeoIntegralProperty* types);
        void setPositions(GeoVectorProperty* Pos);
        void setNormals(GeoVectorProperty* Norms);
        void setColors(GeoVectorProperty* Colors, bool fixMapping = false);
        void setIndices(GeoIntegralProperty* Indices, bool doLengths = false);
        void setTexCoords(GeoVectorProperty* Tex, int i=0, bool fixMapping = false);
        void setLengths(GeoIntegralProperty* lenghts);
        void setPatchVertices(int n);
        void setPositionalTexCoords(float scale = 1.0, int i = 0, Vec3i format = Vec3i(0,1,2));
        void setPositionalTexCoords2D(float scale = 1.0, int i = 0, Vec2i format = Vec2i(0,1));

        void setRandomColors();
        void removeDoubles(float minAngle);
        void decimate(float f);
        void merge(VRGeometryPtr geo);
        void removeSelection(VRSelectionPtr sel);
        VRGeometryPtr copySelection(VRSelectionPtr sel);
        VRGeometryPtr separateSelection(VRSelectionPtr sel);
        void fixColorMapping();
        void updateNormals(bool face = false);
        void flipNormals();

        int getLastMeshChange();

        void genTexCoords(string mapping = "CUBE", float scale = 1, int channel = 0, PosePtr p = 0);

        void showGeometricData(string type, bool b);
        float calcSurfaceArea();

        int size();
        Vec3d getGeometricCenter();
        Vec3d getAverageNormal();
        float getMax(int axis);
        float getMin(int axis);

        /** Returns the mesh as a OSG geometry core **/
        OSGGeometryPtr getMesh();
        VRPrimitive* getPrimitive();

        /** Set the material of the mesh **/
        void setColor(string c);
        void setMaterial(VRMaterialPtr mat = 0);
        VRMaterialPtr getMaterial();

        void influence(vector<Vec3d> pnts, vector<Vec3d> values, int power, float color_code = -1, float dl_max = 1.0);

        void readSharedMemory(string segment, string object);
};

OSG_END_NAMESPACE;

#endif // VRGEOMETRY_H_INCLUDED
