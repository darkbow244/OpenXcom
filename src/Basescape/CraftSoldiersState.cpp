/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "CraftSoldiersState.h"
#include <climits>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Craft.h"
#include "SoldierInfoState.h"
#include "../Mod/RuleInterface.h"

#include "../Engine/Timer.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Soldiers screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param craft ID of the selected craft.
 */
CraftSoldiersState::CraftSoldiersState(Base *base, size_t craft) : _base(base), _craft(craft), _otherCraftColor(0), _pselSoldier(-1), _wasDragging(false)
#ifdef __MOBILE__
	, _clickGuard(false)
#endif
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(102, 9, 122, 32);
	_txtCraft = new Text(84, 9, 224, 32);
	_txtAvailable = new Text(110, 9, 16, 24);
	_txtUsed = new Text(110, 9, 122, 24);
	_lstSoldiers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("craftSoldiers");

	add(_window, "window", "craftSoldiers");
	add(_btnOk, "button", "craftSoldiers");
	add(_txtTitle, "text", "craftSoldiers");
	add(_txtName, "text", "craftSoldiers");
	add(_txtRank, "text", "craftSoldiers");
	add(_txtCraft, "text", "craftSoldiers");
	add(_txtAvailable, "text", "craftSoldiers");
	add(_txtUsed, "text", "craftSoldiers");
	add(_lstSoldiers, "list", "craftSoldiers");

	_otherCraftColor = _game->getMod()->getInterface("craftSoldiers")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK02.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftSoldiersState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftSoldiersState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	Craft *c = _base->getCrafts()->at(_craft);
	_txtTitle->setText(tr("STR_SELECT_SQUAD_FOR_CRAFT").arg(c->getName(_game->getLanguage())));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtCraft->setText(tr("STR_CRAFT"));

	_lstSoldiers->setArrowColumn(192, ARROW_VERTICAL);
	_lstSoldiers->setColumns(3, 106, 102, 72);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onLeftArrowClick((ActionHandler)&CraftSoldiersState::lstItemsLeftArrowClick);
	_lstSoldiers->onRightArrowClick((ActionHandler)&CraftSoldiersState::lstItemsRightArrowClick);
	_lstSoldiers->onMouseClick((ActionHandler)&CraftSoldiersState::lstSoldiersClick, 0);
	_lstSoldiers->onMousePress((ActionHandler)&CraftSoldiersState::lstSoldiersMousePress);
	_lstSoldiers->onMouseWheel((ActionHandler)&CraftSoldiersState::lstSoldiersMouseWheel);

	_lstSoldiers->setDragScrollable(false);
	_lstSoldiers->onMouseOver((ActionHandler)&CraftSoldiersState::lstSoldiersMouseOver);

#ifdef __MOBILE__
	_longPressTimer = new Timer(Options::longPressDuration, false);
	_longPressTimer->onTimer((StateHandler)&CraftSoldiersState::lstSoldiersLongPress);
	_lstSoldiers->onMouseRelease((ActionHandler)&CraftSoldiersState::lstSoldiersMouseRelease);
#endif
}

/**
 *
 */
