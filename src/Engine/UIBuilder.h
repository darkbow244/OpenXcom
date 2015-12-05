/*
 * Externalized creation of UI elements (BATTLESCAPE ONLY!)
 *
 */


#if 0

#ifndef OPENXCOM_UIBUILDER_H
#define OPENXCOM_UIBUILDER_H

#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include "State.h"
#include "InteractiveSurface.h"

namespace OpenXcom
{

namespace UIBuilder
{
	std::map<std::string, Surface*> buildUI(State *currentState,
				ResourcePack* resourcePack,
				std::map<std::string, ActionHandler> &handlers,
				std::map<std::string, SDL_Keycode> &kbShortcuts, 
				const std::string &file);

}

}

#endif //OPENXCOM_UIBUILDER_H

#endif