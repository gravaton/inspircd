
/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"
#include "configreader.h"
#include "mode.h"
#include "channels.h"
#include "users.h"
#include "modules.h"
#include "modes/cmode_v.h"

ModeChannelVoice::ModeChannelVoice(InspIRCd* Instance) : ModeHandler(Instance, NULL, 'v', 1, 1, true, MODETYPE_CHANNEL, false, '+', '%', TR_NICK)
{
}

unsigned int ModeChannelVoice::GetPrefixRank()
{
	return VOICE_VALUE;
}

ModePair ModeChannelVoice::ModeSet(User*, User*, Channel* channel, const std::string &parameter)
{
	User* x = ServerInstance->FindNick(parameter);
	if (x)
	{
		Membership* memb = channel->GetUser(x);
		if (memb && memb->hasMode('v'))
		{
			return std::make_pair(true, x->nick);
		}
		else
		{
			return std::make_pair(false, parameter);
		}
	}
	return std::make_pair(false, parameter);
}

void ModeChannelVoice::RemoveMode(Channel* channel, irc::modestacker* stack)
{
	const UserMembList* clist = channel->GetUsers();

	for (UserMembCIter i = clist->begin(); i != clist->end(); i++)
	{
		if (stack)
			stack->Push(this->GetModeChar(), i->first->nick);
		else
		{
			std::vector<std::string> parameters;
			parameters.push_back(channel->name);
			parameters.push_back("-v");
			parameters.push_back(i->first->nick);
			ServerInstance->SendMode(parameters, ServerInstance->FakeClient);
		}
	}
}

void ModeChannelVoice::RemoveMode(User*, irc::modestacker* stack)
{
}

ModeAction ModeChannelVoice::OnModeChange(User* source, User*, Channel* channel, std::string &parameter, bool adding)
{
	int status = channel->GetPrefixValue(source);

	/* Call the correct method depending on wether we're adding or removing the mode */
	if (adding)
	{
		parameter = this->AddVoice(source, parameter.c_str(), channel, status);
	}
	else
	{
		parameter = this->DelVoice(source, parameter.c_str(), channel, status);
	}
	/* If the method above 'ate' the parameter by reducing it to an empty string, then
	 * it won't matter wether we return ALLOW or DENY here, as an empty string overrides
	 * the return value and is always MODEACTION_DENY if the mode is supposed to have
	 * a parameter.
	 */
	if (parameter.length())
		return MODEACTION_ALLOW;
	else
		return MODEACTION_DENY;
}

std::string ModeChannelVoice::AddVoice(User *user,const char* dest,Channel *chan,int status)
{
	User *d = ServerInstance->Modes->SanityChecks(user,dest,chan,status);

	if (d)
	{
		if (IS_LOCAL(user))
		{
			ModResult MOD_RESULT;
			FIRST_MOD_RESULT(ServerInstance, OnAccessCheck, MOD_RESULT, (user,d,chan,AC_VOICE));

			if (MOD_RESULT == MOD_RES_DENY)
				return "";
			if (MOD_RESULT == MOD_RES_PASSTHRU)
			{
				if ((status < HALFOP_VALUE) && (!ServerInstance->ULine(user->server)))
				{
					user->WriteServ("482 %s %s :You're not a channel (half)operator",user->nick.c_str(), chan->name.c_str());
					return "";
				}
			}
		}

		return d->nick;
	}
	return "";
}

std::string ModeChannelVoice::DelVoice(User *user,const char *dest,Channel *chan,int status)
{
	User *d = ServerInstance->Modes->SanityChecks(user,dest,chan,status);

	if (d)
	{
		if (IS_LOCAL(user))
		{
			ModResult MOD_RESULT;
			FIRST_MOD_RESULT(ServerInstance, OnAccessCheck, MOD_RESULT, (user,d,chan,AC_DEVOICE));

			if (MOD_RESULT == MOD_RES_DENY)
				return "";
			if (MOD_RESULT == MOD_RES_PASSTHRU)
			{
				if ((status < HALFOP_VALUE) && (!ServerInstance->ULine(user->server)))
				{
					user->WriteServ("482 %s %s :You are not a channel (half)operator",user->nick.c_str(), chan->name.c_str());
					return "";
				}
			}
		}

		return d->nick;
	}
	return "";
}
