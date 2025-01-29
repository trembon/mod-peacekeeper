#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ReputationMgr.h"

class PeacekeeperPlayer : public PlayerScript
{
public:
    PeacekeeperPlayer() : PlayerScript("Peacekeeper") {}

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false))
        {
            QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(12581);
            if (aHeroesBurdenStatus == QUEST_STATUS_REWARDED) {
                ReputationMgr& repMgr = player->GetReputationMgr();

                const int32 repToFriendly = repMgr.ReputationRankToStanding(REP_FRIENDLY);

                ReputationRank frenzyheartTribe = player->GetReputationRank(1104);
                if (frenzyheartTribe == REP_HATED || frenzyheartTribe == REP_HOSTILE) {
                    const FactionEntry* frenzyheartTribeEntry = sFactionStore.LookupEntry(1104);

                    repMgr.SetOneFactionReputation(frenzyheartTribeEntry, repToFriendly + 5001.f, false, REP_HONORED);
                    repMgr.SetAtWar(frenzyheartTribeEntry->reputationListID, false);

                    repMgr.SendState(repMgr.GetState(frenzyheartTribeEntry->reputationListID));
                }

                ReputationRank oracles = player->GetReputationRank(1105);
                if (oracles == REP_HATED || oracles == REP_HOSTILE) {
                    const FactionEntry* oraclesEntry = sFactionStore.LookupEntry(1105);

                    repMgr.SetOneFactionReputation(oraclesEntry, repToFriendly + 5001.f, false, REP_HONORED);
                    repMgr.SetAtWar(oraclesEntry->reputationListID, false);

                    repMgr.SendState(repMgr.GetState(oraclesEntry->reputationListID));
                }
            }
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
