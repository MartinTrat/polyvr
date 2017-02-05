#include "VRTree.h"

#include "core/objects/geometry/VRGeoData.h"
#include "core/objects/geometry/OSGGeometry.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/material/VRTextureGenerator.h"
#include "core/objects/VRLod.h"
#include "core/scene/VRSceneManager.h"
#include "core/math/VRConvexHull.h"

#include <OpenSG/OSGQuaternion.h>
#include <random>

using namespace OSG;

struct OSG::seg_params {
    int iterations; //number of iterations
    int child_number; //number of children

    float n_angle; //angle between n1 && parent n2
    float p_angle; //angle between axis (p1 p2) && parent axis (p1 p2)
    float l_factor; //length diminution factor
    float r_factor; //radius diminution factor

    float n_angle_var; //n_angle variation
    float p_angle_var; //p_angle variation
    float l_factor_var; //l_factor variation
    float r_factor_var; //r_factor variation

    seg_params () {
        iterations = 5;
        child_number = 5;

        n_angle = 0.2;
        p_angle = 0.6;
        l_factor = 0.8;
        r_factor = 0.5;

        n_angle_var = 0.2;
        p_angle_var = 0.4;
        l_factor_var = 0.2;
        r_factor_var = 0.2;
    }
};

struct OSG::segment {
    Vec3f p1, p2, n1, n2;
    Vec2f params[2];
    int lvl = 0;
    segment* parent;
    vector<segment*> children;

    //defaults are for the trunc
    segment(int _lvl = 0, segment* _parent = 0,
            Vec3f _p1 = Vec3f(0,0,0),
            Vec3f _n1 = Vec3f(0,1,0),
            Vec3f _p2 = Vec3f(0,1,0),
            Vec3f _n2 = Vec3f(0,1,0)) {
        p1 = _p1;
        p2 = _p2;
        n1 = _n1;
        n2 = _n2;
        lvl = _lvl;

        params[0] = Vec2f(1, 0);
        params[1] = Vec2f(1, 0);

        parent = _parent;
    }
};


VRTree::VRTree() : VRTransform("tree") {}
VRTree::~VRTree() {}
VRTreePtr VRTree::create() { return shared_ptr<VRTree>(new VRTree()); }
VRTreePtr VRTree::ptr() { return static_pointer_cast<VRTree>( shared_from_this() ); }

void VRTree::initLOD() {
    lod = VRLod::create("tree_lod");
    addChild(lod);
    lod->addChild(VRObject::create("tree_lod1"));
    lod->addChild(VRObject::create("tree_lod2"));
    lod->addChild(VRObject::create("tree_lod3"));
    lod->addDistance(20);
    lod->addDistance(50);
}

float VRTree::random (float min, float max) {
    if (max!=min) {
        float c = 1e5;
        return (rand()%(int)(c*max-c*min)+c*min)/c;
    } else return max;
}

float VRTree::variation(float val, float var) { return random(val*(1-var), val*(1+var)); }

Vec3f VRTree::randUVec() { return Vec3f(random(-1,1), random(-1,1), random(-1,1)); }

VRMaterialPtr VRTree::treeMat = 0;
VRMaterialPtr VRTree::leafMat = 0;

//rotate a vector with angle 'a' in a random direction
Vec3f VRTree::randomRotate(Vec3f v, float a) {
    if (a == 0) return v;

    Vec3f x, d;

    do x = randUVec();
    while (x.dot(v) > 1e-3);

    d = v.cross(x);
    d.normalize();

    Quaternion q(d, a);
    q.multVec(v, v);
    return v;
}

void VRTree::grow(const seg_params& sp, segment* p, int iteration) {
    if (p == 0) return;
    if (iteration == sp.iterations) return;

    for (int i=0;i<sp.child_number;i++) {
        segment* c = new segment(p->lvl+1, p, p->p2);
        branches.push_back(c);
        c->p2 = randomRotate(p->p2 - p->p1, variation(sp.p_angle, sp.p_angle_var));
        c->p2 *= variation(sp.l_factor, sp.l_factor_var);
        c->p2 += p->p2;

        c->n2 = c->p2 - c->p1;
        c->n2.normalize();
        c->n1 = p->n2 + (p->n2 - c->n2)*variation(sp.n_angle, sp.n_angle_var);

        c->params[0] = Vec2f(pow(sp.r_factor,iteration), 0);
        c->params[1] = Vec2f(pow(sp.r_factor,iteration+1), 0);

        p->children.push_back(c);
    }

    for (int i=0;i<sp.child_number;i++) {
        grow(sp, p->children[i], iteration+1);
    }
}

