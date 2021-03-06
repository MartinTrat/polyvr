#include "E57.h"

#include <iostream>
#include "E57Foundation.h"
#include "core/objects/geometry/VRGeometry.h"
#include "core/objects/geometry/VRGeoData.h"

using namespace e57;
using namespace std;
using namespace OSG;

void OSG::loadE57(string path, VRTransformPtr res) {
    cout << "load e57 " << path << endl;
    res->setName(path);

    try {
        ImageFile imf(path, "r"); // Read file from disk

        StructureNode root = imf.root();
        if (!root.isDefined("/data3D")) { cout << "File doesn't contain 3D images" << endl; return; }

        e57::Node n = root.get("/data3D");
        if (n.type() != E57_VECTOR) { cout << "bad file" << endl; return; }

        VectorNode data3D(n);
        int64_t scanCount = data3D.childCount(); // number of scans in file
        cout << " file read succefully, it contains " << scanCount << " scans" << endl;

        for (int i = 0; i < scanCount; i++) {
            StructureNode scan(data3D.get(i));
            string sname = scan.pathName();

            CompressedVectorNode points( scan.get("points") );
            string pname = points.pathName();
            auto cN = points.childCount();
            cout << "  scan " << i << " contains " << cN << " points\n";

            StructureNode proto(points.prototype());
            bool hasPos = (proto.isDefined("cartesianX") && proto.isDefined("cartesianY") && proto.isDefined("cartesianZ"));
            bool hasCol = (proto.isDefined("colorRed") && proto.isDefined("colorGreen") && proto.isDefined("colorBlue"));
            if (!hasPos) continue;

            vector<SourceDestBuffer> destBuffers;
            const int N = 4;
            double x[N]; destBuffers.push_back(SourceDestBuffer(imf, "cartesianX", x, N, true));
            double y[N]; destBuffers.push_back(SourceDestBuffer(imf, "cartesianY", y, N, true));
            double z[N]; destBuffers.push_back(SourceDestBuffer(imf, "cartesianZ", z, N, true));
            double r[N];
            double g[N];
            double b[N];
            if (hasCol) {
                destBuffers.push_back(SourceDestBuffer(imf, "colorRed", r, N, true));
                destBuffers.push_back(SourceDestBuffer(imf, "colorGreen", g, N, true));
                destBuffers.push_back(SourceDestBuffer(imf, "colorBlue", b, N, true));
            }

            int Nchunk = 1e6; // separate in chunks because of tcmalloc large alloc issues
            VRGeoData data;
            unsigned int gotCount = 0;
            CompressedVectorReader reader = points.reader(destBuffers);
            do {
                if (data.size() > Nchunk) {
                    cout << "  assemble geometry.. " << endl;
                    auto geo = data.asGeometry(pname);
                    res->addChild(geo);
                    data = VRGeoData();
                }

                gotCount = reader.read();
                for (unsigned j=0; j < gotCount; j++) {
                    int v;
                    if (hasCol) v = data.pushVert(Pnt3d(x[j], y[j], z[j]), Vec3d(0,1,0), Color3f(r[j]/255.0, g[j]/255.0, b[j]/255.0));
                    else v = data.pushVert(Pnt3d(x[j], y[j], z[j]), Vec3d(0,1,0));
                    data.pushPoint(v);
                }
            } while(gotCount);
            reader.close();

            if (data.size()) {
                cout << "  assemble geometry.. " << endl;
                auto geo = data.asGeometry(pname);
                res->addChild(geo);
            }
        }

        imf.close();
    }
    catch (E57Exception& ex) { ex.report(__FILE__, __LINE__, __FUNCTION__); return; }
    catch (std::exception& ex) { cerr << "Got an std::exception, what=" << ex.what() << endl; return; }
    catch (...) { cerr << "Got an unknown exception" << endl; return; }
}

//void writeE57(VRGeometryPtr geo, string path);





