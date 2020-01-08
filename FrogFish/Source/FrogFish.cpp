#include "FrogFish.h"
#include "draw/DebugDraw.h"
#include "production/MakeQueue.h"
#include "data/UnitStorage.h"
#include "data/BaseStorage.h"
#include "data/EnemyBase.h"
#include "data/EconStats.h"
#include "datamgmt/BaseOwnership.h"
#include "datamgmt/BaseAssets.h"
#include "control/WorkerControl.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <vector>
#include "utility/BWTimer.h"

using namespace BWAPI;
using namespace Filter;

UnitStorage unit_storage;
BaseStorage base_storage;
MakeQueue make_queue;
EconStats econ_stats;
BWTimer<void *> timer;

void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(24);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem();
    init_base_storage(the_map, base_storage);
    econ_stats.init();
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {return;}
    timer.on_frame_update();

    // handle data
    unit_storage.update();
    assign_new_bases(the_map, base_storage, unit_storage);
    assign_base_assets(the_map, base_storage, unit_storage);
    unassign_bases(base_storage);
    unit_storage.clear_newly_assigned();

    //econ_stats.on_frame_update();    

    // draw
    draw_units(unit_storage);
    draw_base_info(base_storage);

    if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {return;}
    // run commands
    send_idle_workers_to_mine(base_storage);
    just_make_drones(base_storage);

    if (timer.is_stopped()) {
        std::vector<bool>priority(11);
        std::vector<double>proportions(11);
        proportions[MakeQueue::DRONE] = 0.25;
        proportions[MakeQueue::ZERGLING] = 0.25;
        proportions[MakeQueue::LURKER] = 0.5;
        make_queue.take_order(base_storage, proportions, priority, 30);
        make_queue.temp_print_and_clear_queue();
        /*
        const std::vector<FBase> &self_bases = base_storage.get_self_bases();
        printf("self base ct = %d\n", self_bases.size());
        #ifndef NDEBUG
        for (unsigned int i = 0; i < self_bases.size(); i++) {
            printf("\nbase %d:\n", i);
            const FBase f_base = self_bases[i];
            printf(
                "structures: %d\n",
                f_base->get_structure_ct()
            );
            printf(
                "workers: %d\n",
                f_base->get_worker_ct()
            );
            printf(
                "resource depots: %d\n",
                f_base->get_resource_depot_ct()
            );
            printf(
                "larva: %d\n\n",
                f_base->get_larva_ct()
            );
        }
        #endif
        */
        timer.start(100, 0, false);
    }
}


void FrogFish::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void FrogFish::onReceiveText(Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FrogFish::onPlayerLeft(Player player) {
    Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void FrogFish::onNukeDetect(Position target) {
    if (target) {
        Broodwar << "Nuclear Launch Detected at " << target << std::endl;
    }
    else {
        Broodwar->sendText("Where's the nuke?");
    }
}

void FrogFish::onUnitDiscover(Unit unit) {
    unit_storage.queue_store(unit);
}

void FrogFish::onUnitEvade(Unit unit) {
    // pass
}

void FrogFish::onUnitShow(Unit unit) {
    // pass
}

void FrogFish::onUnitHide(Unit unit) {
    // pass
}

void FrogFish::onUnitCreate(Unit unit) {
    unit_storage.queue_store(unit);
}

void FrogFish::onUnitDestroy(Unit unit) {
    unit_storage.queue_remove(unit);
    // TODO: doesn't catch cases where destroyed out of vision!
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
}

void FrogFish::onUnitMorph(Unit unit) {
    unit_storage.queue_store(unit);
}

void FrogFish::onUnitRenegade(Unit unit) {
    // pass
}

void FrogFish::onSaveGame(std::string gameName) {
    // pass
}

void FrogFish::onUnitComplete(Unit unit) {
    // broadcast for structure usage
}

void FrogFish::onEnd(bool isWinner) {
    FreeConsole();
    unit_storage.free_data();
    base_storage.free_data();
}