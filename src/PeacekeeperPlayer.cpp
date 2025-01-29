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
                FixOraclesOrFrenzyheartReputation(player, TheOracles_FactionID);
                FixOraclesOrFrenzyheartReputation(player, FrenzyheartTribe_FactionID);
            }
        }
    }

private:
    void FixOraclesOrFrenzyheartReputation(Player* player, uint32 factionId) {
        ReputationMgr& repMgr = player->GetReputationMgr();

        ReputationRank rep_rank = player->GetReputationRank(factionId);
        if (rep_rank == REP_HATED || rep_rank == REP_HOSTILE) {
            const FactionEntry* entry = sFactionStore.LookupEntry(factionId);

            const int32 repToFriendly = repMgr.ReputationRankToStanding(REP_FRIENDLY);
            repMgr.SetOneFactionReputation(entry, repToFriendly + 5001.f, false, REP_HONORED);

            repMgr.SendState(repMgr.GetState(entry->reputationListID));
        }

        if (repMgr.IsAtWar(factionId)) {
            repMgr.SetAtWar(factionId, false);
            repMgr.SendState(repMgr.GetState(factionId));
        }
    }
};

void AddPeacekeeperPlayerScripts()
{
    new PeacekeeperPlayer();
}
