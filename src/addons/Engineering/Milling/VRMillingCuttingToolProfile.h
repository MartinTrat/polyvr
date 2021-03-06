#ifndef VRMILLINGCUTTINGTOOLPROFILE_H
#define VRMILLINGCUTTINGTOOLPROFILE_H

#include <string>
#include <vector>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGGeoProperties.h>

OSG_BEGIN_NAMESPACE;

using namespace std;

class VRMillingCuttingToolProfile {
    private:
        vector<Vec2d> profile;

    protected:
        bool alreadyInProfile(float newx);
        float newy(float newx, int p);
        int lookForNearestIndex(float newx);
        float lookForMaxInList(vector<Vec2d> liste);

    public:
        VRMillingCuttingToolProfile();
        virtual ~VRMillingCuttingToolProfile();
        static shared_ptr<VRMillingCuttingToolProfile> create();

        void addPointProfile(Vec2d point);
        float maxProfile(Vec3d position, Vec3d cubePosition, Vec3d cubeSize);
        float getLength();
};

OSG_END_NAMESPACE;

#endif // VRMILLINGCUTTINGTOOLPROFILE_H
