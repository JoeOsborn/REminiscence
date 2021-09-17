
/*
 * REminiscence - Flashback interpreter
 * Copyright (C) 2005-2015 Gregory Montoir (cyx@users.sourceforge.net)
 */

#include "game.h"
#include "menu.h"
#include "resource.h"
#include "util.h"
#include "video.h"

Menu::Menu(Resource *res, Game *stub, Video *vid)
	: _res(res), _game(stub), _vid(vid) {
	_skill = 1;
	_level = 0;
}

void Menu::drawString(const char *str, int16_t y, int16_t x, uint8_t color) {
	uint8_t v1b = _vid->_charFrontColor;
	uint8_t v2b = _vid->_charTransparentColor;
	uint8_t v3b = _vid->_charShadowColor;
	switch (color) {
	case 0:
		_vid->_charFrontColor = _charVar1;
		_vid->_charTransparentColor = _charVar2;
		_vid->_charShadowColor = _charVar2;
		break;
	case 1:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = _charVar1;
		_vid->_charShadowColor = _charVar1;
		break;
	case 2:
		_vid->_charFrontColor = _charVar3;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 3:
		_vid->_charFrontColor = _charVar4;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 4:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 5:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar5;
		break;
	}

	drawString2(str, y, x);

	_vid->_charFrontColor = v1b;
	_vid->_charTransparentColor = v2b;
	_vid->_charShadowColor = v3b;
}

void Menu::drawString2(const char *str, int16_t y, int16_t x) {
	int i = 0;
	for (; str[i]; ++i) {
		_vid->PC_drawChar((uint8_t) str[i], y, x + i, true);
	}
}

void Menu::loadPicture(const char *prefix) {
	_res->load_MAP_menu(prefix, _res->_scratchBuffer);
	for (int i = 0; i < 4; ++i) {
		for (int y = 0; y < 224; ++y) {
			for (int x = 0; x < 64; ++x) {
				_vid->_frontLayer[i + x * 4 + 256 * y] = _res->_scratchBuffer[0x3800 * i + x + 64 * y];
			}
		}
	}
	_res->load_PAL_menu(prefix, _res->_scratchBuffer);
	_vid->setPalette(_res->_scratchBuffer, 256);
}

void Menu::handleInfoScreen() {
	_vid->fadeOut();
	if (_res->_lang == LANG_FR) {
		loadPicture("instru_f");
	} else {
		loadPicture("instru_e");
	}
	_vid->updateScreen();
	do {
		_game->sleep(EVENTS_DELAY);
		_game->processEvents();
		if (_game->_pi.escape) {
			_game->_pi.escape = false;
			break;
		}
		if (_game->_pi.use) {
			_game->_pi.use = false;
			break;
		}
	} while (!_game->_pi.quit);
}

void Menu::handleSkillScreen() {
	static const uint8_t colors[3][3] = {
		{2, 3, 3}, // easy
		{3, 2, 3}, // normal
		{3, 3, 2}  // expert
	};
	_vid->fadeOut();
	loadPicture("menu3");
	drawString(_res->getMenuString(LocaleData::LI_12_SKILL_LEVEL), 12, 4, 3);
	int skill_level = _skill;
	do {
		drawString(_res->getMenuString(LocaleData::LI_13_EASY), 15, 14, colors[skill_level][0]);
		drawString(_res->getMenuString(LocaleData::LI_14_NORMAL), 17, 14, colors[skill_level][1]);
		drawString(_res->getMenuString(LocaleData::LI_15_EXPERT), 19, 14, colors[skill_level][2]);

		_vid->updateScreen();
		_game->sleep(EVENTS_DELAY);
		_game->processEvents();

		if (_game->_pi.dirMask & PlayerInput::DIR_UP) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (skill_level != 0) {
				--skill_level;
			} else {
				skill_level = 2;
			}
		}
		if (_game->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (skill_level != 2) {
				++skill_level;
			} else {
				skill_level = 0;
			}
		}
		if (_game->_pi.escape) {
			_game->_pi.escape = false;
			break;
		}
		if (_game->_pi.use) {
			_game->_pi.use = false;
			_skill = skill_level;
			return;
		}
	} while (!_game->_pi.quit);
	_skill = 1;
}

