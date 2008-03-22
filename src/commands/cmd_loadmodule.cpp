/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2008 InspIRCd Development Team
 * See: http://www.inspircd.org/wiki/index.php/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"
#include "commands/cmd_loadmodule.h"

extern "C" DllExport Command* init_command(InspIRCd* Instance)
{
	return new CommandLoadmodule(Instance);
}

/** Handle /LOADMODULE
 */
CmdResult CommandLoadmodule::Handle (const char* const* parameters, int, User *user)
{
	if (ServerInstance->Modules->Load(parameters[0]))
	{
		ServerInstance->SNO->WriteToSnoMask('A', "NEW MODULE: %s loaded %s",user->nick, parameters[0]);
		user->WriteNumeric(975, "%s %s :Module successfully loaded.",user->nick, parameters[0]);
		return CMD_SUCCESS;
	}
	else
	{
		user->WriteNumeric(974, "%s %s :%s",user->nick, parameters[0], ServerInstance->Modules->LastError().c_str());
		return CMD_FAILURE;
	}
}