void VRTree::initMaterials() {
    if (!treeMat) {
        treeMat = VRMaterial::create("tree_wood");

        treeMat->setDiffuse(Color3f(0.8,0.8,0.6));
        treeMat->setAmbient(Color3f(0.4, 0.4, 0.2));
        treeMat->setSpecular(Color3f(0.1, 0.1, 0.1));

        string wdir = VRSceneManager::get()->getOriginalWorkdir();
        treeMat->readFragmentShader(wdir+"/shader/Trees/Shader_tree_base.fp");
        treeMat->readFragmentShader(wdir+"/shader/Trees/Shader_tree_base.dfp", true);
        treeMat->readVertexShader(wdir+"/shader/Trees/Shader_tree_base.vp");
        treeMat->readGeometryShader(wdir+"/shader/Trees/Shader_tree_base.gp");
        treeMat->readTessControlShader(wdir+"/shader/Trees/Shader_tree_base.tcp");
        treeMat->readTessEvaluationShader(wdir+"/shader/Trees/Shader_tree_base.tep");
        treeMat->setShaderParameter("texture", 0);
        treeMat->enableShaderParameter("OSGCameraPosition");

        VRTextureGenerator tg;
        tg.setSize(Vec3i(50,50,50));
        tg.add("Perlin", 1, Vec3f(0.7,0.5,0.3), Vec3f(1,0.9,0.7));
        tg.add("Perlin", 0.25, Vec3f(1,0.9,0.7), Vec3f(0.7,0.5,0.3));
        treeMat->setTexture(tg.compose(0));
    }

    if (!leafMat) {
        leafMat = VRMaterial::create("tree_leafs");

        leafMat->setDiffuse(Color3f(0.6,1.0,0.4));
        leafMat->setAmbient(Color3f(0.2, 0.6, 0.2));
        leafMat->setSpecular(Color3f(0.1, 0.1, 0.1));

        string wdir = VRSceneManager::get()->getOriginalWorkdir();
        leafMat->readFragmentShader(wdir+"/shader/Trees/Shader_leafs.fp");
        leafMat->readFragmentShader(wdir+"/shader/Trees/Shader_leafs.dfp", true);
        leafMat->readVertexShader(wdir+"/shader/Trees/Shader_leafs.vp");
        leafMat->readGeometryShader(wdir+"/shader/Trees/Shader_leafs.gp");
        leafMat->setShaderParameter("tex", 0);
        leafMat->setTexture( wdir+"/examples/maple-leaf.png" );
    }
}

void VRTree::initArmatureGeo() {
    if (!lod) initLOD();

    VRGeoData geo[3];
    for (segment* s : branches) {
        geo[0].pushVert(s->p1, s->n1, s->params[0]);
        geo[0].pushVert(s->p2, s->n2, s->params[1]);
        geo[0].pushLine();

        if (s->lvl <= 3) {
            geo[1].pushVert(s->p1, s->n1, s->params[0]);
            geo[1].pushVert(s->p2, s->n2, s->params[1]);
            geo[1].pushLine();
        }

        if (s->lvl <= 2) {
            geo[2].pushVert(s->p1, s->n1, s->params[0]);
            geo[2].pushVert(s->p2, s->n2, s->params[1]);
            geo[2].pushLine();
        }
    }

    for (int i=0; i<3; i++) {
        woodGeos.push_back( geo[i].asGeometry("wood2") );
        auto g = woodGeos[i];
        g->setPatchVertices(2);
        g->setType(GL_PATCHES);
        g->setMaterial(treeMat);
        lod->getChild(i)->addChild(g);
    }
}

VRObjectPtr VRTree::copy(vector<VRObjectPtr> children) {
    auto tree = VRTree::create();
    tree->trunc = trunc;
    tree->lod;
    tree->branches = branches;
    tree->woodGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(0)->getChild(0) ) );
    tree->woodGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(1)->getChild(0) ) );
    tree->woodGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(2)->getChild(0) ) );
    tree->leafGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(0)->getChild(1) ) );
    tree->leafGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(1)->getChild(1) ) );
    tree->leafGeos.push_back( dynamic_pointer_cast<VRGeometry>( children[0]->getChild(2)->getChild(1) ) );
    return tree;
}

void VRTree::testSetup() {
    srand(time(0));
    trunc = new segment();
    branches = vector<segment*>();
    branches.push_back(trunc);
    seg_params sp;
    grow(sp, trunc);
    initMaterials();
    initArmatureGeo();
}

void VRTree::setup(int branching, int iterations, int seed,
                   float n_angle, float p_angle, float l_factor, float r_factor,
                   float n_angle_v, float p_angle_v, float l_factor_v, float r_factor_v) {
    srand(seed);
    trunc = new segment();
    branches = vector<segment*>();
    branches.push_back(trunc);
    seg_params sp;

    sp.child_number = branching;
    sp.iterations = iterations;
    sp.n_angle = n_angle;
    sp.p_angle = p_angle;
    sp.l_factor = l_factor;
    sp.r_factor = r_factor;
    sp.n_angle_var = n_angle_v;
    sp.p_angle_var = p_angle_v;
    sp.l_factor_var = l_factor_v;
    sp.r_factor_var = r_factor_v;

    grow(sp, trunc);
    initMaterials();
    initArmatureGeo();
}