bool Menu::handlePasswordScreen() {
	_vid->fadeOut();
	_vid->_charShadowColor = _charVar1;
	_vid->_charTransparentColor = 0xFF;
	_vid->_charFrontColor = _charVar4;
	char password[7];
	int len = 0;
	do {
		loadPicture("menu2");
		drawString2(_res->getMenuString(LocaleData::LI_16_ENTER_PASSWORD1), 15, 3);
		drawString2(_res->getMenuString(LocaleData::LI_17_ENTER_PASSWORD2), 17, 3);

		for (int i = 0; i < len; ++i) {
			_vid->PC_drawChar((uint8_t) password[i], 21, i + 15, false);
		}
		_vid->PC_drawChar(0x20, 21, len + 15, false);

		_vid->updateScreen();
		_game->sleep(EVENTS_DELAY);
		_game->processEvents();
		char c = _game->_pi.lastChar;
		if (c != 0) {
			_game->_pi.lastChar = 0;
			if (len < 6) {
				if (c >= 'a' && c <= 'z') {
					c &= ~0x20;
				}
				if ((c >= 'A' && c <= 'Z') || (c == 0x20)) {
					password[len] = c;
					++len;
				}
			}
		}
		if (_game->_pi.inventory_skip) {
			_game->_pi.inventory_skip = false;
			if (len > 0) {
				--len;
			}
		}
		if (_game->_pi.escape) {
			_game->_pi.escape = false;
			break;
		}
		if (_game->_pi.use) {
			_game->_pi.use = false;
			password[len] = '\0';
			for (int level = 0; level < 8; ++level) {
				for (int skill = 0; skill < 3; ++skill) {
					if (strcmp(_passwords[level][skill], password) == 0) {
						_level = level;
						_skill = skill;
						return true;
					}
				}
			}
			return false;
		}
	} while (!_game->_pi.quit);
	return false;
}

bool Menu::handleLevelScreen() {
	_vid->fadeOut();
	loadPicture("menu2");
	int currentSkill = _skill;
	int currentLevel = _level;
	do {
		static const char *levelTitles[] = {
			"Titan / The Jungle",
			"Titan / New Washington",
			"Titan / Death Tower Show",
			"Earth / Surface",
			"Earth / Paradise Club",
			"Planet Morphs / Surface",
			"Planet Morphs / Inner Core"
		};
		for (int i = 0; i < 7; ++i) {
			drawString(levelTitles[i], 7 + i * 2, 4, (currentLevel == i) ? 2 : 3);
		}

		drawString(_res->getMenuString(LocaleData::LI_13_EASY), 23, 4, (currentSkill == 0) ? 2 : 3);
		drawString(_res->getMenuString(LocaleData::LI_14_NORMAL), 23, 14, (currentSkill == 1) ? 2 : 3);
		drawString(_res->getMenuString(LocaleData::LI_15_EXPERT), 23, 24, (currentSkill == 2) ? 2 : 3);

		_vid->updateScreen();
		_game->sleep(EVENTS_DELAY);
		_game->processEvents();

		if (_game->_pi.dirMask & PlayerInput::DIR_UP) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (currentLevel != 0) {
				--currentLevel;
			} else {
				currentLevel = 6;
			}
		}
		if (_game->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (currentLevel != 6) {
				++currentLevel;
			} else {
				currentLevel = 0;
			}
		}
		if (_game->_pi.dirMask & PlayerInput::DIR_LEFT) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_LEFT;
			if (currentSkill != 0) {
				--currentSkill;
			} else {
				currentSkill = 2;
			}
		}
		if (_game->_pi.dirMask & PlayerInput::DIR_RIGHT) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
			if (currentSkill != 2) {
				++currentSkill;
			} else {
				currentSkill = 0;
			}
		}
		if (_game->_pi.escape) {
			_game->_pi.escape = false;
			break;
		}
		if (_game->_pi.use) {
			_game->_pi.use = false;
			_skill = currentSkill;
			_level = currentLevel;
			return true;
		}
	} while (!_game->_pi.quit);
	return false;
}

