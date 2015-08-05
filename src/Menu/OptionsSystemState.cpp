#include "OptionsSystemState.h"
#include "../Engine/Game.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/ComboBox.h"
#include "../Engine/Palette.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/CrossPlatform.h"
#include "OptionsControlsState.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

OptionsSystemState::OptionsSystemState(OptionsOrigin origin) : OptionsBaseState(origin)
{
	setCategory(_btnSystem);

	_txtSystemUIControls = new Text(104, 9, 94, 8);
	_cbxSystemUI = new ComboBox(this, 104, 16, 94, 18);

	_txtReconfigureDirs = new Text(104, 9, 206, 38);
	_btnReconfigureDirs = new TextButton(104, 16, 206, 48);

	_txtControls = new Text(104, 9, 206, 8);
	_btnControls = new TextButton(104, 16, 206, 18);

	_txtLogDestination = new Text(104, 9, 94, 38);
	_btnLogFile = new ToggleTextButton(50, 16, 94, 48);
	_btnLogSystem = new ToggleTextButton(50, 16, 94 + 50 + 4, 48);

	_txtDebugOptions = new Text(104, 9, 94, 66);
	_btnLogTouchEvents = new ToggleTextButton(104, 16, 94, 76);
	_btnFakeEvents = new ToggleTextButton(104, 16, 94, 94);
	_btnForceGLMode = new ToggleTextButton(104, 16, 94, 112);
        
    _txtMouseMode = new Text(104, 9, 206, 66);
    _cbxMouseMode = new ComboBox(this, 104, 16, 206, 76);

	add(_txtSystemUIControls);

	add(_txtReconfigureDirs);
	add(_btnReconfigureDirs);

	add(_txtControls);
	add(_btnControls);

	add(_txtLogDestination);
	add(_btnLogFile);
	add(_btnLogSystem);

	add(_txtDebugOptions);
	add(_btnLogTouchEvents);
	add(_btnFakeEvents);
	add(_btnForceGLMode);
        
    add(_txtMouseMode);

	// Combobox should be added last, because it will be overlapped by other elements otherwise
	add(_cbxSystemUI);
    add(_cbxMouseMode);

	centerAllSurfaces();

	_txtLogDestination->setColor(Palette::blockOffset(8) + 10);
	_txtLogDestination->setText(tr("STR_LOG_DESTINATION"));

	_btnLogFile->setColor(Palette::blockOffset(15) - 1);
	_btnLogFile->setText(tr("STR_LOG_FILE"));
	_btnLogFile->setPressed(Options::logToFile);
	_btnLogFile->onMouseClick((ActionHandler)&OptionsSystemState::btnLogClick);
	_btnLogFile->setTooltip("STR_LOG_FILE_DESC");
	_btnLogFile->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_btnLogFile->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_btnLogSystem->setColor(Palette::blockOffset(15) - 1);
	_btnLogSystem->setText(tr("STR_LOG_SYSTEM"));
	_btnLogSystem->setPressed(Options::logToSystem);
	_btnLogSystem->onMouseClick((ActionHandler)&OptionsSystemState::btnLogClick);
	_btnLogSystem->setTooltip("STR_LOG_SYSTEM_DESC");
	_btnLogSystem->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_btnLogSystem->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_txtDebugOptions->setColor(Palette::blockOffset(8) + 10);
	_txtDebugOptions->setText(tr("STR_DEBUG_OPTIONS"));

	_btnLogTouchEvents->setColor(Palette::blockOffset(15) - 1);
	_btnLogTouchEvents->setText(tr("STR_LOG_TOUCH_EVENTS"));
	_btnLogTouchEvents->setPressed(Options::logTouch);
	_btnLogTouchEvents->onMouseClick((ActionHandler)&OptionsSystemState::btnLogTouchEventsClick);
	_btnLogTouchEvents->setTooltip("STR_LOG_TOUCH_EVENTS_DESC");
	_btnLogTouchEvents->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_btnLogTouchEvents->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_btnFakeEvents->setColor(Palette::blockOffset(15) - 1);
	_btnFakeEvents->setText(tr("STR_FAKE_EVENTS"));
	_btnFakeEvents->setPressed(Options::fakeEvents);
	_btnFakeEvents->onMouseClick((ActionHandler)&OptionsSystemState::btnFakeEventsClick);
	_btnFakeEvents->setTooltip("STR_FAKE_EVENTS_DESC");
	_btnFakeEvents->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_btnFakeEvents->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_btnForceGLMode->setColor(Palette::blockOffset(15) - 1);
	_btnForceGLMode->setText(tr("STR_FORCE_GL_MODE"));
	_btnForceGLMode->setPressed(Options::forceGLMode);
	_btnForceGLMode->onMouseClick((ActionHandler)&OptionsSystemState::btnForceGLModeClick);
	_btnForceGLMode->setTooltip("STR_FORCE_GL_MODE_DESC");
	_btnForceGLMode->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_btnForceGLMode->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_txtSystemUIControls->setColor(Palette::blockOffset(8) + 10);
	_txtSystemUIControls->setText(tr("STR_SYSTEM_UI"));

	_txtControls->setColor(Palette::blockOffset(8) + 10);
	_txtControls->setText(tr("STR_CONTROLS_LABEL"));

	std::vector<std::string> sysUI;
	int sysVersion = CrossPlatform::getSystemVersion();
	sysUI.push_back("STR_ALWAYS_SHOWN");
	if (sysVersion >= 11)
	{
		sysUI.push_back("STR_LOW_PROFILE");
	}
	if (sysVersion >= 19)
	{
		sysUI.push_back("STR_IMMERSIVE");
	}
	
	std::vector<std::string> mouseMode;
        mouseMode.push_back("STR_MOUSE_BASIC");
        if (sysVersion >= 14)
        {
                mouseMode.push_back("STR_MOUSE_EXTENDED");
        }

	_cbxSystemUI->setColor(Palette::blockOffset(15) - 1);
	_cbxSystemUI->setOptions(sysUI);
	_cbxSystemUI->setSelected((int)Options::systemUI);
	_cbxSystemUI->onChange((ActionHandler)&OptionsSystemState::cbxSystemUIChange);
	_cbxSystemUI->setTooltip("STR_SYSTEM_UI_DESC");
	_cbxSystemUI->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
	_cbxSystemUI->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);
        
        
    _txtMouseMode->setColor(Palette::blockOffset(8) + 10);
    _txtMouseMode->setText(tr("STR_MOUSE_MODE"));
        
    _cbxMouseMode->setColor(Palette::blockOffset(15) - 1);
    _cbxMouseMode->setOptions(mouseMode);
    _cbxMouseMode->setSelected((int)Options::mouseMode);
    _cbxMouseMode->onChange((ActionHandler)&OptionsSystemState::cbxMouseModeChange);
    _cbxMouseMode->setTooltip("STR_MOUSE_MODE_DESC");
    _cbxMouseMode->onMouseIn((ActionHandler)&OptionsSystemState::txtTooltipIn);
    _cbxMouseMode->onMouseOut((ActionHandler)&OptionsSystemState::txtTooltipOut);

	_txtReconfigureDirs->setColor(Palette::blockOffset(8) + 10);
	_txtReconfigureDirs->setText(tr("STR_RECONFIGURE_DIRS"));

	_btnReconfigureDirs->setColor(Palette::blockOffset(15) - 1);
	_btnReconfigureDirs->setText(tr("STR_RECONFIGURE_DIRS_BTN"));
	_btnReconfigureDirs->onMouseClick((ActionHandler)&OptionsSystemState::btnReconfigureDirsClick);

	_btnControls->setColor(Palette::blockOffset(15) - 1);
	_btnControls->setText(tr("STR_CONTROLS"));
	_btnControls->onMouseClick((ActionHandler)&OptionsSystemState::btnControlsClick);

	// This option requires restart, so it's disabled in-game.
	_txtReconfigureDirs->setVisible(_origin == OPT_MENU);
	_btnReconfigureDirs->setVisible(_origin == OPT_MENU);
	
}


