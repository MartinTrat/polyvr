#include "VRTerrain.h"
#include "VRTerrainPhysicsShape.h"
#include "VRPlanet.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/material/VRMaterialT.h"
#include "core/objects/material/VRTexture.h"
#include "core/objects/material/VRTextureGenerator.h"
#include "core/objects/geometry/VRPhysics.h"
#include "core/objects/geometry/VRGeoData.h"
#include "core/objects/geometry/OSGGeometry.h"
#include "core/utils/VRFunction.h"
#include "core/math/boundingbox.h"
#include "core/math/polygon.h"
#include "core/math/triangulator.h"
#include "core/math/pose.h"
#include "core/math/path.h"
#include "core/scene/import/GIS/VRGDAL.h"
#include "addons/WorldGenerator/GIS/OSMMap.h"

#include <OpenSG/OSGIntersectAction.h>
#include <OpenSG/OSGGeoProperties.h>
#include <OpenSG/OSGGeometry.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#define GLSL(shader) #shader

using namespace OSG;


VREmbankment::VREmbankment(pathPtr p1, pathPtr p2, pathPtr p3, pathPtr p4) : VRGeometry("embankment"), p1(p1), p2(p2), p3(p3), p4(p4) {
    for (auto p : p1->getPoints()) { auto pos = p.pos(); area.addPoint(Vec2d(pos[0],pos[2])); };
    for (auto p : p2->getPoints()) { auto pos = p.pos(); area.addPoint(Vec2d(pos[0],pos[2])); };
}

VREmbankment::~VREmbankment() {}
VREmbankmentPtr VREmbankment::create(pathPtr p1, pathPtr p2, pathPtr p3, pathPtr p4) { return VREmbankmentPtr( new VREmbankment(p1,p2,p3,p4) ); }

bool VREmbankment::isInside(Vec2d p) { return area.isInside(p); }

float VREmbankment::getHeight(Vec2d p) { // TODO: optimize!
    auto computeHeight = [&](float h) {
        Vec3d P(p[0], h, p[1]);
        float t1 = p1->getClosestPoint(P);
        float t2 = p2->getClosestPoint(P);
        Vec3d P1 = p1->getPosition(t1);
        Vec3d P2 = p2->getPosition(t2);
        float d1 = (P1-P).length();
        float d2 = (P2-P).length();
        float t = d2/(d1+d2);
        return P1[1]*t+P2[1]*(1-t);
    };

    float h1 = computeHeight(0); // first estimate
    float h2 = computeHeight(h1); // first estimate
    float h3 = computeHeight(h2); // first estimate
    return h3;
}

vector<Vec3d> VREmbankment::probeHeight(Vec2d p) { // TODO: optimize!
    vector<Vec3d> res(3);

    auto computeHeight = [&](float h) {
        Vec3d P(p[0], h, p[1]);
        float t1 = p1->getClosestPoint(P);
        float t2 = p2->getClosestPoint(P);
        Vec3d P1 = p1->getPosition(t1);
        Vec3d P2 = p2->getPosition(t2);
        float d1 = (P1-P).length();
        float d2 = (P2-P).length();
        float t = d2/(d1+d2);
        float H = P1[1]*t+P2[1]*(1-t);

        res[0] = Vec3d(p[0], H, p[1]);
        res[1] = P1;
        res[2] = P2;
        return H;
    };

    float h1 = computeHeight(0); // first estimate
    float h2 = computeHeight(h1); // first estimate
    computeHeight(h2); // first estimate
    getMaterial()->setWireFrame(1);
    return res;
}

void VREmbankment::createGeometry() {
    float res = 0.025;
    int N = round(1.0/res);
    VRGeoData data;
    Vec3d u = Vec3d(0,1,0);
    for (int i=0; i<=N; i++) {
        auto pos1 = p1->getPosition(i*res);
        auto pos2 = p2->getPosition(1-i*res);
        auto pos3 = p3->getPosition(i*res);
        auto pos4 = p4->getPosition(1-i*res);
        data.pushVert(pos1, u, Vec2d(pos1[0], pos1[2]));
        data.pushVert(pos2, u, Vec2d(pos2[0], pos2[2]));
        data.pushVert(pos3, u, Vec2d(pos3[0], pos3[2]));
        data.pushVert(pos4, u, Vec2d(pos4[0], pos4[2]));
        if (i < N) {
            data.pushQuad(i*4, i*4+1, (i+1)*4+1, (i+1)*4); // top
            data.pushQuad(i*4, (i+1)*4, (i+1)*4+2, i*4+2); // side1
            data.pushQuad(i*4+1, i*4+3, (i+1)*4+3, (i+1)*4+1); // side2
        }
    }

    data.apply(ptr());
    updateNormals();
}


