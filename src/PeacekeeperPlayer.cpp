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
            if (aHeroesBurdenStatus == QUEST_STATUS_COMPLETE) {
                ReputationMgr& repMgr = player->GetReputationMgr();

                ReputationRank frenzyheartTribe = player->GetReputationRank(1104);
                if (frenzyheartTribe == REP_HATED || frenzyheartTribe == REP_HOSTILE) {
                    repMgr.SetOneFactionReputation(sFactionStore.LookupEntry(1104), 42999.f, false, REP_HONORED);
                }

                ReputationRank oracles = player->GetReputationRank(1105);
                if (oracles == REP_HATED || oracles == REP_HOSTILE) {
                    repMgr.SetOneFactionReputation(sFactionStore.LookupEntry(1105), 42999.f, false, REP_HONORED);
                }
            }
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
