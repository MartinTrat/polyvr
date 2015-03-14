#include "VRScript.h"
#include <iostream>
#include <functional>
#include "core/scene/VRScene.h"
#include "VRPyObject.h"
#include "VRPyGeometry.h"
#include "VRPyDevice.h"
#include "VRPyLight.h"
#include "VRPyLod.h"
#include "core/setup/devices/VRMobile.h"
#include "VRPySocket.h"
#include "VRPyHaptic.h"
#include "VRPyBaseT.h"
#include "core/scene/VRSceneManager.h"
#include "core/utils/VRTimer.h"
#include "core/utils/toString.h"
#include "core/setup/VRSetupManager.h"
#include "core/setup/VRSetup.h"
#include "core/objects/material/VRMaterial.h"
#include <libxml++/nodes/element.h>

OSG_BEGIN_NAMESPACE;
using namespace std;

void updateArgPtr(VRScript::arg* a) {
    string t = a->type;
    VRScene* scene = VRSceneManager::getCurrent();
    VRSetup* setup = VRSetupManager::getCurrent();

    if (t == "VRPyObjectType" || t == "VRPyGeometryType" || t == "VRPyTransformType" || t == "VRPyLightType" || t == "VRPyLodType") {
        a->ptr = (void*)scene->get(a->val);
        return;
    }
    if (t == "VRPySocketType") {
        a->ptr = (void*)scene->getSocket(a->val);
        return;
    }
    if (t == "VRPyDeviceType" || t == "VRPyHapticType") {
        a->ptr = (void*)setup->getDevice(a->val);
        return;
    }

    if (t == "int" || t == "float" || t == "str" || t == "NoneType") return;

    cout << "\nupdateArgPtr: " << t << " is an unknown argument type!" << endl;
}

VRScript::trig::trig() { setName("trigger"); }
VRScript::arg::arg(string nspace, string name) {
    setSeparator('_');
    setNameSpace(nspace);
    setName(name);
}

void VRScript::clean() {
    VRMobile* mob = (VRMobile*)VRSetupManager::getCurrent()->getDevice(this->mobile);
    if (mob) mob->remWebSite(getName());

    VRScene* scene = VRSceneManager::getCurrent();

    for (t_itr = trigs.begin(); t_itr != trigs.end(); t_itr++) {
        trig* t = t_itr->second;
        if (t->a) { args.erase(t->a->getName()); delete t->a; }
        if (t->soc) t->soc->unsetCallbacks();
        if (t->sig) t->sig->sub(cbfkt_dev);
        if (t->trigger == "on_timeout") scene->dropTimeoutFkt(cbfkt_sys);
        t->soc = 0;
        t->sig = 0;
        t->a = 0;
    }
}

void VRScript::update() {
    if (type == "HTML") {
        VRMobile* mob = (VRMobile*)VRSetupManager::getCurrent()->getDevice(mobile);
        if (mob) mob->addWebSite(getName(), core);
    }

    VRScene* scene = VRSceneManager::getCurrent();

    for (t_itr = trigs.begin(); t_itr != trigs.end(); t_itr++) {
        trig* t = t_itr->second;
        if (t->trigger == "on_timeout") {
            int i = toInt(t->param.c_str());
            scene->addTimeoutFkt(cbfkt_sys, 0, i);
            continue;
        }

        if (t->trigger == "on_device") {
            VRDevice* dev = VRSetupManager::getCurrent()->getDevice(t->dev);
            int state = 0;
            if (t->state == "Pressed") state = 1;
            else if (t->state == "Released") state = 0;
            else continue;
            if (dev != 0) {
                t->sig = dev->addSignal(t->key, state);
                if (t->sig == 0) continue;
                t->sig->add(cbfkt_dev);
            }

            // add dev argument
            arg* a = new arg(VRName::getName(), "dev");
            args[a->getName()] = a;
            a->type = "VRPyDeviceType";
            if (dev) {
                if (dev->getType() == "haptic") a->type = "VRPyHapticType";
                a->val = dev->getName();
            }
            a->ptr = dev;
            a->trig = true;
            t->a = a;
            continue;
        }

        if (t->trigger == "on_socket") {
            t->soc = scene->getSocket(t->dev);
            if (t->soc == 0) continue;
            t->soc->setTCPCallback(cbfkt_soc);

            // add msg argument
            arg* a = new arg(VRName::getName(), "msg");
            args[a->getName()] = a;
            a->type = "str";
            a->val = "";
            t->a = a;
            a->trig = true;
            continue;
        }
    }

    // update args namespaces && map
    map<string, arg*> tmp_args;
    for (auto _a : args) {
        arg* a = _a.second;
        a->setNameSpace(VRName::getName());
        tmp_args[a->getName()] = a;
        changeArgValue(a->getName(), a->val);
    }
    args = tmp_args;

    // update head
    head = "";
    if (type == "Python") {
        head = "def " + name + "(";
        int i=0;
        for (a_itr = args.begin(); a_itr != args.end(); a_itr++, i++) {
            if (i != 0) head += ", ";
            head += a_itr->second->getName();
        }
        head += "):\n";
    }
}

