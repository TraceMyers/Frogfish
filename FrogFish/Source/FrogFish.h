#pragma once

#include <BWAPI.h>
#include <Windows.h>
// Remember not to use "Broodwar" in any global class constructor!

using namespace BWAPI;

class FrogFish : public AIModule {
public:
  // Virtual functions for callbacks, leave these as they are.
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(Player player, std::string text);
  virtual void onPlayerLeft(Player player);
  virtual void onNukeDetect(Position target);
  virtual void onUnitDiscover(Unit unit);
  virtual void onUnitEvade(Unit unit);
  virtual void onUnitShow(Unit unit);
  virtual void onUnitHide(Unit unit);
  virtual void onUnitCreate(Unit unit);
  virtual void onUnitDestroy(Unit unit);
  virtual void onUnitMorph(Unit unit);
  virtual void onUnitRenegade(Unit unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(Unit unit);
  // Everything below this line is safe to modify.
};
