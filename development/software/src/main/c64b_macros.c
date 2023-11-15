#include "c64b_macros.h"

//----------------------------------------------------------------------------//
// startup commands

char* feed_cmd_gui[KB_MACRO_COUNT] =
{
	"~clr~load tape",
	"~clr~load disk",
	"~clr~run disk"
};

char* feed_cmd_str[KB_MACRO_COUNT] =
{
	"~clr~load~ret~",
	"~clr~load \"$\",8~ret~",
	"~clr~load \"*\",8~ret~"
};


//char* solitaire_prg[] =
//{
//
//};
