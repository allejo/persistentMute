/*
 Copyright (C) 2017 Vladimir "allejo" Jimenez

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <memory>

#include "bzfsAPI.h"
#include "plugin_utils.h"

class MutePersist : public bz_Plugin, public bz_CustomSlashCommandHandler
{
    virtual const char* Name ();
    virtual void Init (const char* config);
    virtual void Cleanup ();
    virtual void Event (bz_EventData* eventData);
    virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

    struct MuteDefinition
    {
        std::string callsign;
        std::string bzID;
        std::string ipAddress;
        double expiration;

        std::string muter;
    };

    typedef std::map<std::string, MuteDefinition> MuteMap;

    MuteMap mutes;
};

BZ_PLUGIN(MutePersist)

const char* MutePersist::Name ()
{
    return "Persistent Mutes";
}

void MutePersist::Init (const char* /*config*/)
{
    Register(bz_ePlayerJoinEvent);

    bz_registerCustomSlashCommand("mute", this);
    bz_registerCustomSlashCommand("mutelist", this);
    bz_registerCustomSlashCommand("unmute", this);
}

void MutePersist::Cleanup ()
{
    Flush();

    bz_removeCustomSlashCommand("mute");
    bz_removeCustomSlashCommand("mutelist");
    bz_removeCustomSlashCommand("unmute");
}

void MutePersist::Event (bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerJoinEvent:
        {
            bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
            MuteMap::iterator entry = mutes.find(joinData->record->ipAddress);

            if (entry != mutes.end())
            {
                if (entry->second.expiration < bz_getCurrentTime())
                {
                    mutes.erase(entry);
                }
                else
                {
                    bz_revokePerm(joinData->playerID, "privatemessage");
                    bz_revokePerm(joinData->playerID, "report");
                    bz_revokePerm(joinData->playerID, "talk");
                }
            }
        }
        break;

        default: break;
    }
}

bool MutePersist::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (!bz_hasPerm(playerID, "mute"))
    {
        return false;
    }

    if (command == "mutelist")
    {
        bz_sendTextMessage(BZ_SERVER, playerID, "Persistent Mute Rules");
        bz_sendTextMessage(BZ_SERVER, playerID, "---------------------");

        for (auto entry : mutes)
        {
            bz_sendTextMessagef(BZ_SERVER, playerID, "%s (%s) muted by %s", entry.second.callsign.c_str(), entry.second.ipAddress.c_str(), entry.second.muter.c_str());
            bz_sendTextMessagef(BZ_SERVER, playerID, "  %.0f minutes remaining", (std::max(0.0, (entry.second.expiration - bz_getCurrentTime())) / 60));
        }

        bz_sendTextMessagef(BZ_SERVER, playerID, "");

        return false;
    }

    if (params->size() != 1)
    {
        return false;
    }

    std::unique_ptr<bz_BasePlayerRecord> pr(bz_getPlayerBySlotOrCallsign(params->get(0).c_str()));

    if (command == "mute" && pr)
    {
        MuteDefinition _mute;

        _mute.callsign = pr->callsign;
        _mute.bzID = pr->bzID;
        _mute.ipAddress = pr->ipAddress;
        _mute.expiration = bz_getCurrentTime() + (5 * 3600);
        _mute.muter = bz_getPlayerCallsign(playerID);

        mutes[pr->ipAddress] = _mute;
    }
    else if (command == "unmute")
    {
        if (pr && mutes.find(pr->ipAddress) != mutes.end())
        {
            mutes.erase(pr->ipAddress);
        }
        else
        {
            std::string target = params->get(0);

            for (auto entry : mutes)
            {
                if (entry.second.callsign == target || entry.second.ipAddress == target)
                {
                    mutes.erase(entry.second.ipAddress);
                    bz_sendTextMessagef(BZ_SERVER, playerID, "%s has been removed from the mute list", entry.second.callsign.c_str());
                    return true;
                }
            }
        }
    }
    
    return false;
}
