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
        LOG_INFO("module", "Peacekeeper :: player login");
        if (sConfigMgr->GetOption<bool>("Peacekeeper.Enable", false))
        {
            LOG_INFO("module", "Peacekeeper :: enabled");
            QuestStatus aHeroesBurdenStatus = player->GetQuestStatus(12581);
            LOG_INFO("module", "Peacekeeper :: quest status {}", aHeroesBurdenStatus);
            if (aHeroesBurdenStatus == QUEST_STATUS_COMPLETE) {
                ReputationMgr& repMgr = player->GetReputationMgr();

                ReputationRank frenzyheartTribe = player->GetReputationRank(1104);
                LOG_INFO("module", "Peacekeeper :: frenzyheartTribe {}", frenzyheartTribe);
                if (frenzyheartTribe == REP_HATED || frenzyheartTribe == REP_HOSTILE) {
                    repMgr.SetOneFactionReputation(sFactionStore.LookupEntry(1104), 42999.f, false, REP_HONORED);
                    ChatHandler(player->GetSession()).SendSysMessage("Reputation with Frenzyheart Tribe as increased, verify you are no longer At War with them.");
                }

                ReputationRank oracles = player->GetReputationRank(1105);
                LOG_INFO("module", "Peacekeeper :: oracles {}", oracles);
                if (oracles == REP_HATED || oracles == REP_HOSTILE) {
                    repMgr.SetOneFactionReputation(sFactionStore.LookupEntry(1105), 42999.f, false, REP_HONORED);
                    ChatHandler(player->GetSession()).SendSysMessage("Reputation with The Oracles as increased, verify you are no longer At War with them.");
                }
            }
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
