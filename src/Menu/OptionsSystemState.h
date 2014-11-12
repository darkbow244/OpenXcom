

#ifndef OPENXCOM_OPTIONSSYSTEMSTATE_H
#define OPENXCOM_OPTIONSSYSTEMSTATE_H

#include "OptionsBaseState.h"

namespace OpenXcom 
{

class ComboBox;
class Text;
class TextButton;
class ToggleTextButton;

/**
 * System-specific options menu (used in place of "Controls" menu)
 */
class OptionsSystemState : public OptionsBaseState
{
private:
	Text *_txtTurningOptions;
	Text *_txtSystemUIControls;
	Text *_txtReconfigureDirs;
	ToggleTextButton *_btnSwipeToTurn;
	ToggleTextButton *_btnHoldToTurn;
	ComboBox *_cbxSystemUI;
	TextButton *_btnReconfigureDirs;
public:
	OptionsSystemState(OptionsOrigin origin);
	~OptionsSystemState();
	void turnSettingChange(Action *action);
	void cbxSystemUIChange(Action *action);
	void btnReconfigureDirsClick(Action *action);
	void btnControlsClick(Action *action);
};

}
#endif /*OPENXCOM_OPTIONSSYSTEMSATE_H*/