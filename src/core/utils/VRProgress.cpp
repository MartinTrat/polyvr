#include "VRProgress.h"

#include "VRFunction.h"

VRProgress::VRProgress(string title, int max, Mode m) { setup(title, max, m); }

VRProgress::~VRProgress() {
	if (mode == CONSOLE_M) cout << endl;
}

void VRProgress::setCallback(VRUpdatePtr cb) { callback = cb; }

VRProgressPtr VRProgress::create() { return VRProgressPtr( new VRProgress() ); }

void VRProgress::update(int i) {
    if (count < max) {
        count += i;
        float k = float(count)/max;
        if (k-part < 0.01) return;
        part = k;
    }

    switch(mode) {
        case CONSOLE_M:
            cout << "\r" << title << " " << this << " " << int(part*100) << "%" << flush;
            break;
		case CALLBACK_M:
            if (auto cl = callback.lock()) (*cl)(part*100);
            break;
        case WIDGET_M:
            break;
    }
}

void VRProgress::finish() { count = max; part = 1.0; update(0); }
float VRProgress::get() { return part; }
void VRProgress::reset() { part = count = 0; }

void VRProgress::setup(string title, int max, Mode m) {
    this->title = title;
    this->max = max;
    mode = m;
    reset();
}
