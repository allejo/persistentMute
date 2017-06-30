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

#include <map>

#include "bzfsAPI.h"

class PersistentMutes : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
    virtual const char* Name ();
    virtual void Init (const char* config);
    virtual void Cleanup ();
    virtual void Event (bz_EventData* eventData);
    virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

private:
    virtual bool isIpAddressMuted (const char* ipAddress);

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

BZ_PLUGIN(PersistentMutes)

const char* PersistentMutes::Name ()
{
    return "Persistent Mutes";
}

void PersistentMutes::Init (const char* /*config*/)
{
    Register(bz_eAllowPlayer);
    Register(bz_eMuteEvent);

    bz_registerCustomSlashCommand("mutelist", this);
    bz_registerCustomSlashCommand("unmute", this);
}

void PersistentMutes::Cleanup ()
{
    Flush();

    bz_removeCustomSlashCommand("mutelist");
    bz_removeCustomSlashCommand("unmute");
}

void PersistentMutes::Event (bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eAllowPlayer:
        {
            bz_AllowPlayerEventData_V1 *data = (bz_AllowPlayerEventData_V1*)eventData;

            if (isIpAddressMuted(data->ipAddress.c_str()))
            {
                bz_revokePerm(data->playerID, "privatemessage");
                bz_revokePerm(data->playerID, "report");
                bz_revokePerm(data->playerID, "talk");
            }
        }
        break;

        case bz_eMuteEvent:
        {
            bz_MuteEventData_V1 *data = (bz_MuteEventData_V1*)eventData;
            bz_BasePlayerRecord *pr = bz_getPlayerByIndex(data->victimID);

            if (!pr)
            {
                return;
            }

            MuteDefinition _mute;

            _mute.callsign = pr->callsign;
            _mute.bzID = pr->bzID;
            _mute.ipAddress = pr->ipAddress;
            _mute.expiration = bz_getCurrentTime() + (5 * 3600);
            _mute.muter = bz_getPlayerCallsign(data->muterID);

            mutes[pr->ipAddress] = _mute;

            bz_freePlayerRecord(pr);
        }
        break;

        default:
            break;
    }
}

bool PersistentMutes::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (!bz_hasPerm(playerID, "mute"))
    {
        return false;
    }

    if (command == "mutelist")
    {
        bz_sendTextMessage(BZ_SERVER, playerID, " ");
        bz_sendTextMessage(BZ_SERVER, playerID, "Persistent Mute Rules");
        bz_sendTextMessage(BZ_SERVER, playerID, "---------------------");

        for (auto entry : mutes)
        {
            bz_sendTextMessagef(BZ_SERVER, playerID, "%s (%s) muted by %s", entry.second.callsign.c_str(), entry.second.ipAddress.c_str(), entry.second.muter.c_str());
            bz_sendTextMessagef(BZ_SERVER, playerID, "  %.0f minutes remaining", (std::max(0.0, (entry.second.expiration - bz_getCurrentTime())) / 60));
        }

        return false;
    }

    if (params->size() != 1)
    {
        return false;
    }

    bz_BasePlayerRecord *pr = bz_getPlayerBySlotOrCallsign(params->get(0).c_str());

    if (command == "unmute")
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

                    bz_freePlayerRecord(pr);

                    return true;
                }
            }
        }
    }

    bz_freePlayerRecord(pr);
    
    return false;
}

bool PersistentMutes::isIpAddressMuted (const char *ipAddress)
{
    auto entry = mutes.find(ipAddress);

    if (entry != mutes.end())
    {
        if (entry->second.expiration < bz_getCurrentTime())
        {
            mutes.erase(entry);
            return false;
        }

        return true;
    }

    return false;
}
