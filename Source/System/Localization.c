// LOCALIZATION.C
// (C) 2021 Iliyas Jorio
// This file is part of Otto Matic. https://github.com/jorio/ottomatic

#include "Pomme.h"
#include "localization.h"
#include "misc.h"
#include <string.h>
#include <SDL.h>

#define SEPARATOR '\t'
#define CSV_PATH ":system:strings.tsv"

#define MAX_STRINGS (NUM_LOCALIZED_STRINGS + 1)

extern FSSpec gDataSpec;

static GameLanguageID	gCurrentStringsLanguage = LANGUAGE_ILLEGAL;
static Ptr				gStringsBuffer = nil;
static const char*		gStringsTable[MAX_STRINGS];

static const char kLanguageCodesISO639_1[MAX_LANGUAGES][3] =
{
	[LANGUAGE_ENGLISH	] = "en",
	[LANGUAGE_FRENCH	] = "fr",
	[LANGUAGE_GERMAN	] = "de",
	[LANGUAGE_SPANISH	] = "es",
	[LANGUAGE_ITALIAN	] = "it",
	[LANGUAGE_SWEDISH	] = "sv",
};

void LoadLocalizedStrings(GameLanguageID languageID)
{
	// Don't bother reloading strings if we've already loaded this language
	if (languageID == gCurrentStringsLanguage)
	{
		return;
	}

	// Free previous strings buffer
	if (gStringsBuffer != nil)
	{
		SafeDisposePtr(gStringsBuffer);
		gStringsBuffer = nil;
	}

	GAME_ASSERT(languageID >= 0);
	GAME_ASSERT(languageID < MAX_LANGUAGES);

	FSSpec spec;
	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, CSV_PATH, &spec);

	short refNum;
	if (FSpOpenDF(&spec, fsRdPerm, &refNum) != noErr)
		DoFatalAlert("LoadTextFile: FSpOpenDF failed");

	long eof = 0;
	GetEOF(refNum, &eof);
	long count = eof;
	gStringsBuffer = AllocPtrClear(count + 1);		// +1 for final '\0'
	FSRead(refNum, &count, gStringsBuffer);
	FSClose(refNum);

	GAME_ASSERT(count == eof);

	for (int i = 0; i < MAX_STRINGS; i++)
		gStringsTable[i] = nil;

	int col = 0;
	int row = 0;
	bool rowIsEmpty = true;

	char* currentString = gStringsBuffer;
	char* fallbackString = gStringsBuffer;

	for (int i = 0; i < count; i++)
	{
		char currChar = gStringsBuffer[i];

		if (currChar == SEPARATOR)
		{
			gStringsBuffer[i] = '\0';

			if (col == languageID)
			{
				gStringsTable[row] = currentString[0]? currentString: fallbackString;
			}

			currentString = &gStringsBuffer[i + 1];
			col++;
		}
		else if (currChar == '\r' && gStringsBuffer[i + 1] == '\n')  // Windows CRLF
		{
			gStringsBuffer[i] = '\0';
			continue;
		}
		else if (currChar == '\n' || currChar == '\r')
		{
			gStringsBuffer[i] = '\0';

			if (col == languageID)
			{
				gStringsTable[row] = currentString[0]? currentString: fallbackString;
			}

			col = 0;
			if (!rowIsEmpty)
			{
				row++;
				rowIsEmpty = true;
				GAME_ASSERT(row < MAX_STRINGS);
			}

			currentString = &gStringsBuffer[i + 1];
			fallbackString = currentString;
		}
		else
		{
			rowIsEmpty = false;
		}
	}

	//for (int i = 0; i < MAX_STRINGS; i++) printf("String #%d: %s\n", i, gStringsTable[i]);
}

const char* Localize(LocStrID stringID)
{
	if (!gStringsBuffer)
		return "STRINGS NOT LOADED";

	if (stringID < 0 || stringID >= MAX_STRINGS)
		return "ILLEGAL STRING ID";

	if (!gStringsTable[stringID])
		return "";

	return gStringsTable[stringID];
}

GameLanguageID GetBestLanguageIDFromSystemLocale(void)
{
	GameLanguageID languageID = LANGUAGE_ENGLISH;

#if !(SDL_VERSION_ATLEAST(2,0,14))
	#warning Please upgrade to SDL 2.0.14 or later for SDL_GetPreferredLocales. Will default to English for now.
#else
	SDL_Locale* localeList = SDL_GetPreferredLocales();
	if (!localeList)
		return languageID;

	for (SDL_Locale* locale = localeList; locale->language; locale++)
	{
		for (int i = 0; i < MAX_LANGUAGES; i++)
		{
			if (0 == strncmp(locale->language, kLanguageCodesISO639_1[i], 2))
			{
				languageID = i;
				goto foundLocale;
			}
		}
	}

foundLocale:
	SDL_free(localeList);
#endif

	return languageID;
}
