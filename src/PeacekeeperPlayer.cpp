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

                FixOraclesOrFrenzyheartReputation(player, repMgr, TheOracles_FactionID);
                FixOraclesOrFrenzyheartReputation(player, repMgr, FrenzyheartTribe_FactionID);
            }
        }
    }

private:
    void FixOraclesOrFrenzyheartReputation(Player* player, ReputationMgr& repMgr, uint32 factionId) {
        LOG_INFO("module", "Fixing rep :: init {}", factionId);
        bool hasChange = false;

        ReputationRank rep_rank = player->GetReputationRank(factionId);
        LOG_INFO("module", "Fixing rep :: rank {}", rep_rank);
        if (rep_rank == REP_HATED || rep_rank == REP_HOSTILE) {
            const FactionEntry* entry = sFactionStore.LookupEntry(factionId);
            LOG_INFO("module", "Fixing rep :: loaded faction");

            const int32 repToFriendly = repMgr.ReputationRankToStanding(REP_FRIENDLY);
            repMgr.SetOneFactionReputation(entry, repToFriendly + 5001.f, false, REP_HONORED);
            LOG_INFO("module", "Fixing rep :: set reputation");

            hasChange = true;
        }

        LOG_INFO("module", "Fixing rep :: checking war");
        if (repMgr.IsAtWar(factionId)) {
            LOG_INFO("module", "Fixing rep :: is at war");
            repMgr.SetAtWar(factionId, false);
            LOG_INFO("module", "Fixing rep :: no longer at war");
            hasChange = true;
        }

        if (hasChange) {
            LOG_INFO("module", "Fixing rep :: sending state for {}", factionId);
            repMgr.SendState(repMgr.GetState(factionId));
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
