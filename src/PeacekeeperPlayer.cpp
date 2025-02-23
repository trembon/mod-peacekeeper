#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ReputationMgr.h"

const int32 AHeroesBurden_QuestID = 12581;
const int32 FrenzyheartTribe_FactionID = 1104;
const int32 FrenzyheartTribe_PreQuestID = 12692;
const int32 TheOracles_FactionID = 1105;
const int32 TheOracles_PreQuestID = 12695;

class PeacekeeperPlayer : public PlayerScript, public WorldScript
{
public:
    PeacekeeperPlayer() : PlayerScript("Peacekeeper", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_REPUTATION_CHANGE
        }), WorldScript("Peacekeeper") {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        m_ModuleEnabled = sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false);
        m_AnnounceModuleEnabled = sConfigMgr->GetOption<bool>("Peacekeeper.Announce", false);
    }

    void OnPlayerLogin(Player* player) override
    {
        if (m_ModuleEnabled)
        {
            if (m_AnnounceModuleEnabled)
            {
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Peacekeeper |rmodule.");
            }

            if (HasCompleted_AHeroesBurden(player)) {
                CompleteRequiredOracleAndFrenzyheartPreQuests(player);
                SyncOraclesAndFrenzyheartReputation(player);
            }
        }
    }

    bool OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool /*incremental*/) override {
        bool result = true;
        if (m_ModuleEnabled)
        {
            // what to do when faction is affected by peacekeeper?
            // dont allow decrease of reputation
            // if one faction gains rep, the other one gains as well
            const std::string playerName = player->GetName();
            if (activeHandlers[playerName] == 0) {
                activeHandlers[playerName] = factionID;

                // handle faction The Oracles and Frenzyheart Tribe, as they should only be handled after the quest has been completed
                if ((factionID == FrenzyheartTribe_FactionID || factionID == TheOracles_FactionID) && HasCompleted_AHeroesBurden(player)) {
                    ReputationMgr& repMgr = player->GetReputationMgr();

                    if (factionID == FrenzyheartTribe_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, TheOracles_FactionID, standing);
                    }
                    if (factionID == TheOracles_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, FrenzyheartTribe_FactionID, standing);
                    }
                }

                activeHandlers.erase(playerName);
            }
        }

        return result;
    }

private:
    bool m_ModuleEnabled = false;
    bool m_AnnounceModuleEnabled = false;

    std::map<std::string, uint32> activeHandlers;

    bool HasCompleted_AHeroesBurden(Player* player) {
        QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(AHeroesBurden_QuestID);
        return aHeroesBurdenStatus == QUEST_STATUS_REWARDED;
    }

    bool HandleRepuatationGain(ReputationMgr& repMgr, uint32 gainFactionID, uint32 partnerFactionID, int32& newStanding) {
        const FactionEntry* mainEntry = sFactionStore.LookupEntry(gainFactionID);

        // check if reputation is decreasing
        int32 currentRep = repMgr.GetReputation(mainEntry);
        if (currentRep > newStanding) {
            return false;
        }

        // if gain, set partner faction to gain same amount
        const FactionEntry* partnerEntry = sFactionStore.LookupEntry(partnerFactionID);
        repMgr.SetOneFactionReputation(partnerEntry, newStanding, false);

        return true;
    }

    void SyncOraclesAndFrenzyheartReputation(Player* player) {
        ReputationMgr& repMgr = player->GetReputationMgr();

        const FactionEntry* frenzyheartTribeEntry = sFactionStore.LookupEntry(FrenzyheartTribe_FactionID);
        const FactionEntry* oraclesEntry = sFactionStore.LookupEntry(TheOracles_FactionID);

        int32 frenzyheartTribeRep = repMgr.GetReputation(frenzyheartTribeEntry);
        int32 oraclesRep = repMgr.GetReputation(oraclesEntry);

        if (frenzyheartTribeRep > oraclesRep) {
            repMgr.SetOneFactionReputation(oraclesEntry, frenzyheartTribeRep, false);
            repMgr.SetAtWar(oraclesEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(oraclesEntry->reputationListID));
        }
        if (oraclesRep > frenzyheartTribeRep) {
            repMgr.SetOneFactionReputation(frenzyheartTribeEntry, oraclesRep, false);
            repMgr.SetAtWar(frenzyheartTribeEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(frenzyheartTribeEntry->reputationListID));
        }
    }

    void CompleteRequiredOracleAndFrenzyheartPreQuests(Player* player) {
        QuestStatus frenzyheartPreQuestStatus = player->GetQuestStatus(FrenzyheartTribe_PreQuestID);
        if (frenzyheartPreQuestStatus != QUEST_STATUS_REWARDED) {
            player->AddQuest(sObjectMgr->GetQuestTemplate(FrenzyheartTribe_PreQuestID), nullptr);
            player->RewardQuest(sObjectMgr->GetQuestTemplate(FrenzyheartTribe_PreQuestID), 0, player, false);
        }

        QuestStatus oraclesPreQuestStatus = player->GetQuestStatus(TheOracles_PreQuestID);
        if (oraclesPreQuestStatus != QUEST_STATUS_REWARDED) {
            player->AddQuest(sObjectMgr->GetQuestTemplate(TheOracles_PreQuestID), nullptr);
            player->RewardQuest(sObjectMgr->GetQuestTemplate(TheOracles_PreQuestID), 0, player, false);
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
