#ifndef VRCAMERAMANAGER_H_INCLUDED
#define VRCAMERAMANAGER_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include <string>
#include <vector>
#include "core/scene/VRSceneFwd.h"
#include "core/objects/VRObjectFwd.h"

OSG_BEGIN_NAMESPACE;
using namespace std;


class VRCameraManager {
    private:
        VRCameraWeakPtr active;
        VRSpaceWarperPtr spaceWarper;

    public:
        VRCameraManager();
        ~VRCameraManager();

        void addCamera(VRCameraPtr cam);
        void setMActiveCamera(string cam);
        VRCameraPtr getCamera(int ID);
        VRCameraPtr getActiveCamera();
        int getActiveCameraIndex();

        vector<string> getCameraNames();

        void setupSpaceWarper(bool b);
        VRSpaceWarperPtr getSpaceWarper();
};

OSG_END_NAMESPACE;

#endif // VRCAMERAMANAGER_H_INCLUDED
