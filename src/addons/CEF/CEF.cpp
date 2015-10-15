#include "CEF.h"
#include "CEFWindowsKey.h"

#include <OpenSG/OSGTextureEnvChunk.h>
#include <OpenSG/OSGTextureObjChunk.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGGeoProperties.h>

#include "core/scene/VRSceneManager.h"
#include "core/scene/VRScene.h"
#include "core/setup/devices/VRDevice.h"
#include "core/setup/devices/VRKeyboard.h"
#include "core/objects/material/VRMaterial.h"
#include "core/utils/VRLogger.h"

using namespace std;
using namespace OSG;

vector< weak_ptr<CEF> > instances;
bool cef_gl_init = false;

CEF_handler::CEF_handler() {
    image = Image::create();
}

bool CEF_handler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    rect = CefRect(0, 0, width, height);
    return true;
}

void CEF_handler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) {
    image->set(Image::OSG_BGRA_PF, width, height, 1, 0, 1, 0.0, (const uint8_t*)buffer, Image::OSG_UINT8_IMAGEDATA, true, 1);
}

OSG::ImageRecPtr CEF_handler::getImage() { return image; }

void CEF_handler::resize(int resolution, float aspect) {
    width = resolution;
    height = width/aspect;
}

CEF_client::CEF_client() {
    handler = new CEF_handler();
}

CefRefPtr<CefRenderHandler> CEF_client::GetRenderHandler() { return handler; }
CefRefPtr<CEF_handler> CEF_client::getHandler() { return handler; }

CEF::CEF() {
    client = new CEF_client();
    update_callback = VRFunction<int>::create("webkit_update", boost::bind(&CEF::update, this));
    auto scene = VRSceneManager::getCurrent();
    if (scene) scene->addUpdateFkt(update_callback);
}

CEF::~CEF() {
    browser = 0;
    cout << "CEF destroyed " << this << endl;
}

void CEF::shutdown() { if (!cef_gl_init) return; cout << "CEF shutdown\n"; CefShutdown(); }

CEFPtr CEF::create() {
    auto cef = CEFPtr(new CEF());
    instances.push_back(cef);
    return cef;
}

void CEF::initiate() {
    init = true;

    static bool global_init = false;
    if (!global_init) {
        cout << "Global CEF init\n";
        cef_gl_init = true;
        CefSettings settings;
#ifndef _WIN32
        string bsp = VRSceneManager::get()->getOriginalWorkdir() + "/ressources/cef/CefSubProcess";
#else
        string bsp = VRSceneManager::get()->getOriginalWorkdir() + "/ressources/cef/CefSubProcessWin.exe";
#endif
        string ldp = VRSceneManager::get()->getOriginalWorkdir() + "/ressources/cef/locales";
        string rdp = VRSceneManager::get()->getOriginalWorkdir() + "/ressources/cef";
        string lfp = VRSceneManager::get()->getOriginalWorkdir() + "/ressources/cef/wblog.log";
        CefString(&settings.browser_subprocess_path).FromASCII(bsp.c_str());
        CefString(&settings.locales_dir_path).FromASCII(ldp.c_str());
        CefString(&settings.resources_dir_path).FromASCII(rdp.c_str());
        CefString(&settings.log_file).FromASCII(lfp.c_str());
        settings.no_sandbox = true;

        CefMainArgs args;
        CefInitialize(args, settings, 0, 0);
        global_init = true;
    }

    CefWindowInfo win;
    CefBrowserSettings browser_settings;

    win.SetAsWindowless(0, true);
    browser = CefBrowserHost::CreateBrowserSync(win, client, "www.google.de", browser_settings, 0);
}

void CEF::setMaterial(VRMaterialPtr mat) {
    if (!mat) return;
    this->mat = mat;
    mat->setTexture(client->getHandler()->getImage());
}

string CEF::getSite() { return site; }
void CEF::reload() { browser->Reload(); }

void CEF::update() {
    if (init) CefDoMessageLoopWork();
}

void CEF::open(string site) {
    if (!init) initiate();
    this->site = site;
    browser->GetMainFrame()->LoadURL(site);
}

void CEF::resize() {
    client->getHandler()->resize(resolution, aspect);
    if (init) browser->GetHost()->WasResized();
    if (init) reload();
}

void CEF::reloadScripts(string path) {
    for (auto i : instances) {
        auto cef = i.lock();
        if (!cef) return;
        string s = cef->getSite();
        stringstream ss(s); vector<string> res; while (getline(ss, s, '/')) res.push_back(s); // split by ':'
        if (res.size() == 0) continue;
        if (res[res.size()-1] == path) {
            cef->resize();
            cef->reload();
        }
    }
}

void CEF::setResolution(float a) { resolution = a; resize(); }
void CEF::setAspectRatio(float a) { aspect = a; resize(); }

// dev callbacks:

void CEF::addMouse(VRDevice* dev, VRObjectWeakPtr obj, int lb, int rb, int wu, int wd) {
    if (dev == 0 || obj.lock() == 0) return;
    this->obj = obj;

    if (!mouse_dev_callback) mouse_dev_callback = VRFunction<VRDevice*>::create( "CEF::MOUSE", boost::bind(&CEF::mouse, this, lb,rb,wu,wd,_1 ) );
    dev->addSignal(-1,0)->add(mouse_dev_callback);
    dev->addSignal(-1,1)->add(mouse_dev_callback);

    if (!mouse_move_callback) mouse_move_callback = VRFunction<int>::create( "CEF::MM", boost::bind(&CEF::mouse_move, this, dev, _1) );
    auto scene = VRSceneManager::getCurrent();
    if (scene) scene->addUpdateFkt(mouse_move_callback);
}

void CEF::addKeyboard(VRDevice* dev) {
    if (dev == 0) return;
    if (!keyboard_dev_callback) keyboard_dev_callback = VRFunction<VRDevice*>::create( "CEF::KR", boost::bind(&CEF::keyboard, this, _1 ) );
    dev->addSignal(-1, 0)->add( keyboard_dev_callback );
    dev->addSignal(-1, 1)->add( keyboard_dev_callback );
}

void CEF::mouse_move(VRDevice* dev, int i) {
    if (dev == 0) return;
    auto geo = obj.lock();
    if (!geo) return;
    VRIntersection ins = dev->intersect(geo);

    if (!ins.hit) return;
    if (ins.object.lock() != geo) return;

    CefMouseEvent me;
    me.x = ins.texel[0]*resolution;
    me.y = ins.texel[1]*(resolution/aspect);
    browser->GetHost()->SendMouseMoveEvent(me, dev->b_state(dev->key()));
}

void CEF::mouse(int lb, int rb, int wu, int wd, VRDevice* dev) {
    int b = dev->key();
    bool down = dev->getState();

    if (b == lb) b = 0;
    else if (b == rb) b = 2;
    else if (b == wu) b = 3;
    else if (b == wd) b = 4;
    else return;

    auto geo = obj.lock();
    if (!geo) return;

    VRIntersection ins = dev->intersect(geo);
    auto iobj = ins.object.lock();

    if (VRLog::tag("net")) {
        string o = "NONE";
        if (iobj) o = iobj->getName();
        stringstream ss;
        ss << "CEF::mouse " << this << " dev " << dev->getName();
        ss << " hit " << ins.hit << " " << o << ", trg " << geo->getName();
        ss << " b: " << b << " s: " << down;
        ss << " texel: " << ins.texel;
        ss << endl;
        VRLog::log("net", ss.str());
    }

    auto host = browser->GetHost();
    if (!host) return;
    if (!ins.hit) { host->SendFocusEvent(false); focus = false; return; }
    if (iobj != geo) { host->SendFocusEvent(false); focus = false; return; }
    host->SendFocusEvent(true); focus = true;

    int width = resolution;
    int height = resolution/aspect;

    CefMouseEvent me;
    me.x = ins.texel[0]*width;
    me.y = ins.texel[1]*height;

    if (b < 3) {
        cef_mouse_button_type_t mbt;
        if (b == 0) mbt = MBT_LEFT;
        if (b == 1) mbt = MBT_MIDDLE;
        if (b == 2) mbt = MBT_RIGHT;
        host->SendMouseClickEvent(me, mbt, !down, 1);
    }

    if (b == 3 || b == 4) {
        int d = b==3 ? -1 : 1;
        host->SendMouseWheelEvent(me, d*width*0.05, d*height*0.05);
    }
}

void CEF::keyboard(VRDevice* dev) {
    if (!focus) return;
    if (dev->getType() != "keyboard") return;
    //bool down = dev->getState();
    VRKeyboard* kb = (VRKeyboard*)dev;
    auto event = kb->getGtkEvent();
    auto host = browser->GetHost();
    if (!host) return;

    CefKeyEvent kev;
    kev.modifiers = GetCefStateModifiers(event->state);
    if (event->keyval >= GDK_KP_Space && event->keyval <= GDK_KP_9) kev.modifiers |= EVENTFLAG_IS_KEY_PAD;
    if (kev.modifiers & EVENTFLAG_ALT_DOWN) kev.is_system_key = true;

    KeyboardCode windows_key_code = GdkEventToWindowsKeyCode(event);
    kev.windows_key_code = GetWindowsKeyCodeWithoutLocation(windows_key_code);

    kev.native_key_code = event->keyval;

    if (windows_key_code == VKEY_RETURN) kev.unmodified_character = '\r'; else
    kev.unmodified_character = static_cast<int>(gdk_keyval_to_unicode(event->keyval));

    if (kev.modifiers & EVENTFLAG_CONTROL_DOWN) kev.character = GetControlCharacter(windows_key_code, kev.modifiers & EVENTFLAG_SHIFT_DOWN); else
    kev.character = kev.unmodified_character;

    if (event->type == GDK_KEY_PRESS) {
        kev.type = KEYEVENT_RAWKEYDOWN; host->SendKeyEvent(kev);
    } else {
        kev.type = KEYEVENT_KEYUP; host->SendKeyEvent(kev);
        kev.type = KEYEVENT_CHAR; host->SendKeyEvent(kev);
    }
}
