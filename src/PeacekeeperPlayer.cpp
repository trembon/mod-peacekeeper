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
                ReputationMgr& repMgr = player->GetReputationMgr();
                FixOraclesAndFrenzyheartReputation(player, repMgr);
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

    void FixOraclesAndFrenzyheartReputation(Player* player, ReputationMgr& repMgr) {
        const int32 repToFriendly = repMgr.ReputationRankToStanding(REP_FRIENDLY);

        ReputationRank frenzyheartTribe = player->GetReputationRank(FrenzyheartTribe_FactionID);
        if (frenzyheartTribe == REP_HATED || frenzyheartTribe == REP_HOSTILE) {
            const FactionEntry* frenzyheartTribeEntry = sFactionStore.LookupEntry(FrenzyheartTribe_FactionID);

            repMgr.SetOneFactionReputation(frenzyheartTribeEntry, repToFriendly + 5001.f, false, REP_HONORED);
            repMgr.SetAtWar(frenzyheartTribeEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(frenzyheartTribeEntry->reputationListID));
        }

        ReputationRank oracles = player->GetReputationRank(TheOracles_FactionID);
        if (oracles == REP_HATED || oracles == REP_HOSTILE) {
            const FactionEntry* oraclesEntry = sFactionStore.LookupEntry(TheOracles_FactionID);
            repMgr.SetOneFactionReputation(oraclesEntry, repToFriendly + 5001.f, false, REP_HONORED);
            repMgr.SetAtWar(oraclesEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(oraclesEntry->reputationListID));
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
