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
            if (aHeroesBurdenStatus == QUEST_STATUS_REWARDED) {
                LOG_INFO("module", "Peacekeeper :: quest is rewarded, continue");
                ReputationMgr& repMgr = player->GetReputationMgr();

                ReputationRank frenzyheartTribe = player->GetReputationRank(1104);
                LOG_INFO("module", "Peacekeeper :: frenzyheartTribe {}", frenzyheartTribe);
                if (frenzyheartTribe == REP_HATED || frenzyheartTribe == REP_HOSTILE) {
                    const FactionEntry* frenzyheartTribeEntry = sFactionStore.LookupEntry(1104);
                    repMgr.SetOneFactionReputation(frenzyheartTribeEntry, 100000.f, false, REP_HONORED);
                    repMgr.SetOneFactionReputation(frenzyheartTribeEntry, -6999.f, true, REP_HONORED);
                    repMgr.SetAtWar(frenzyheartTribeEntry->ID, false);

                    repMgr.SendState(repMgr.GetState(frenzyheartTribeEntry->ID));
                    LOG_INFO("module", "Peacekeeper :: frenzyheartTribe increased");
                }

                ReputationRank oracles = player->GetReputationRank(1105);
                LOG_INFO("module", "Peacekeeper :: oracles {}", oracles);
                if (oracles == REP_HATED || oracles == REP_HOSTILE) {
                    const FactionEntry* oraclesEntry = sFactionStore.LookupEntry(1105);
                    repMgr.SetOneFactionReputation(oraclesEntry, 100000.f, false, REP_HONORED);
                    repMgr.SetOneFactionReputation(oraclesEntry, -6999.f, true, REP_HONORED);
                    repMgr.SetAtWar(oraclesEntry->ID, false);

                    repMgr.SendState(repMgr.GetState(oraclesEntry->ID));

                    LOG_INFO("module", "Peacekeeper :: oracles increased");
                }
            }
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