CraftSoldiersState::~CraftSoldiersState()
{
#ifdef __MOBILE__
	delete _longPressTimer;
#endif
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void CraftSoldiersState::initList(size_t scrl)
{
	int row = 0;
	_lstSoldiers->clearList();
	Craft *c = _base->getCrafts()->at(_craft);
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		_lstSoldiers->addRow(3, (*i)->getName(true, 19).c_str(), tr((*i)->getRankString()).c_str(), (*i)->getCraftString(_game->getLanguage()).c_str());

		Uint8 color;
		if ((*i)->getCraft() == c)
		{
			color = _lstSoldiers->getSecondaryColor();
		}
		else if ((*i)->getCraft() != 0)
		{
			color = _otherCraftColor;
		}
		else
		{
			color = _lstSoldiers->getColor();
		}
		_lstSoldiers->setRowColor(row, color);
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();

	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
}

/**
 * Shows the soldiers in a list.
 */
void CraftSoldiersState::init()
{
	State::init();
	initList(0);

}

/**
 * Reorders a soldier up.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstItemsLeftArrowClick(Action *action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	if (row > 0)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveSoldierUp(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveSoldierUp(action, row, true);
		}
	}
}

/**
 * Moves a soldier up on the list.
 * @param action Pointer to an action.
 * @param row Selected soldier row.
 * @param max Move the soldier to the top?
 */
void CraftSoldiersState::moveSoldierUp(Action *action, unsigned int row, bool max)
{
	Soldier *s = _base->getSoldiers()->at(row);
	if (max)
	{
		_base->getSoldiers()->erase(_base->getSoldiers()->begin() + row);
		_base->getSoldiers()->insert(_base->getSoldiers()->begin(), s);
	}
	else
	{
		_base->getSoldiers()->at(row) = _base->getSoldiers()->at(row - 1);
		_base->getSoldiers()->at(row - 1) = s;
		if (row != _lstSoldiers->getScroll())
		{
#ifndef __MOBILE__
			SDL_WarpMouseInWindow(NULL, (action->getLeftBlackBand() + action->getXMouse()), (action->getTopBlackBand() + action->getYMouse() - static_cast<Uint16>(8 * action->getYScale())));
#endif
		}
		else
		{
			_lstSoldiers->scrollUp(false);
		}
	}
	initList(_lstSoldiers->getScroll());
}

/**
 * Reorders a soldier down.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstItemsRightArrowClick(Action *action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	size_t numSoldiers = _base->getSoldiers()->size();
	if (0 < numSoldiers && INT_MAX >= numSoldiers && row < numSoldiers - 1)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveSoldierDown(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveSoldierDown(action, row, true);
		}
	}
}

/**
 * Moves a soldier down on the list.
 * @param action Pointer to an action.
 * @param row Selected soldier row.
 * @param max Move the soldier to the bottom?
 */
void CraftSoldiersState::moveSoldierDown(Action *action, unsigned int row, bool max)
{
	Soldier *s = _base->getSoldiers()->at(row);
	if (max)
	{
		_base->getSoldiers()->erase(_base->getSoldiers()->begin() + row);
		_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
	}
	else
	{
		_base->getSoldiers()->at(row) = _base->getSoldiers()->at(row + 1);
		_base->getSoldiers()->at(row + 1) = s;
		if (row != _lstSoldiers->getVisibleRows() - 1 + _lstSoldiers->getScroll())
		{
			//assert (0 && "FIXME");
#ifndef __MOBILE__
			SDL_WarpMouseInWindow(NULL, action->getLeftBlackBand() + action->getXMouse(), action->getTopBlackBand() + action->getYMouse() + static_cast<Uint16>(8 * action->getYScale()));
#endif
		}
		else
		{
			_lstSoldiers->scrollDown(false);
		}
	}
	initList(_lstSoldiers->getScroll());
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersClick(Action *action)
{
	if (_wasDragging)
	{
		_wasDragging = false;
		return;
	}
#ifdef __MOBILE__
	if (_clickGuard)
	{
		_clickGuard = false;
		return;
	}
#endif
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstSoldiers->getArrowsLeftEdge() && mx < _lstSoldiers->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstSoldiers->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		Craft *c = _base->getCrafts()->at(_craft);
		Soldier *s = _base->getSoldiers()->at(_lstSoldiers->getSelectedRow());
		Uint8 color = _lstSoldiers->getColor();
		if (s->getCraft() == c)
		{
			s->setCraft(0);
			_lstSoldiers->setCellText(row, 2, tr("STR_NONE_UC"));
		}
		else if (s->getCraft() && s->getCraft()->getStatus() == "STR_OUT")
		{
			color = _otherCraftColor;
		}
		else if (c->getSpaceAvailable() > 0 && s->getWoundRecovery() == 0)
		{
			s->setCraft(c);
			_lstSoldiers->setCellText(row, 2, c->getName(_game->getLanguage()));
			color = _lstSoldiers->getSecondaryColor();
		}
		_lstSoldiers->setRowColor(row, color);

		_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
		_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoState(_base, row));
	}
}

/**
 * Handles the mouse-wheels on the arrow-buttons.
 * Also, starts the long-press timer.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersMousePress(Action *action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	if (row < _base->getSoldiers()->size())
	{
		_pselSoldier = row;
#ifdef __MOBILE__
		_longPressTimer->start();
#endif
	}
}

/**
 * Handles the mouse-wheels on the arrow-buttons.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersMouseWheel(Action *action)
{
	if (Options::changeValueByMouseWheel == 0)
		return;
	unsigned int row = _lstSoldiers->getSelectedRow();
	size_t numSoldiers = _base->getSoldiers()->size();
	const SDL_Event &ev(*action->getDetails());
	if (ev.type == SDL_MOUSEWHEEL)
	{
		if (ev.wheel.y > 0 && row > 0)
		{
			if (action->getAbsoluteXMouse() >= _lstSoldiers->getArrowsLeftEdge() &&
				action->getAbsoluteXMouse() <= _lstSoldiers->getArrowsRightEdge())
			{
				moveSoldierUp(action, row);
			}
		}
		else if (ev.wheel.y < 0 && 0 < numSoldiers &&
			   	INT_MAX >= numSoldiers && row < (int)numSoldiers - 1)
		{
			if (action->getAbsoluteXMouse() >= _lstSoldiers->getArrowsLeftEdge() &&
				action->getAbsoluteXMouse() <= _lstSoldiers->getArrowsRightEdge())
			{
				moveSoldierDown(action, row);
			}
		}
	}
}

/**
 * Handles soldier drag-dropping in craft equipment screen.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersMouseOver(Action *action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	if (_pselSoldier < 0)
	{
		if (row < _base->getSoldiers()->size())
		{
			_pselSoldier = row;
		}
		return;
	}
	if (Options::dragSoldierReorder) {
		const SDL_Event *ev = action->getDetails();
		if ((ev->type == SDL_MOUSEMOTION) && (ev->motion.state) &&
											 (_lstSoldiers->getSelectedRow() < _base->getSoldiers()->size()))
		{
			int delta = row - _pselSoldier;
			{
				if (delta > 0) {
					_wasDragging = true;
					moveSoldierDown(action, _pselSoldier);
				}
				if (delta < 0) {
					_wasDragging = true;
					moveSoldierUp(action, _pselSoldier);
				}
			}
			_pselSoldier = row;
		}
	}
}

#ifdef __MOBILE__
/**
 * Pokes the timer.
 */
void CraftSoldiersState::think()
{
	State::think();
	_clickGuard = false;
	_longPressTimer->think(this, 0);
}

/**
 * Stops the long-press timer.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersMouseRelease(Action *action)
{
	_longPressTimer->stop();
}

/**
 * Emulates right-clicking.
 */
void CraftSoldiersState::lstSoldiersLongPress()
{
	_longPressTimer->stop();
	if (_wasDragging)
	{
		return;
	}
	_clickGuard = true;
	_game->pushState(new SoldierInfoState(_base, _pselSoldier));
}
#endif
}
