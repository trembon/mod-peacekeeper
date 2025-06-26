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

const int32 AllegianceToTheAldor_QuestID = 10551;
const int32 AllegianceToTheScryers_QuestID = 10552;
const int32 Aldor_FactionID = 932;
const int32 TheScryers_FactionID = 934;

class PeacekeeperPlayer : public PlayerScript, public WorldScript
{
public:
    PeacekeeperPlayer() : PlayerScript("Peacekeeper", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_REPUTATION_CHANGE,
        PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST
        }), WorldScript("Peacekeeper") {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        m_ModuleEnabled = sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false);
        m_AnnounceModuleEnabled = sConfigMgr->GetOption<bool>("Peacekeeper.Announce", false);
        m_PartnerGainReputationEnabled = sConfigMgr->GetOption<bool>("Peacekeeper.PartnerGainReputation", false);
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

            if (HasCompleted_ScryerAldorSelectionQuest(player)) {
                SetAldorScryersBaseReputation(player);
                CompleteAldorAndTheScryerQuests(player);
            }
        }
    }

    bool OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool /*incremental*/) override {
        bool result = true;
        if (m_ModuleEnabled)
        {
            // add lock so we only process one reputation gain per player at the time (example for partner gains)
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

                if ((factionID == Aldor_FactionID || factionID == TheScryers_FactionID) && HasCompleted_ScryerAldorSelectionQuest(player)) {
                    ReputationMgr& repMgr = player->GetReputationMgr();

                    if (factionID == Aldor_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, TheScryers_FactionID, standing);
                    }
                    if (factionID == TheScryers_FactionID) {
                        result = HandleRepuatationGain(repMgr, factionID, Aldor_FactionID, standing);
                    }
                }

                activeHandlers.erase(playerName);
            }
        }

        return result;
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest_id) {
        uint32 qid = quest_id->GetQuestId();
        if (qid == AllegianceToTheAldor_QuestID || qid == AllegianceToTheScryers_QuestID) {
            SetAldorScryersBaseReputation(player);
            CompleteAldorAndTheScryerQuests(player);
        }
    }

private:
    bool m_ModuleEnabled = false;
    bool m_AnnounceModuleEnabled = false;
    bool m_PartnerGainReputationEnabled = false;

    std::map<std::string, uint32> activeHandlers;

    bool HasCompleted_AHeroesBurden(Player* player) {
        QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(AHeroesBurden_QuestID);
        return aHeroesBurdenStatus == QUEST_STATUS_REWARDED;
    }

    bool HasCompleted_ScryerAldorSelectionQuest(Player* player) {
        QuestStatus aldor = player->GetQuestStatus(AllegianceToTheAldor_QuestID);
        QuestStatus scryer = player->GetQuestStatus(AllegianceToTheScryers_QuestID);
        return aldor == QUEST_STATUS_REWARDED || scryer == QUEST_STATUS_REWARDED;
    }

    bool HandleRepuatationGain(ReputationMgr& repMgr, uint32 gainFactionID, uint32 partnerFactionID, int32& newStanding) {
        const FactionEntry* mainEntry = sFactionStore.LookupEntry(gainFactionID);

        // check if reputation is decreasing
        int32 currentRep = repMgr.GetReputation(mainEntry);
        if (currentRep > newStanding) {
            return false;
        }

        // if gain, set partner faction to gain same amount
        if (m_PartnerGainReputationEnabled) {
            const FactionEntry* partnerEntry = sFactionStore.LookupEntry(partnerFactionID);
            repMgr.SetOneFactionReputation(partnerEntry, newStanding, false);
        }

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

    void SetAldorScryersBaseReputation(Player* player) {
        ReputationMgr& repMgr = player->GetReputationMgr();

        const FactionEntry* aldorEntry = sFactionStore.LookupEntry(Aldor_FactionID);
        const FactionEntry* scryerEntry = sFactionStore.LookupEntry(TheScryers_FactionID);

        int32 aldorRep = repMgr.GetReputation(aldorEntry);
        int32 scryerRep = repMgr.GetReputation(scryerEntry);

        if (aldorRep < 0) {
            repMgr.SetOneFactionReputation(aldorEntry, 0, false);
            repMgr.SetAtWar(aldorEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(aldorEntry->reputationListID));
        }

        if (scryerRep < 0) {
            repMgr.SetOneFactionReputation(scryerEntry, 0, false);
            repMgr.SetAtWar(scryerEntry->reputationListID, false);

            repMgr.SendState(repMgr.GetState(scryerEntry->reputationListID));
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

    void CompleteAldorAndTheScryerQuests(Player* player) {
        QuestStatus aldorQuestStatus = player->GetQuestStatus(AllegianceToTheAldor_QuestID);
        if (aldorQuestStatus != QUEST_STATUS_REWARDED) {
            player->AddQuest(sObjectMgr->GetQuestTemplate(AllegianceToTheAldor_QuestID), nullptr);
            player->RewardQuest(sObjectMgr->GetQuestTemplate(AllegianceToTheAldor_QuestID), 0, player, false);
        }

        QuestStatus scryersQuestStatus = player->GetQuestStatus(AllegianceToTheScryers_QuestID);
        if (scryersQuestStatus != QUEST_STATUS_REWARDED) {
            player->AddQuest(sObjectMgr->GetQuestTemplate(AllegianceToTheScryers_QuestID), nullptr);
            player->RewardQuest(sObjectMgr->GetQuestTemplate(AllegianceToTheScryers_QuestID), 0, player, false);
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
