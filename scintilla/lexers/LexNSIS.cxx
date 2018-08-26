// Lexer for NSIS.

#include <cstring>
#include <cassert>
#include <cctype>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

using namespace Scintilla;

static constexpr bool IsNsisOp(int ch) noexcept {
	return ch == '(' || ch == ')' || ch == '+' || ch == '-' || ch == '&' || ch == '|'
		|| ch == '=' || ch == ':' || ch == ',' || ch == '<' || ch == '>'
		|| ch == '!' || ch == '.';
}

#define MAX_WORD_LENGTH	15
static void ColouriseNSISDoc(Sci_PositionU startPos, Sci_Position length, int initStyle, LexerWordList, Accessor &styler) {
	int state = initStyle;
	int ch = 0, chNext = styler[startPos];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	Sci_PositionU endPos = startPos + length;
	if (endPos == static_cast<Sci_PositionU>(styler.Length()))
		++endPos;

	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	//char buf[MAX_WORD_LENGTH + 1] = {0};
	//int wordLen = 0;

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const int chPrev = ch;
		ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		const bool atLineStart = i == static_cast<Sci_PositionU>(styler.LineStart(lineCurrent));

		switch (state) {
		case SCE_C_OPERATOR:
			styler.ColourTo(i - 1, state);
			state = SCE_C_DEFAULT;
			break;
		case SCE_C_NUMBER:
			if (!(iswordchar(ch) || ch == '%' || ((ch == '+' || ch == '-') && IsADigit(chNext)))) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_IDENTIFIER:
			if (!(iswordchar(ch) || ch == '-' || ch == '\\')) {
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_WORD:
			if (!iswordchar(ch)) {
				if (ch == ':' && chNext != ':') {
					state = SCE_C_LABEL;
				}
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_PREPROCESSOR:
		case SCE_C_ASM_INSTRUCTION:
			if (!iswordchar(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_ASM_REGISTER:
			if (chPrev == '}') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_COMMENTDOC_TAG:
			if (chPrev == ')') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_CHARACTER:
			if (ch == '\'') {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
				continue;
			}
			break;
		case SCE_C_STRING:
			if (chPrev == '$' && ch == '\\' && (chNext == '\\' || chNext == '\"')) {
				i++;
				ch = chNext;
				chNext = styler.SafeGetCharAt(i + 1);
			} else if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
				continue;
			}
			break;
		case SCE_C_STRINGEOL:
			if (ch == '`') {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
				continue;
			}
			break;
		case SCE_C_COMMENTLINE:
			if (atLineStart) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_COMMENT:
			if (ch == '*' && chNext == '/') {
				i++;
				ch = chNext;
				chNext = styler.SafeGetCharAt(i + 1);
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
				continue;
			}
			break;
		}

		if (state != SCE_C_COMMENTLINE && ch == '\\' && (chNext == '\n' || chNext == '\r')) {
			i++;
			lineCurrent++;
			ch = chNext;
			chNext = styler.SafeGetCharAt(i + 1);
			if (ch == '\r' && chNext == '\n') {
				i++;
				ch = chNext;
				chNext = styler.SafeGetCharAt(i + 1);
			}
			continue;
		}

		if (state == SCE_C_DEFAULT) {
			if (ch == ';' || ch == '#') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENTLINE;
			} else if (ch == '/' && chNext == '*') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENT;
				i++;
				ch = chNext;
				chNext = styler.SafeGetCharAt(i + 1);
			} else if (ch == '\'') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_CHARACTER;
			} else if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_STRING;
			} else if (ch == '`') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_STRINGEOL;
			} else if (IsADigit(ch) || (ch == '.' && IsADigit(chNext))) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_NUMBER;
			} else if (visibleChars == 0 && ch == '!' && iswordstart(chNext)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_PREPROCESSOR;
			} else if (ch == '$' && iswordstart(chNext)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_ASM_INSTRUCTION;
			} else if (ch == '$' && chNext == '{') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_ASM_REGISTER;
			} else if (ch == '$' && chNext == '(') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENTDOC_TAG;
			} else if (visibleChars == 0 && iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_WORD;
			} else if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_IDENTIFIER;
			} else if (IsNsisOp(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_OPERATOR;
			}
		}

		if (atEOL || i == endPos - 1) {
			lineCurrent++;
			visibleChars = 0;
		}
		if (!isspacechar(ch)) {
			visibleChars++;
		}
	}

	// Colourise remaining document
	styler.ColourTo(endPos - 1, state);
}

#define IsCommentLine(line)			IsLexCommentLine(line, styler, SCE_C_COMMENTLINE)
#define IsStreamCommantStyle(style)	(style == SCE_C_COMMENT)
static constexpr bool IsNsisFoldWordStart(int ch) noexcept {
	return (ch == 'S' || ch == 'F' || ch == 'P')
		|| (ch == 's' || ch == 'f' || ch == 'p');
}
static constexpr bool IsNsisFoldPPStart(int ch) noexcept {
	return (ch == 'i' || ch == 'e' || ch == 'm')
		|| (ch == 'I' || ch == 'E' || ch == 'M');
}

static void FoldNSISDoc(Sci_PositionU startPos, Sci_Position length, int initStyle, LexerWordList, Accessor &styler) {
	if (styler.GetPropertyInt("fold") == 0)
		return;
	const bool foldComment = styler.GetPropertyInt("fold.comment") != 0;
	const bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

	const Sci_PositionU endPos = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
	int levelNext = levelCurrent;

	int chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const int ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		const int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (foldComment && atEOL && IsCommentLine(lineCurrent)) {
			if (!IsCommentLine(lineCurrent - 1) && IsCommentLine(lineCurrent + 1))
				levelNext++;
			else if (IsCommentLine(lineCurrent - 1) && !IsCommentLine(lineCurrent + 1))
				levelNext--;
		}
		if (foldComment && IsStreamCommantStyle(style)) {
			if (!IsStreamCommantStyle(stylePrev))
				levelNext++;
			else if (!IsStreamCommantStyle(styleNext) && !atEOL)
				levelNext--;

		}

		if (IsNsisFoldWordStart(ch) && style == SCE_C_WORD && stylePrev != SCE_C_WORD) {
			char buf[MAX_WORD_LENGTH + 1];
			LexGetRangeLowered(i, styler, iswordchar, buf, sizeof(buf));
			if (!(strcmp(buf, "section") && strcmp(buf, "function") && strcmp(buf, "sectiongroup") && strcmp(buf, "pageex")))
				levelNext++;
			else if (!(strcmp(buf, "sectionend") && strcmp(buf, "functionend") && strcmp(buf, "sectiongroupend") && strcmp(buf, "pageexend")))
				levelNext--;
		}
		if (ch == '!' && IsNsisFoldPPStart(chNext) && style == SCE_C_PREPROCESSOR) {
			char buf[MAX_WORD_LENGTH + 1];
			LexGetRangeLowered(i + 1, styler, iswordchar, buf, sizeof(buf));
			if (strcmp(buf, "macro") == 0 || (buf[0] == 'i' && buf[1] == 'f'))
				levelNext++;
			else if (strcmp(buf, "macroend") == 0 || (buf[0] == 'e' && buf[1] == 'n' && buf[2] == 'd'))
				levelNext--;
		}

		if (!isspacechar(ch))
			visibleChars++;

		if (atEOL || (i == endPos - 1)) {
			const int levelUse = levelCurrent;
			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			visibleChars = 0;
		}
	}
}

LexerModule lmNsis(SCLEX_NSIS, ColouriseNSISDoc, "nsis", FoldNSISDoc);