VRTerrain::VRTerrain(string name) : VRGeometry(name) {}
VRTerrain::~VRTerrain() {}

VRTerrainPtr VRTerrain::create(string name) {
    auto t = VRTerrainPtr( new VRTerrain(name) );
    t->setupMat();
    return t;
}

VRTerrainPtr VRTerrain::ptr() { return dynamic_pointer_cast<VRTerrain>( shared_from_this() ); }

void VRTerrain::clear() {
    for (auto e : embankments) e.second->destroy();
    embankments.clear();
}

void VRTerrain::setParameters( Vec2d s, double r, double h ) {
    size = s;
    resolution = r;
    heightScale = h;
    grid = r*64;
    mat->setShaderParameter("resolution", resolution);
    mat->setShaderParameter("heightScale", heightScale);
    mat->setShaderParameter("doHeightTextures", 0);
    updateTexelSize();
    setupGeo();
}

void VRTerrain::setMap( VRTexturePtr t, int channel ) {
    if (!t) return;
    if (t->getChannels() != 4) { // fix mono channels
        VRTextureGenerator tg;
        auto dim = t->getSize();
        tg.setSize(dim, true);
        tex = tg.compose(0);
        for (int i = 0; i < dim[0]; i++) {
            for (int j = 0; j < dim[1]; j++) {
                double h = t->getPixel(Vec3i(i,j,0))[0];
                tex->setPixel(Vec3i(i,j,0), Color4f(1.0,1.0,1.0,h));
            }
        }
    } else tex = t;
    mat->setTexture(tex);
	mat->setShaderParameter("channel", channel);
    mat->setTextureParams(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MODULATE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    updateTexelSize();
    setupGeo();
}

void VRTerrain::updateTexelSize() {
    if (!tex) return;
    Vec3i s = tex->getSize();
    texelSize[0] = size[0]/(s[0]-1);
    texelSize[1] = size[1]/(s[1]-1);
	mat->setShaderParameter("texelSize", texelSize);
}

void VRTerrain::setupGeo() {
    Vec2i gridN = Vec2i(round(size[0]*1.0/grid-0.5), round(size[1]*1.0/grid-0.5));
    if (gridN[0] < 1) gridN[0] = 1;
    if (gridN[1] < 1) gridN[1] = 1;
    Vec2d gridS = size;
    gridS[0] /= gridN[0];
    gridS[1] /= gridN[1];

    auto texSize = tex->getSize();
    Vec2d texel = Vec2d( 1.0/texSize[0], 1.0/texSize[1] );
	Vec2d tcChunk = Vec2d((1.0-texel[0])/gridN[0], (1.0-texel[1])/gridN[1]);

	VRGeoData geo;
    for (int i =0; i < gridN[0]; i++) {
		double px1 = -size[0]*0.5 + i*gridS[0];
		double px2 = px1 + gridS[0];
		double tcx1 = texel[0]*0.5 + i*tcChunk[0];
		double tcx2 = tcx1 + tcChunk[0];

		for (int j =0; j < gridN[1]; j++) {
			double py1 = -size[1]*0.5 + j*gridS[1];
			double py2 = py1 + gridS[1];
			double tcy1 = texel[1]*0.5 + j*tcChunk[1];
			double tcy2 = tcy1 + tcChunk[1];

			geo.pushVert(Vec3d(px1,0,py1), Vec3d(0,1,0), Vec2d(tcx1,tcy1));
			geo.pushVert(Vec3d(px1,0,py2), Vec3d(0,1,0), Vec2d(tcx1,tcy2));
			geo.pushVert(Vec3d(px2,0,py2), Vec3d(0,1,0), Vec2d(tcx2,tcy2));
			geo.pushVert(Vec3d(px2,0,py1), Vec3d(0,1,0), Vec2d(tcx2,tcy1));
			geo.pushQuad();
		}
	}

	geo.apply(ptr());
	setType(GL_PATCHES);
	setPatchVertices(4);
	setMaterial(mat);
}

Vec2d VRTerrain::toUVSpace(Vec2d p) {
    int W = tex->getSize()[0]-1;
    int H = tex->getSize()[1]-1;
    double u = (p[0]/size[0] + 0.5)*W;
    double v = (p[1]/size[1] + 0.5)*H;
    return Vec2d(u,v);
};

Vec2d VRTerrain::fromUVSpace(Vec2d uv) {
    int W = tex->getSize()[0]-1;
    int H = tex->getSize()[1]-1;
    double x = ((uv[0])/W-0.5)*size[0];
    double z = ((uv[1])/H-0.5)*size[1];
    return Vec2d(x,z);
};

vector<Vec3d> VRTerrain::probeHeight( Vec2d p ) {
    Vec2d uv = toUVSpace(p); // uv, i and j are tested
    int i = round(uv[0]-0.5);
    int j = round(uv[1]-0.5);

    double h00 = tex->getPixel(Vec3i(i,j,0))[3];
    double h10 = tex->getPixel(Vec3i(i+1,j,0))[3];
    double h01 = tex->getPixel(Vec3i(i,j+1,0))[3];
    double h11 = tex->getPixel(Vec3i(i+1,j+1,0))[3];

    double u = uv[0]-i;
    double v = uv[1]-j;
    double h = ( h00*(1-u) + h10*u )*(1-v) + ( h01*(1-u) + h11*u )*v;

    Vec2d p0 = fromUVSpace( Vec2d(i,j) );
    Vec2d p1 = fromUVSpace( Vec2d(i+1,j+1) );
    //cout << " VRTerrain::getHeight " << uv << " " << i << " " << j << " " << W << " " << H << endl;
    for (auto e : embankments) if (e.second->isInside(p)) return e.second->probeHeight(p);

    return {Vec3d(p[0], h, p[1]),
            Vec3d(p0[0], h00, p0[1]),
            Vec3d(p1[0], h10, p0[1]),
            Vec3d(p0[0], h01, p1[1]),
            Vec3d(p1[0], h11, p1[1]) };
}

VRTexturePtr VRTerrain::getMap() { return tex; }
Vec2f VRTerrain::getTexelSize() { return texelSize; }

void VRTerrain::btPhysicalize() {
    auto dim = tex->getSize();
    float roadTerrainOffset = 0.03; // also defined in vrroadbase.cpp

    double Hmax = -1e6;
    physicsHeightBuffer = shared_ptr<vector<float>>( new vector<float>(dim[0]*dim[1]) );
    for (int i = 0; i < dim[0]; i++) {
        for (int j = 0; j < dim[1]; j++) {
            int k = j*dim[0]+i;
            float h = tex->getPixel(Vec3i(i,j,0))[3];
            (*physicsHeightBuffer)[k] = h + roadTerrainOffset;
            if (Hmax < h) Hmax = h;
        }
    }

    auto shape = new btHeightfieldTerrainShape(dim[0], dim[1], &(*physicsHeightBuffer)[0], 1, -Hmax, Hmax, 1, PHY_FLOAT, false);
    shape->setLocalScaling(btVector3(texelSize[0],1,texelSize[1]));
    getPhysics()->setCustomShape( shape );
}

void VRTerrain::vrPhysicalize() {
    auto shape = new VRTerrainPhysicsShape( ptr(), resolution );
    getPhysics()->setCustomShape( shape );
}

void VRTerrain::physicalize(bool b) {
    if (!tex) return;
    if (!b) { getPhysics()->setPhysicalized(false); return; }

    //btPhysicalize();
    vrPhysicalize();
    getPhysics()->setPhysicalized(true);
}

Boundingbox VRTerrain::getBoundingBox() {
    Boundingbox bb;
    float hmax = -1e30;
    float hmin = 1e30;

    for (int i=0; i<tex->getSize()[0]; i++) {
        for (int j=0; j<tex->getSize()[1]; j++) {
            auto h = tex->getPixel(Vec3i(i,j,0))[3];
            if (h < hmin) hmin = h;
            if (h > hmax) hmax = h;
        }
    }

    bb.update( Vec3d( size[0]*0.5, hmax,  size[1]*0.5) );
    bb.update( Vec3d(-size[0]*0.5, hmin, -size[1]*0.5) );
    return bb;
}

void VRTerrain::setSimpleNoise() {
    Color4f w(1,1,1,1);
    VRTextureGenerator tg;
    tg.setSize(Vec3i(128,128,1),true);
    tg.add("Perlin", 1.0, w*0.97, w);
    tg.add("Perlin", 1.0/2, w*0.95, w);
    tg.add("Perlin", 1.0/4, w*0.85, w);
    tg.add("Perlin", 1.0/8, w*0.8, w);
    tg.add("Perlin", 1.0/16, w*0.7, w);
    tg.add("Perlin", 1.0/32, w*0.5, w);
    tex = tg.compose(0);
	auto defaultMat = VRMaterial::get("defaultTerrain");
    defaultMat->setTexture(tex);
}

void VRTerrain::setupMat() {
	auto defaultMat = VRMaterial::get("defaultTerrain");
	tex = defaultMat->getTexture();
	if (!tex) {
        Color4f w(0,0,0,0);
        VRTextureGenerator tg;
        tg.setSize(Vec3i(128,128,1),true);
        tg.drawFill(w);
        tex = tg.compose(0);
        defaultMat->setTexture(tex);
	}

	mat = VRMaterial::create("terrain");
	mat->setVertexShader(vertexShader, "terrainVS");
	mat->setFragmentShader(fragmentShader, "terrainFS");
	mat->setTessControlShader(tessControlShader, "terrainTCS");
	mat->setTessEvaluationShader(tessEvaluationShader, "terrainTES");
	mat->setShaderParameter("resolution", resolution);
	mat->setShaderParameter("channel", 3);
	mat->setShaderParameter("texelSize", texelSize);
    mat->setZOffset(1,1);
	setMap(tex);
}

bool VRTerrain::applyIntersectionAction(Action* action) {
    if (!mesh || !mesh->geo) return false;

    auto tex = getMaterial()->getTexture();

	auto distToSurface = [&](Pnt3d p) -> double {
		float h = getHeight(Vec2d(p[0], p[2]));
		return p[1] - h;
	};

	if (!VRGeometry::applyIntersectionAction(action)) return false;

    IntersectAction* ia = dynamic_cast<IntersectAction*>(action);

    auto ia_line = ia->getLine();
    Pnt3d p0 = Pnt3d(ia_line.getPosition());
    Vec3d d = Vec3d(ia_line.getDirection());
    Pnt3d p = Pnt3d(ia->getHitPoint());

    Vec3f norm(0,1,0); // TODO
    int N = 1000;
    double step = 10; // TODO
    int dir = 1;
    for (int i = 0; i < N; i++) {
        p = p - d*step*dir; // walk
        double l = distToSurface(p);
        if (l > 0 && l < 0.03) {
            Real32 t = p0.dist( p );
            ia->setHit(t, ia->getActNode(), 0, norm, -1);
            break;
        } else if (l*dir > 0) { // jump over surface
            dir *= -1;
            step *= 0.5;
        }
    }

    return true;
}

double VRTerrain::getHeight(const Vec2d& p, bool useEmbankments) {
    int W = tex->getSize()[0]-1;
    int H = tex->getSize()[1]-1;

    auto toUVSpace = [&](Vec2d p) {
        double u = (p[0]/size[0] + 0.5)*W;
        double v = (p[1]/size[1] + 0.5)*H;
        return Vec2d(u,v);
    };

    /*auto fromUVSpace = [&](Vec2d uv) { // keep for debugging
        double x = ((uv[0])/W-0.5)*size[0];
        double z = ((uv[1])/H-0.5)*size[1];
        return Vec2d(x,z);
    };*/

    Vec2d uv = toUVSpace(p); // uv, i and j are tested
    int i = round(uv[0]-0.5);
    int j = round(uv[1]-0.5);

    double h00 = tex->getPixel(Vec3i(i,j,0))[3];
    double h10 = tex->getPixel(Vec3i(i+1,j,0))[3];
    double h01 = tex->getPixel(Vec3i(i,j+1,0))[3];
    double h11 = tex->getPixel(Vec3i(i+1,j+1,0))[3];

    double u = uv[0]-i;
    double v = uv[1]-j;
    double h = ( h00*(1-u) + h10*u )*(1-v) + ( h01*(1-u) + h11*u )*v;
    if (useEmbankments) {
        for (auto e : embankments) {
            if (e.second->isInside(p)) {
                double k = e.second->getHeight(p);
                if (k > h) h = k;
            }
        }
    }
    return h;
}

void VRTerrain::elevateObject(VRTransformPtr t, float offset) { auto p = t->getFrom(); elevatePoint(p, offset); t->setFrom(p); }
void VRTerrain::elevatePose(PosePtr p, float offset) { auto P = p->pos(); elevatePoint(P, offset); p->setPos(P); }
void VRTerrain::elevatePoint(Vec3d& p, float offset, bool useEmbankments) { p[1] = getHeight(Vec2d(p[0], p[2]), useEmbankments) + offset; }

void VRTerrain::elevateVertices(VRGeometryPtr geo, float offset) {
    if (!terrain) return;
    GeoPnt3fPropertyRecPtr pos = (GeoPnt3fProperty*)geo->getMesh()->geo->getPositions();
    for (uint i=0; i<pos->size(); i++) {
        Pnt3f p;
        pos->getValue(p, i);
        p[1] = getHeight(Vec2d(p[0], p[2])) + offset;
        pos->setValue(p, i);
    }
}

void VRTerrain::elevatePolygon(VRPolygonPtr poly, float offset, bool useEmbankments) {
    for (auto p2 : poly->get()) {
        Vec3d p3(p2[0], 0, p2[1]);
        elevatePoint(p3, offset, useEmbankments);
        poly->addPoint(p3);
    }
    poly->get().clear();
}

void VRTerrain::projectTangent( Vec3d& t, Vec3d p) {
    t[1] = 0;
    t.normalize(); // TODO: to optimize!
    float h1 = getHeight(Vec2d(p[0]-t[0]*0.5, p[2]-t[2]*0.5));
    float h2 = getHeight(Vec2d(p[0]+t[0]*0.5, p[2]+t[2]*0.5));
    t[1] = h2-h1;
    t.normalize();
}

void VRTerrain::loadMap( string path, int channel ) {
    cout << "   ----------- VRTerrain::loadMap " << path << " " << channel << endl ;
    auto tex = loadGeoRasterData(path);
    setMap(tex, channel);
}

void VRTerrain::projectOSM() {
    /*if (!tex) return;

    // -------------- prepare polygons in terrain space
    Matrix4d terrainMatrix = dynamic_pointer_cast<VRTransform>( getParent() )->getMatrix();
    terrainMatrix.invert();
    map< string, vector<VRPolygonPtr> > polygons;
    auto map = world->get;
    for (auto way : map->getWays()) {
        auto p = way.second->polygon;
        auto pp = VRPolygon::create();

        for (auto pnt : p.get()) {
            Pnt3d pos = Pnt3d( planet->fromLatLongPosition(pnt[1], pnt[0]) );
            terrainMatrix.mult(pos, pos);
            pp->addPoint( Vec2d(pos[0], pos[2]) );
        }

        for (auto tag : way.second->tags) polygons[tag.first].push_back(pp);
    }*/

    // training ground hack flat ground
    /*auto tgPolygon = VRPolygon::create();
    Pnt3d pos;
    float d = 0.003;
    pos = Pnt3d( planet->fromLatLongPosition(29.924500-d, 119.896806-d) );
    terrainMatrix.mult(pos, pos); tgPolygon->addPoint( Vec2d(pos[0], pos[2]) );
    pos = Pnt3d( planet->fromLatLongPosition(29.924500-d, 119.896806+d) );
    terrainMatrix.mult(pos, pos); tgPolygon->addPoint( Vec2d(pos[0], pos[2]) );
    pos = Pnt3d( planet->fromLatLongPosition(29.924500+d, 119.896806+d) );
    terrainMatrix.mult(pos, pos); tgPolygon->addPoint( Vec2d(pos[0], pos[2]) );
    pos = Pnt3d( planet->fromLatLongPosition(29.924500+d, 119.896806-d) );
    terrainMatrix.mult(pos, pos); tgPolygon->addPoint( Vec2d(pos[0], pos[2]) );
    tgPolygon->scale( Vec3d(1.0/size[0], 1, 1.0/size[1]) );
    tgPolygon->translate( Vec3d(0.5,0,0.5) );*/

    // -------------------- project OSM polygons on texture
    /*auto dim = tex->getSize();
    VRTextureGenerator tg;
    tg.setSize(dim, true);

    //for (auto tag : polygons) cout << "polygon tag: " << tag.first << endl;

    auto drawPolygons = [&](string tag, Color4f col) {
        if (!polygons.count(tag)) {
            //cout << "\ndrawPolygons: tag '" << tag << "' not found!" << endl;
            return;
        }

        for (auto p : polygons[tag]) {
            p->scale( Vec3d(1.0/size[0], 1, 1.0/size[1]) );
            p->translate( Vec3d(0.5,0,0.5) );
            tg.drawPolygon( p, col );
        }
    };

    drawPolygons("natural", Color4f(0,1,0,1));
    drawPolygons("water", Color4f(0.2,0.4,1,1));
    drawPolygons("industrial", Color4f(0.2,0.2,0.2,1));
    VRTexturePtr t = tg.compose(0);

    // ----------------------- combine OSM texture with heightmap
    for (int i = 0; i < dim[0]; i++) {
        for (int j = 0; j < dim[1]; j++) {
            Vec3i pixK = Vec3i(i,j,0);
            double h = tex->getPixel(pixK)[3];
            auto pix = Vec2d(i*1.0/(dim[0]-1), j*1.0/(dim[1]-1));
            //if (tgPolygon->isInside(pix)) h = 14;
            Color4f col = t->getPixel(pixK);
            col[3] = h;
            t->setPixel(pixK, col);
        }
    }
    setMap(t);*/
}

void VRTerrain::paintHeights(string path) {
    mat->setTexture(path, 0, 1);
    mat->setTexture("world/textures/gravel2.jpg", 0, 2);
    mat->setShaderParameter("texWoods", 1);
    mat->setShaderParameter("texGravel", 2);
    mat->setShaderParameter("doHeightTextures", 1);
}

void VRTerrain::addEmbankment(string ID, pathPtr p1, pathPtr p2, pathPtr p3, pathPtr p4) {
    auto e = VREmbankment::create(p1, p2, p3, p4);
    auto m = VRMaterial::get("embankment");
    m->setTexture("world/textures/gravel2.jpg");
    m->setDiffuse(Color3f(0.5,0.5,0.5));
    m->setZOffset(1,1);
    e->createGeometry();
    e->setMaterial(m);
    addChild(e);
    embankments[ID] = e;
}

// --------------------------------- shader ------------------------------------

string VRTerrain::vertexShader =
"#version 120\n"
GLSL(
attribute vec4 osg_Vertex;
attribute vec2 osg_MultiTexCoord0;

void main(void) {
    gl_TexCoord[0] = vec4(osg_MultiTexCoord0,0.0,0.0);
	gl_Position = osg_Vertex;
}
);

string VRTerrain::fragmentShader =
"#version 400 compatibility\n"
GLSL(
uniform sampler2D tex;
uniform sampler2D texWoods;
uniform sampler2D texGravel;
const ivec3 off = ivec3(-1,0,1);
const vec3 light = vec3(-1,-1,-0.5);
uniform vec2 texelSize;
uniform int doHeightTextures;

in vec4 pos;
in float height;
vec3 norm;
vec4 color;

vec3 mixColor(vec3 c1, vec3 c2, float t) {
	t = clamp(t, 0.0, 1.0);
	return mix(c1, c2, t);
}

vec3 getColor() {
	return texture2D(tex, gl_TexCoord[0].xy).rgb;
}

void applyBlinnPhong() {
	norm = normalize( gl_NormalMatrix * norm );
	vec3  light = normalize( gl_LightSource[0].position.xyz );// directional light
	float NdotL = max(dot( norm, light ), 0.0);
	vec4  ambient = gl_LightSource[0].ambient * color;
	vec4  diffuse = gl_LightSource[0].diffuse * NdotL * color;
	float NdotHV = max(dot( norm, normalize(gl_LightSource[0].halfVector.xyz)),0.0);
	vec4  specular = 0.25*gl_LightSource[0].specular * pow( NdotHV, gl_FrontMaterial.shininess );
	//gl_FragColor = ambient + diffuse + specular;
    color = mix(diffuse + specular, vec4(0.7,0.9,1,1), clamp(1e-4*length(pos.xyz), 0.0, 1.0)); // atmospheric effects
	gl_FragColor = color;
	gl_FragColor[3] = 1.0;
	//gl_FragColor = vec4(diffuse.rgb, 1);
}

vec3 getNormal() {
	vec2 tc = gl_TexCoord[0].xy;
    float s11 = texture(tex, tc).a;
    float s01 = textureOffset(tex, tc, off.xy).a;
    float s21 = textureOffset(tex, tc, off.zy).a;
    float s10 = textureOffset(tex, tc, off.yx).a;
    float s12 = textureOffset(tex, tc, off.yz).a;

    vec2 r2 = 2.0*texelSize;
    vec3 va = normalize(vec3(r2.x,s21-s01,0));
    vec3 vb = normalize(vec3(   0,s12-s10,r2.y));
    vec3 n = cross(vb,va);
	return n;
}

void applyBumpMap(vec4 b) {
    float a = b.g*10;
    norm += 0.1*vec3(cos(a),0,sin(a));
    //norm.x += b.r*0.5;
    //norm.z += (b.g-1.0)*1.0;
    normalize(norm);
}

void main( void ) {
	vec2 tc = gl_TexCoord[0].xy;
	norm = getNormal();

	if (doHeightTextures == 0) color = vec4(getColor(),1.0);
	else {
        vec4 cW1 = texture(texWoods, tc*1077);
        vec4 cW2 = texture(texWoods, tc*107);
        vec4 cW3 = texture(texWoods, tc*17);
        vec4 cW4 = texture(texWoods, tc);
        vec4 cW = mix(cW1,mix(cW2,mix(cW3,cW4,0.5),0.5),0.5);
        applyBumpMap(cW3);

        vec4 cG0 = texture(texGravel, tc*10777);
        vec4 cG1 = texture(texGravel, tc*1077);
        vec4 cG2 = texture(texGravel, tc*107);
        vec4 cG3 = texture(texGravel, tc*17);
        vec4 cG4 = texture(texGravel, tc);
        vec4 cG = mix(cG0,mix(cG1,mix(cG2,mix(cG3,cG4,0.5),0.5),0.5),0.5);
        color = mix(cG, cW, min(cW3.r*0.1*max(height,0),1));
	}

	applyBlinnPhong();
	//gl_FragColor = vec4( norm, 1.0 );
}
);

string VRTerrain::tessControlShader =
"#version 400 compatibility\n"
"#extension GL_ARB_tessellation_shader : enable\n"
GLSL(
layout(vertices = 4) out;
out vec3 tcPosition[];
out vec2 tcTexCoords[];
uniform float resolution;
)
"\n#define ID gl_InvocationID\n"
GLSL(
void main() {
    tcPosition[ID] = gl_in[ID].gl_Position.xyz;
    tcTexCoords[ID] = gl_in[ID].gl_TexCoord[0].xy;

    if (ID == 0) {
		vec4 mid = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;
		vec4 pos = gl_ModelViewProjectionMatrix * vec4(mid.xyz, 1.0);
		float D = length(pos.xyz);
		//int p = int(5.0/(resolution*D));
		//int res = int(pow(2,p));
		int res = int(resolution*2048/D); // 32*64
		res = clamp(res, 1, 64);
		if (res >= 64) res = 64; // take closest power of two to avoid jumpy effects
		else if (res >= 32) res = 32;
		else if (res >= 16) res = 16;
		else if (res >= 8) res = 8;
		else if (res >= 4) res = 4;
		else if (res >= 2) res = 2;

        gl_TessLevelInner[0] = res;
        gl_TessLevelInner[1] = res;

        gl_TessLevelOuter[0] = 64;
        gl_TessLevelOuter[1] = 64;
        gl_TessLevelOuter[2] = 64;
        gl_TessLevelOuter[3] = 64;
    }
}
);

string VRTerrain::tessEvaluationShader =
"#version 400 compatibility\n"
"#extension GL_ARB_tessellation_shader : enable\n"
GLSL(
layout( quads ) in;
in vec3 tcPosition[];
in vec2 tcTexCoords[];
out float height;
out vec4 pos;

uniform float heightScale;
uniform int channel;
uniform sampler2D texture;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 ta = mix(tcTexCoords[0], tcTexCoords[1], u);
    vec2 tb = mix(tcTexCoords[3], tcTexCoords[2], u);
    vec2 tc = mix(ta, tb, v);
    gl_TexCoord[0] = vec4(tc.x, tc.y, 1.0, 1.0);

    vec3 a = mix(tcPosition[0], tcPosition[1], u);
    vec3 b = mix(tcPosition[3], tcPosition[2], u);
    vec3 tePosition = mix(a, b, v);
    height = heightScale * texture2D(texture, gl_TexCoord[0].xy)[channel];
    tePosition.y = height;
    pos = gl_ModelViewProjectionMatrix * vec4(tePosition, 1);
    gl_Position = pos;
}
);

