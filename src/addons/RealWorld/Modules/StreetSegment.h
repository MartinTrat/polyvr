#ifndef STREETSEGMENT_H
#define	STREETSEGMENT_H

#include <OpenSG/OSGVector.h>

OSG_BEGIN_NAMESPACE;
using namespace std;

class StreetSegment {
    public:
        string streetJointA_ID;
        string streetJointB_ID;
        float width;
        int lanes = 0;
        string id;
        string name;
        bool bridge = false;
        bool smallBridge = false;
        bool leftBridge = false;
        bool rightBridge = false;

        Vec2f leftA, leftB, rightA, rightB, leftExtA, leftExtB;

        StreetSegment(string streetJointA_ID, string streetJointB_ID, float width, string id);

        void setLeftPointFor(string jointId, Vec2f posLeft);
        void setLeftExtPointFor(string jointId, Vec2f posLeft);
        void setRightPointFor(string jointId, Vec2f posRight);

        string getOtherJointId(string jointId);
        float getDistance();
};

OSG_END_NAMESPACE;

#endif	/* STREETSEGMENT_H */

