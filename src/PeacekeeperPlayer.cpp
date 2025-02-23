#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ReputationMgr.h"

const int32 AHeroesBurden_QuestID = 12581;
const int32 FrenzyheartTribe_FactionID = 1104;
const int32 TheOracles_FactionID = 1105;

class PeacekeeperPlayer : public PlayerScript
{
public:
    PeacekeeperPlayer() : PlayerScript("Peacekeeper", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_REPUTATION_CHANGE
        }) {
    }

    void OnPlayerLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false))
        {
            if (sConfigMgr->GetOption<bool>("Peacekeeper.Announce", true))
            {
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Peacekeeper |rmodule.");
            }

            if (HasCompleted_AHeroesBurden(player)) {
                SyncOraclesAndFrenzyheartReputation(player);
            }
        }
    }

    bool OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental) override {
        bool result = true;
        if (sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false))
        {
            // what to do when faction is affected by peacekeeper?
            // dont allow decrease of reputation
            // if one faction gains rep, the other one gains as well
            ObjectGuid playerID = player->GetGUID();
            LOG_INFO("module", "rep gain :: player {}, faction {}, standing {}", playerID, factionID, standing);
            if (activeHandlers[playerID] == 0) {
                activeHandlers[playerID] = factionID;
                LOG_INFO("module", "rep gain :: can handle player {}", playerID);

                // handle faction The Oracles and Frenzyheart Tribe, as they should only be handled after the quest has been completed
                if ((factionID == FrenzyheartTribe_FactionID || factionID == TheOracles_FactionID) && HasCompleted_AHeroesBurden(player)) {
                    ReputationMgr& repMgr = player->GetReputationMgr();
                    LOG_INFO("module", "rep gain :: is oracle/frenzy");

                    if (factionID == FrenzyheartTribe_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, TheOracles_FactionID, standing);
                    }
                    if (factionID == TheOracles_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, FrenzyheartTribe_FactionID, standing);
                    }
                }

                activeHandlers.erase(playerID);
            }
        }

        return result;
    }

private:
    std::map<ObjectGuid, uint32> activeHandlers;

    bool HasCompleted_AHeroesBurden(Player* player) {
        QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(AHeroesBurden_QuestID);
        return aHeroesBurdenStatus == QUEST_STATUS_REWARDED;
    }

    bool HandleRepuatationGain(ReputationMgr& repMgr, uint32 gainFactionID, uint32 partnerFactionID, int32& newStanding) {
        const FactionEntry* mainEntry = sFactionStore.LookupEntry(gainFactionID);

        // check if reputation is decreasing
        int32 currentRep = repMgr.GetReputation(mainEntry);
        if (currentRep > newStanding) {
            LOG_INFO("module", "rep gain :: cant loose rep to {}", gainFactionID);
            return false;
        }

        // if gain, set partner faction to gain same amount
        const FactionEntry* partnerEntry = sFactionStore.LookupEntry(partnerFactionID);
        LOG_INFO("module", "rep gain :: updating partner entry {}", partnerFactionID);
        repMgr.SetOneFactionReputation(partnerEntry, newStanding, false);

        return true;
    }

    void SyncOraclesAndFrenzyheartReputation(Player* player) {
        ReputationMgr& repMgr = player->GetReputationMgr();

        const FactionEntry* frenzyheartTribeEntry = sFactionStore.LookupEntry(FrenzyheartTribe_FactionID);
        const FactionEntry* oraclesEntry = sFactionStore.LookupEntry(TheOracles_FactionID);

        int32 frenzyheartTribeRep = repMgr.GetReputation(frenzyheartTribeEntry);
        int32 oraclesRep = repMgr.GetReputation(oraclesEntry);

        LOG_INFO("module", "sync rep :: player {}, frenzy {}, oracles {}", playerID, frenzyheartTribeRep, oraclesRep);

        if (frenzyheartTribeRep > oraclesRep) {
            LOG_INFO("module", "sync rep :: setting oracles rep to {}", frenzyheartTribeRep);
            repMgr.SetOneFactionReputation(oraclesEntry, frenzyheartTribeRep, false);
            repMgr.SetAtWar(oraclesEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(oraclesEntry->reputationListID));
        }
        if (oraclesRep > frenzyheartTribeRep) {
            LOG_INFO("module", "sync rep :: setting frenzy rep to {}", oraclesRep);
            repMgr.SetOneFactionReputation(frenzyheartTribeEntry, oraclesRep, false);
            repMgr.SetAtWar(frenzyheartTribeEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(frenzyheartTribeEntry->reputationListID));
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