OptionsSystemState::~OptionsSystemState(void)
{
}

/**
 * Handles changing system UI on Android.
 */
void OptionsSystemState::cbxSystemUIChange(Action *action)
{
	Options::systemUI = (SystemUIStyle) _cbxSystemUI->getSelected();
}

/**
 * Handles resetting the game's directories.
 */
void OptionsSystemState::btnReconfigureDirsClick(Action *action)
{
	Options::reload = true;
	CrossPlatform::findDirDialog();
}

/**
 * Handles transitioning to the Controls state.
 */
void OptionsSystemState::btnControlsClick(Action *action)
{
	_game->popState();
	_game->pushState(new OptionsControlsState(_origin));
}

void OptionsSystemState::btnLogClick(Action *action)
{
	InteractiveSurface *sender = action->getSender();
	if (sender == _btnLogFile)
	{
		// Make sure there's a file to write to.
		std::string s = Options::getUserFolder();
		s += "openxcom.log";
		Logger::logFile() = s;
		Logger::logToFile() = Options::logToFile = _btnLogFile->getPressed();
	}
	if (sender == _btnLogSystem)
	{
		Logger::logToSystem() = Options::logToSystem = _btnLogSystem->getPressed();
	}
}

void OptionsSystemState::btnLogTouchEventsClick(Action *action)
{
	Options::logTouch = _btnLogTouchEvents->getPressed();
}

void OptionsSystemState::btnFakeEventsClick(Action *action)
{
	Options::fakeEvents = _btnFakeEvents->getPressed();
}

void OptionsSystemState::btnForceGLModeClick(Action *action)
{
	Options::forceGLMode = _btnForceGLMode->getPressed();
}

void OptionsSystemState::cbxMouseModeChange(Action *action)
{
    Options::mouseMode = _cbxMouseMode->getSelected();
#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH, Options::mouseMode == 0 ? "0" : "1");
#endif
}

}
