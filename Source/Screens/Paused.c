/****************************/
/*   	PAUSED.C			*/
/* By Brian Greenstone      */
/* (c)2000 Pangea Software  */
/* (c)2022 Iliyas Jorio     */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/

#include "game.h"
#include "menu.h"
#include "network.h"
#include "miscscreens.h"

/****************************/
/*    PROTOTYPES            */
/****************************/

static bool ShouldDisplaySplitscreenModeCycler(const MenuItem* mi);
static void OnToggleSplitscreenMode(const MenuItem* mi);


/****************************/
/*    CONSTANTS             */
/****************************/

static const MenuItem gPauseMenuTree[] =
{
	{ .id='paus' },
	{kMIPick, STR_RESUME_GAME, .id=0, .next='EXIT' },
	{kMIPick, STR_RETIRE_GAME, .id=1, .next='EXIT' },
	{kMICycler1, STR_SPLITSCREEN_MODE,
		.displayIf = ShouldDisplaySplitscreenModeCycler,
		.callback = OnToggleSplitscreenMode,
		.cycler =
		{
			.valuePtr = &gGamePrefs.desiredSplitScreenMode,
			.choices =
			{
				{ .text = STR_SPLITSCREEN_HORIZ, .value = SPLITSCREEN_MODE_2X1 },
				{ .text = STR_SPLITSCREEN_VERT, .value = SPLITSCREEN_MODE_1X2 },
			},
		}
	},
	{kMIPick, STR_SETTINGS, .callback=RegisterSettingsMenu, .next='sett' },
	{kMIPick, STR_QUIT_APPLICATION, .id=2, .next='EXIT' },
	{ 0 },
};


/*********************/
/*    VARIABLES      */
/*********************/

Boolean gGamePaused = false;


/****************** TOGGLE SPLIT-SCREEN MODE ********************/

bool ShouldDisplaySplitscreenModeCycler(const MenuItem* mi)
{
	return gNumSplitScreenPanes == 2;
}

void OnToggleSplitscreenMode(const MenuItem* mi)
{
	gActiveSplitScreenMode = gGamePrefs.desiredSplitScreenMode;
}


/********************** DO PAUSED **************************/

static void UpdatePausedMenuCallback(void)
{
	MoveObjects();
	DoPlayerTerrainUpdate();							// need to call this to keep supertiles active


			/* IF DOING NET GAME, LET OTHER PLAYERS KNOW WE'RE STILL GOING SO THEY DONT TIME OUT */

	if (gNetGameInProgress)
		PlayerBroadcastNullPacket();
}

static void DrawPausedMenuCallback(OGLSetupOutputType* setupInfo)
{

	DrawTerrain(setupInfo);
}

void DoPaused(void)
{
	Boolean	oldMute = gMuteMusicFlag;

	MenuStyle style = kDefaultMenuStyle;
	style.canBackOutOfRootMenu = true;
	style.fadeOutSceneOnExit = false;

	PushKeys();										// save key state so things dont get de-synced during net games

	if (!gMuteMusicFlag)							// see if pause music
		ToggleMusic();

	gGamePaused = true;

				/*************/
				/* MAIN LOOP */
				/*************/

	CalcFramesPerSecond();
	ReadKeyboard();

	int outcome = StartMenu(gPauseMenuTree, &style, UpdatePausedMenuCallback, DrawPausedMenuCallback);

	gGamePaused = false;
	
	PopKeys();										// restore key state

	if (!oldMute)									// see if restart music
		ToggleMusic();

	switch (outcome)
	{
		case	0:								// RESUME
			break;

		case	1:								// EXIT
			gGameOver = true;
			break;

		case	2:								// QUIT
			CleanQuit();
			break;
	}
}
