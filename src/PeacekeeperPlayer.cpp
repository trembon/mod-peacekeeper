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
    PeacekeeperPlayer() : PlayerScript("Peacekeeper") {}

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false))
        {
            if (sConfigMgr->GetOption<bool>("Peacekeeper.Announce", true))
            {
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Peacekeeper |rmodule.");
            }

            QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(AHeroesBurden_QuestID);
            if (aHeroesBurdenStatus == QUEST_STATUS_REWARDED) {
                ReputationMgr& repMgr = player->GetReputationMgr();
                FixOraclesAndFrenzyheartReputation(player, repMgr);
            }
        }
    }

private:
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
