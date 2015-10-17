#ifndef MAPMANAGER_H
#define	MAPMANAGER_H

#include "World.h"
#include "MapGeometryGenerator.h"
#include "MapCoordinator.h"
#include "MapLoader.h"
#include <boost/format.hpp>
#include "Timer.h"
#include "Modules/BaseModule.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class MapManager {
    public:
        Vec2f position;
        MapLoader* mapLoader;
        MapGeometryGenerator* mapGeometryGenerator;
        MapCoordinator* mapCoordinator;
        World* world;
        vector<BaseModule*> modules;
        VRObjectPtr root;

        map<string, AreaBoundingBox*> loadedBboxes;

        MapManager(Vec2f position, MapLoader* mapLoader, MapGeometryGenerator* mapGeometryGenerator, MapCoordinator* mapCoordinator, World* world, VRObjectPtr root);

        void addModule(BaseModule* mod);

        void updatePosition(Vec2f worldPosition);

        void physicalize(bool b);

    private:
        void unloadBbox(AreaBoundingBox* bbox);

        void loadBboxIfNecessary(AreaBoundingBox* bbox);

};

}

#endif	/* MAPMANAGER_H */