VRScript::VRScript(string _name) {
    setSeparator('_');
    setNameSpace("__script__");
    setName(_name);
    cbfkt_sys = new VRFunction<int>(_name + "_ScriptCallback_sys", boost::bind(&VRScript::execute, this));
    cbfkt_dev = new VRFunction<VRDevice*>(_name + "_ScriptCallback_dev", boost::bind(&VRScript::execute_dev, this, _1));
    cbfkt_soc = new VRFunction<string>(_name + "_ScriptCallback_soc", boost::bind(&VRScript::execute_soc, this, _1));
}

VRScript::~VRScript() {
    for (auto a : args) delete a.second;
    for (auto t : trigs) delete t.second;
}

VRScript::arg* VRScript::addArgument() {
    clean();
    arg* a = new arg(VRName::getName());
    args[a->getName()] = a;
    update();
    return a;
}

PyObject* VRScript::getPyObj(arg* a) {
    updateArgPtr(a);
    if (a->type == "int") return Py_BuildValue("i", toInt(a->val.c_str()));
    else if (a->type == "float") return Py_BuildValue("f", toFloat(a->val.c_str()));
    else if (a->type == "NoneType") return Py_None;
    else if (a->type == "str") return PyString_FromString(a->val.c_str());
    else if (a->type == "VRPyObjectType") return VRPyObject::fromPtr((VRObject*)a->ptr);
    else if (a->type == "VRPyTransformType") return VRPyTransform::fromPtr((VRTransform*)a->ptr);
    else if (a->type == "VRPyGeometryType") return VRPyGeometry::fromPtr((VRGeometry*)a->ptr);
    else if (a->type == "VRPyLightType") return VRPyLight::fromPtr((VRLight*)a->ptr);
    else if (a->type == "VRPyLodType") return VRPyLod::fromPtr((VRLod*)a->ptr);
    else if (a->type == "VRPyDeviceType") return VRPyDevice::fromPtr((VRDevice*)a->ptr);
    else if (a->type == "VRPyHapticType") return VRPyHaptic::fromPtr((VRHaptic*)a->ptr);
    else if (a->type == "VRPySocketType") return VRPySocket::fromPtr((VRSocket*)a->ptr);
    else { cout << "\ngetPyObj ERROR: " << a->type << " unknown!\n"; return NULL; }
}

void VRScript::changeArgName(string name, string _new) {
    clean();
    a_itr = args.find(name);
    if (a_itr == args.end()) return;
    a_itr->second->setName(_new);
    update();
}

void VRScript::changeArgValue(string name, string _new) {
    if (args.count(name) == 0) return;
    args[name]->val = _new;
    args[name]->ptr = 0;
    updateArgPtr(args[name]);
}

void VRScript::changeArgType(string name, string _new) {
    if (args.count(name) == 0) return;
    args[name]->type = _new;
    args[name]->val = "0";
}

VRScript::Search VRScript::getSearch() { return search; }
VRScript::Search VRScript::find(string s) {
    search = Search();
    if (s == "") return search;

    search.search = s;
    map<int, bool> res;

    uint pos = core.find(s, 0);
    while(pos != string::npos) {
        res[pos] = false;
        pos = core.find(s, pos+1);
    }
    pos = core.find("\n", 0);
    while(pos != string::npos) {
        res[pos] = true;
        pos = core.find("\n", pos+1);
    }

    int l = getHeadSize()+1;
    int lpo = 0;
    for (auto r : res) {
        if (r.second) { l++; lpo = r.first; continue; } // new line
        if (search.result.count(l) == 0) search.result[l] = vector<int>();
        search.result[l].push_back(r.first - lpo);
    }

    search.N = search.result.size();

    return search;
}

map<string, VRScript::arg*> VRScript::getArguments() { return args; }

void VRScript::setName(string n) { clean(); VRName::setName(n); update(); }
void VRScript::setFunction(PyObject* fkt) { this->fkt = fkt; }
void VRScript::setCore(string core) { clean(); this->core = core; update(); }
void VRScript::setType(string type) { clean(); this->type = type; update(); }
void VRScript::setHTMLHost(string mobile) { clean(); this->mobile = mobile; update(); }

string VRScript::getCore() { return core; }
string VRScript::getHead() { return head; }
string VRScript::getScript() { return head + core; }
string VRScript::getType() { return type; }
string VRScript::getMobile() { return mobile; }
int VRScript::getHeadSize() { // number of head lines
    if (type == "Python") return 1;
    return 0;
}

