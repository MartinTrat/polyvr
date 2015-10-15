#ifndef VIRTUOSE_H_INCLUDED
#define VIRTUOSE_H_INCLUDED
#include <virtuose/virtuoseAPI.h>

#include <OpenSG/OSGVector.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGGLUT.h>


#include <list>
#include "core/objects/VRTransform.h"
#include "core/objects/geometry/VRPhysics.h"


OSG_BEGIN_NAMESPACE;
using namespace std;

class virtuose {
    private:
        void* vc = 0; // virtuose context
        bool isAttached = false;
        /** attached Object*/
        VRTransformPtr attached = 0;
        /** represents the haptic's center in the virtual scene **/
        VRTransformPtr base = 0;

        /** haptic timestep **/
        float timestep;
        /** user generated force **/
        Vec3f totalForce;

        /** diff between frames (used to avoid build-ups)**/
        Vec3f pPos;
        Vec3f sPos;
        Vec3f sRot;


        VirtCommandType commandType  = COMMAND_TYPE_NONE;
        float gripperPosition;
        float gripperSpeed;



    public:
        virtuose();
        ~virtuose();

        bool connected();
        void connect(string IP,float timestep = 0.002f);
        void disconnect();

        void setSimulationScales(float translation, float forces);


        Matrix getPose();

        /**returns force generated by the user**/
        Vec3f getForce();
        /**
        Applies given Force/Torque on the haptic
        **/
        void applyForce(Vec3f force, Vec3f torque);
        /** parses position/rotation data of given VRPhysics into the given float[7] array**/
        void fillPosition(VRPhysics* p, float *to, VRPhysics* origin);
        /** parses speed data of given VRPhysics into the given float[6] array**/
        void fillSpeed(VRPhysics* p, float *to,VRPhysics* origin);
        /** parses given btMatrix3x3 into the given float[9] array**/
        void Matrix3ToArray(btMatrix3x3 m, float *to);
        /** sets the base **/
        void setBase(VRTransformPtr tBase);
        /**connect a physicalized Object to this virtuose and push it in the same direction the virtuose moves.**/
        void attachTransform(VRTransformPtr trans);
        /** detach the previously attached Transform and returns it**/
        VRTransformPtr detachTransform();
        /** update functions of the Virtuose, have to be called each frame**/
        void updateVirtMechPre();
        void updateVirtMechPost();
        /** 1 means button pressed, 0 means released**/
        Vec3i getButtonStates();

        void enableForceFeedback(bool enable);
};

OSG_END_NAMESPACE;

#endif // VIRTUOSE_H_INCLUDED