void Menu::handleTitleScreen() {

	_charVar1 = 0;
	_charVar2 = 0;
	_charVar3 = 0;
	_charVar4 = 0;
	_charVar5 = 0;

	static const int MAX_MENU_ITEMS = 6;
	Item menuItems[MAX_MENU_ITEMS];
	int menuItemsCount = 0;

	menuItems[menuItemsCount].str = LocaleData::LI_07_START;
	menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_START;
	++menuItemsCount;
	if (g_options.enable_password_menu) {
		menuItems[menuItemsCount].str = LocaleData::LI_08_SKILL;
		menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_SKILL;
		++menuItemsCount;
		menuItems[menuItemsCount].str = LocaleData::LI_09_PASSWORD;
		menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_PASSWORD;
		++menuItemsCount;
	} else {
		menuItems[menuItemsCount].str = LocaleData::LI_06_LEVEL;
		menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_LEVEL;
		++menuItemsCount;
	}
	menuItems[menuItemsCount].str = LocaleData::LI_10_INFO;
	menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_INFO;
	++menuItemsCount;
	menuItems[menuItemsCount].str = LocaleData::LI_23_DEMO;
	menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_DEMO;
	++menuItemsCount;
	menuItems[menuItemsCount].str = LocaleData::LI_11_QUIT;
	menuItems[menuItemsCount].opt = MENU_OPTION_ITEM_QUIT;
	++menuItemsCount;

	_selectedOption = -1;
	_currentScreen = -1;
	_nextScreen = SCREEN_TITLE;

	bool quitLoop = false;
	int currentEntry = 0;

	while (!quitLoop) {
		if (_nextScreen == SCREEN_TITLE) {
			_vid->fadeOut();
			loadPicture("menu1");
			_charVar3 = 1;
			_charVar4 = 2;
			currentEntry = 0;
			_currentScreen = _nextScreen;
			_nextScreen = -1;
		}
		int selectedItem = -1;
		const int yPos = 26 - menuItemsCount * 2;
		for (int i = 0; i < menuItemsCount; ++i) {
			drawString(_res->getMenuString(menuItems[i].str), yPos + i * 2, 20, (i == currentEntry) ? 2 : 3);
		}

		_vid->updateScreen();
		_game->sleep(EVENTS_DELAY);
		_game->processEvents();

		if (_game->_pi.dirMask & PlayerInput::DIR_UP) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (currentEntry != 0) {
				--currentEntry;
			} else {
				currentEntry = menuItemsCount - 1;
			}
		}
		if (_game->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_game->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (currentEntry != menuItemsCount - 1) {
				++currentEntry;
			} else {
				currentEntry = 0;
			}
		}
		if (_game->_pi.use) {
			_game->_pi.use = false;
			selectedItem = currentEntry;
		}

		if (selectedItem != -1) {
			_selectedOption = menuItems[selectedItem].opt;
			switch (_selectedOption) {
			case MENU_OPTION_ITEM_START:
				quitLoop = true;
				break;
			case MENU_OPTION_ITEM_SKILL:
				_currentScreen = SCREEN_SKILL;
				handleSkillScreen();
				break;
			case MENU_OPTION_ITEM_PASSWORD:
				_currentScreen = SCREEN_PASSWORD;
				quitLoop = handlePasswordScreen();
				break;
			case MENU_OPTION_ITEM_LEVEL:
				_currentScreen = SCREEN_LEVEL;
				quitLoop = handleLevelScreen();
				break;
			case MENU_OPTION_ITEM_INFO:
				_currentScreen = SCREEN_INFO;
				handleInfoScreen();
				break;
			case MENU_OPTION_ITEM_DEMO:
				quitLoop = true;
				break;
			case MENU_OPTION_ITEM_QUIT:
				quitLoop = true;
				break;
			}
			_nextScreen = SCREEN_TITLE;
		}
		if (_game->_pi.quit) {
			break;
		}
	}
}
