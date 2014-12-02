

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
	//Text *_txtTurningOptions;
	Text *_txtSystemUIControls;
	Text *_txtReconfigureDirs;
	Text *_txtControls;
	//ToggleTextButton *_btnSwipeToTurn;
	//ToggleTextButton *_btnHoldToTurn;
	Text *_txtLogDestination;
	Text *_txtDebugOptions;
	ToggleTextButton *_btnLogFile;
	ToggleTextButton *_btnLogSystem;
	ToggleTextButton *_btnLogTouchEvents;
	ToggleTextButton *_btnFakeEvents;
	ToggleTextButton *_btnForceGLMode;
	ComboBox *_cbxSystemUI;
	TextButton *_btnReconfigureDirs;
public:
	OptionsSystemState(OptionsOrigin origin);
	~OptionsSystemState();
	void cbxSystemUIChange(Action *action);
	void btnReconfigureDirsClick(Action *action);
	void btnControlsClick(Action *action);
	void btnLogClick(Action *action);
	void btnLogTouchEventsClick(Action *action);
	void btnFakeEventsClick(Action *action);
	void btnForceGLModeClick(Action *action);
};

}
#endif /*OPENXCOM_OPTIONSSYSTEMSATE_H*/