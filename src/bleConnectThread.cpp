#include "bleConnectThread.h"
#include "global.h"

void BLEConnectThread::run() {
    while (!this->isInterruptionRequested()) {
        for (auto device: Global::dglabList) {
            if (device->getBLEController()->state() == QLowEnergyController::ControllerState::UnconnectedState) {
                SKIPNEXTQWARNING(SKIPNEXTERROR(device->getBLEController()->connectToDevice()));
            }
        }
        sleep(3);
    }
}