void VRScript::execute() {
    if (type == "Python") {
        if (fkt == 0) return;
        if (!active) return;
        if (PyErr_Occurred() != NULL) PyErr_Print();

        VRTimer timer; timer.start();

        PyObject* pArgs = PyTuple_New(args.size());

        int i=0;
        for (a_itr = args.begin(); a_itr != args.end(); a_itr++, i++) {
            arg* a = a_itr->second;
            a->pyo = getPyObj(a);
            PyTuple_SetItem(pArgs, i, a->pyo);
            a_itr->second = a;
        }
        PyObject_CallObject(fkt, pArgs);

        execution_time = timer.stop();

        Py_XDECREF(pArgs);

        if (PyErr_Occurred() != NULL) PyErr_Print();
    }

    if (type == "HTML") {
        VRMobile* mob = (VRMobile*)VRSetupManager::getCurrent()->getDevice(this->mobile);
        if (mob) mob->updateClients(getName());
    }

    if (type == "GLSL") {
        for (auto m : VRMaterial::materials) {
            VRMaterial* mat = m.second;
            if (mat->getVertexScript() == getName()) mat->setVertexScript(getName());
            if (mat->getFragmentScript() == getName()) mat->setFragmentScript(getName());
            if (mat->getGeometryScript() == getName()) mat->setGeometryScript(getName());
        }
    }
}

void VRScript::execute_dev(VRDevice* dev) {
    if (type != "Python") return;
    execute();
}

void VRScript::execute_soc(string s) {
    if (type != "Python") return;
    if (!active) return;

    args["Message"]->val = s;
    execute();
}

void VRScript::enable(bool b) { active = b; }
bool VRScript::enabled() { return active; }

map<string, VRScript::trig*> VRScript::getTriggers() { return trigs; }
void VRScript::addTrigger() { trig* t = new trig(); trigs[t->getName()] = t; }
void VRScript::changeTrigger(string name, string trigger) { clean(); trigs[name]->trigger = trigger; update(); }
void VRScript::changeTrigDev(string name, string dev) { clean(); trigs[name]->dev = dev; update(); }
void VRScript::changeTrigParams(string name, string params) { clean(); trigs[name]->param = params; update(); }
void VRScript::changeTrigKey(string name, int key) { clean(); trigs[name]->key = key; update(); }
void VRScript::changeTrigState(string name, string state) { clean(); trigs[name]->state = state; update(); }

void VRScript::remTrigger(string name) {
    if (trigs.count(name) == 0) return;
    clean();
    delete trigs[name]; trigs.erase(name);
    update();
}

float VRScript::getExecutionTime() { return execution_time; }

void VRScript::remArgument(string name) {
    if (args.count(name) == 0) return;
    clean();
    delete args[name]; args.erase(name);
    update();
}

void VRScript::save(xmlpp::Element* e) {
    saveName(e);
    e->set_attribute("core", core);
    e->set_attribute("type", type);
    e->set_attribute("mobile", mobile);

    for (a_itr = args.begin(); a_itr != args.end(); a_itr++) {
        arg* a = a_itr->second;
        if (a->trig) continue;

        xmlpp::Element* ea = e->add_child("arg");
        ea->set_attribute("type", a->type);
        ea->set_attribute("value", a->val);
        a->saveName(ea);
    }

    for (t_itr = trigs.begin(); t_itr != trigs.end(); t_itr++) {
        trig* t = t_itr->second;
        xmlpp::Element* ea = e->add_child("trig");
        ea->set_attribute("type", t->trigger);
        ea->set_attribute("dev", t->dev);
        ea->set_attribute("state", t->state);
        ea->set_attribute("param", t->param);
        ea->set_attribute("key", toString(t->key));
        t->saveName(ea);
    }
}

void VRScript::load(xmlpp::Element* e) {
    clean();
    loadName(e);
    if (e->get_attribute("core")) core = e->get_attribute("core")->get_value();
    if (e->get_attribute("type")) type = e->get_attribute("type")->get_value();
    if (e->get_attribute("mobile")) mobile = e->get_attribute("mobile")->get_value();

    xmlpp::Node::NodeList nl = e->get_children();
    xmlpp::Node::NodeList::iterator itr;
    for (itr = nl.begin(); itr != nl.end(); itr++) {
        xmlpp::Node* n = *itr;
        xmlpp::Element* el = dynamic_cast<xmlpp::Element*>(n);
        if (!el) continue;

        string name = el->get_name();
        if (name == "arg") {
            arg* a = addArgument();
            a->type = el->get_attribute("type")->get_value();
            a->val  = el->get_attribute("value")->get_value();
            string oname = a->getName();
            a->loadName(el);
            changeArgName(oname, a->getName());
        }

        if (name == "trig") {
            trig* t = new trig();
            t->trigger = el->get_attribute("type")->get_value();
            t->dev = el->get_attribute("dev")->get_value();
            t->state = el->get_attribute("state")->get_value();
            t->param = el->get_attribute("param")->get_value();
            t->key = toInt( el->get_attribute("key")->get_value() );
            t->loadName(el);
            trigs[t->getName()] = t;

            if (t->trigger == "on_scene_load" && active) {
                VRScene* scene = VRSceneManager::getCurrent();
                scene->queueJob(cbfkt_sys);
            }
        }
    }

    update();
}

OSG_END_NAMESPACE;