void VRTree::addLeafs(int lvl, int amount) { // TODO: add default material!
    if (!lod) initLOD();

    if (leafGeos.size() == 0) {
        for (int i=0; i<3; i++) {
            auto g = VRGeometry::create("branches");
            leafGeos.push_back( g );
            lod->getChild(i)->addChild(g);
            g->setMaterial(leafMat);
        }
    }

    random_device rd;
    mt19937 e2(rd());
    normal_distribution<> ndist(0,1);

    auto randVecInSphere = [&](float r) {
        return Vec3f(ndist(e2), ndist(e2), ndist(e2))*r;
    };

    float s = 0.03;
    float ca = 1.0; // carotene
    float ch = 1.0; // chlorophyl
    ca = 0.5 + rand()*0.5/RAND_MAX;
    ch = 0.5 + rand()*0.5/RAND_MAX;
    VRGeoData geo0, geo1, geo2;
    for (auto b : branches) {
        if (b->lvl != lvl) continue;

        // compute branch segment basis
        Vec3f p = (b->p1 + b->p2)*0.5;
        Vec3f d = b->p2 - b->p1;
        Vec3f u = Vec3f(0,1,0);
        Vec3f n = u.cross(d); n.normalize();
        u = d.cross(n); u.normalize();

        float L = d.length();

        for (int i=0; i<amount; i++) {
            Vec3f v = randVecInSphere(L*0.3);
            Vec3f n = p+v-b->p1;
            n.normalize();
            // TODO: add model for carotene and chlorophyl depending on leaf age/size and more..
            geo0.pushVert(p+v, n, Vec3f(s,ca,ch)); // color: leaf size, amount of carotene, amount of chlorophyl
            geo0.pushPoint();
        }
    }

    for (int i=0; i<geo0.size(); i+=4) {
        auto c = geo0.getColor(i); c[0] *= 2; // double lod leaf size
        geo1.pushVert( geo0.getPosition(i), geo0.getNormal(i), c);
        geo1.pushPoint();
    }

    for (int i=0; i<geo0.size(); i+=16) {
        auto c = geo0.getColor(i); c[0] *= 4; // double lod leaf size
        geo2.pushVert( geo0.getPosition(i), geo0.getNormal(i), c);
        geo2.pushPoint();
    }

    geo0.apply(leafGeos[0]);
    geo1.apply(leafGeos[1]);
    geo2.apply(leafGeos[2]);
}

void VRTree::setLeafMaterial(VRMaterialPtr mat) {
    for (auto g : leafGeos) g->setMaterial(mat);
}

void VRTree::createHullLeafLod(VRGeoData& geo, float amount, Vec3f offset) { // TODO, depending on amount, make multiple smaller convex hulls
    if (leafGeos.size() == 0) return;
    VRGeoData g0(leafGeos[0]);

    VRConvexHull hull;

    Vec4f meanColor;
    VRGeoData tmpData;
    int N = g0.size()*amount;
    float D = 1.0/amount;
    for (int i=0; i<N; i++) {
        int j = max( min( int(i*D), g0.size()-1), 0);
        meanColor += g0.getColor(j);
        Pnt3f pos = g0.getPosition(j) + offset;
        tmpData.pushVert( pos, g0.getNormal(j) );
        tmpData.pushPoint();
    }

    auto res = VRGeoData( hull.compute( tmpData.asGeometry("tmpdata") ) );
    meanColor *= 1.0/N;
    float ca = meanColor[1]; // carotene
    float ch = meanColor[2]; // chlorophyll
    Vec3f leafColor = Vec3f(0.4*ca,0.8*ch,0.2*ch);
    for (int i=0; i<res.size(); i++) {
        res.pushColor( leafColor );
    }

    geo.append(res);
}

void VRTree::createHullTrunkLod(VRGeoData& geo, float amount, Vec3f offset) { // TODO
    if (leafGeos.size() == 0) return;
    VRGeoData g0(leafGeos[0]);

    auto normalize = [](Vec3f v) {
        v.normalize();
        return v;
    };

    float r = 0.1;
    float h = 3;
    int N = geo.size();
    geo.pushVert( Pnt3f(-r,0,-r) + offset, normalize( Vec3f(-1,0,-1) ) );
    geo.pushVert( Pnt3f(-r,0, r) + offset, normalize( Vec3f(-1,0, 1) ) );
    geo.pushVert( Pnt3f( r,0, r) + offset, normalize( Vec3f( 1,0, 1) ) );
    geo.pushVert( Pnt3f( r,0,-r) + offset, normalize( Vec3f( 1,0,-1) ) );
    geo.pushVert( Pnt3f(-r,h,-r) + offset, normalize( Vec3f(-1,0,-1) ) );
    geo.pushVert( Pnt3f(-r,h, r) + offset, normalize( Vec3f(-1,0, 1) ) );
    geo.pushVert( Pnt3f( r,h, r) + offset, normalize( Vec3f( 1,0, 1) ) );
    geo.pushVert( Pnt3f( r,h,-r) + offset, normalize( Vec3f( 1,0,-1) ) );
    geo.pushQuad(N+0,N+1,N+5,N+4);
    geo.pushQuad(N+1,N+2,N+6,N+5);
    geo.pushQuad(N+2,N+3,N+7,N+6);
    geo.pushQuad(N+3,N+0,N+4,N+7);
}






