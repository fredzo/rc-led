#ifndef RADIO_H
#define RADIO_H

#include <RcLedConf.h>
#include <EffectManager.h>

//////////// Gestion de la radio //////////////

// Etat des canaux radio
enum ChannelState { LEFT, CENTER, RIGHT, FOWRWARD, STOP, BACKWARD };
// Etat du déplacement (avant / arrière / frein)
enum MotionState { FRONT, BRAKE, BACK, NONE };

void radioInit(EffectManager* effectManager);

void radioProcessSignal();

ChannelState radioGetChannel1State();
MotionState radioGetMotionState();

#endif 