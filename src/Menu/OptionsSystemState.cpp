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

namespace OpenXcom
{

OptionsSystemState::OptionsSystemState(OptionsOrigin origin) : OptionsBaseState(origin)
{
	setCategory(_btnSystem);

	_txtTurningOptions = new Text(104, 9, 94, 8);

	_btnSwipeToTurn = new ToggleTextButton(104, 16, 94, 18);
	_btnHoldToTurn = new ToggleTextButton(104, 16, 94, 36);

	_txtSystemUIControls = new Text(104, 9, 94, 54);
	_cbxSystemUI = new ComboBox(this, 104, 16, 94, 64);

	_txtReconfigureDirs = new Text(104, 9, 94, 82);
	_btnReconfigureDirs = new TextButton(104, 16, 94, 92);

	_btnControls = new TextButton(104, 16, 206, 8);

	add(_txtTurningOptions);
	add(_btnSwipeToTurn);
	add(_btnHoldToTurn);

	add(_txtSystemUIControls);

	add(_txtReconfigureDirs);
	add(_btnReconfigureDirs);

	add(_btnControls);

	// Combobox should be added last, because it will be overlapped by other elements otherwise
	add(_cbxSystemUI);

	centerAllSurfaces();

	_txtTurningOptions->setColor(Palette::blockOffset(8) + 10);
	_txtTurningOptions->setText(tr("STR_TURNING_OPTIONS"));

	_btnSwipeToTurn->setColor(Palette::blockOffset(15) - 1);
	_btnSwipeToTurn->setText(tr("STR_SWIPE"));
	_btnSwipeToTurn->setPressed(Options::swipeToTurn);
	_btnSwipeToTurn->onMouseClick((ActionHandler)&OptionsSystemState::turnSettingChange);

	_btnHoldToTurn->setColor(Palette::blockOffset(15) - 1);
	_btnHoldToTurn->setText(tr("STR_HOLD"));
	_btnHoldToTurn->setPressed(Options::holdToTurn);
	_btnHoldToTurn->onMouseClick((ActionHandler)&OptionsSystemState::turnSettingChange);

	_txtSystemUIControls->setColor(Palette::blockOffset(8) + 10);
	_txtSystemUIControls->setText(tr("STR_SYSTEM_UI"));

	std::vector<std::string> sysUI;
	sysUI.push_back("STR_ALWAYS_SHOWN");
	sysUI.push_back("STR_LOW_PROFILE");
	sysUI.push_back("STR_IMMERSIVE");

	_cbxSystemUI->setColor(Palette::blockOffset(15) - 1);
	_cbxSystemUI->setOptions(sysUI);
	_cbxSystemUI->setSelected((int)Options::systemUI);
	_cbxSystemUI->onChange((ActionHandler)&OptionsSystemState::cbxSystemUIChange);

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
 * Handles changing soldier turning setting.
 * @param action Pointer to the action (not used)
 */
void OptionsSystemState::turnSettingChange(Action *action)
{
	Options::swipeToTurn = _btnSwipeToTurn->getPressed();
	Options::holdToTurn = _btnHoldToTurn->getPressed();
}

/**
 * Handles changing system UI 
 */
void OptionsSystemState::cbxSystemUIChange(Action *action)
{
	Options::systemUI = (SystemUIStyle) _cbxSystemUI->getSelected();
}

void OptionsSystemState::btnReconfigureDirsClick(Action *action)
{
	Options::reload = true;
	CrossPlatform::findDirDialog();
}

void OptionsSystemState::btnControlsClick(Action *action)
{
	_game->popState();
	_game->pushState(new OptionsControlsState(_origin));
}

}