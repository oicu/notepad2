// Scintilla source code edit control
/** @file ViewStyle.h
 ** Store information on how the document is to be viewed.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once

namespace Scintilla::Internal {

/**
 */
class MarginStyle {
public:
	Scintilla::MarginType style;
	ColourRGBA back;
	int width;
	MarkerMask mask;
	bool sensitive;
	Scintilla::CursorShape cursor;
	MarginStyle(Scintilla::MarginType style_ = Scintilla::MarginType::Symbol, int width_ = 0, MarkerMask mask_ = 0) noexcept;
	bool ShowsFolding() const noexcept;
};

/**
 */
class FontRealised : public FontMeasurements {
public:
	std::shared_ptr<Font> font;
	FontRealised() noexcept = default;
	// FontRealised objects can not be copied.
	FontRealised(const FontRealised &) = delete;
	FontRealised(FontRealised &&) = delete;
	FontRealised &operator=(const FontRealised &) = delete;
	FontRealised &operator=(FontRealised &&) = delete;
	virtual ~FontRealised() noexcept = default;
	void Realise(Surface &surface, int zoomLevel, Scintilla::Technology technology, const FontSpecification &fs, const char *localeName);
};

using FontMap = std::map<FontSpecification, std::unique_ptr<FontRealised>>;

constexpr int GetFontSizeZoomed(int size, int zoomLevel) noexcept {
	size = (size * zoomLevel + 50) / 100;
	// Hangs if sizeZoomed (in point) <= 1
	return std::max(size, 2 * Scintilla::FontSizeMultiplier);
}

constexpr std::optional<ColourRGBA> OptionalColour(uptr_t wParam, sptr_t lParam) {
	if (wParam) {
		return ColourRGBA::FromIpRGB(lParam);
	}
	return std::nullopt;
}

struct SelectionAppearance {
	// Whether to draw on base layer or over text
	Scintilla::Layer layer;
	// Draw selection past line end characters up to right border
	bool eolFilled;
	int eolSelectedWidth;
};

struct CaretLineAppearance {
	// Whether to draw on base layer or over text
	Scintilla::Layer layer;
	// Also show when non-focused
	bool alwaysShow;
	// Non-0: draw a rectangle around line instead of filling line. Value is pixel width of frame
	int frame;
};

struct CaretAppearance {
	// Line, block, over-strike bar ...
	Scintilla::CaretStyle style;
	// Width in pixels
	int width;
};

struct WrapAppearance {
	// No wrapping, word, character, whitespace appearance
	Scintilla::Wrap state;
	// Show indication of wrap at line end, line start, or in margin
	Scintilla::WrapVisualFlag visualFlags;
	// Show indication near margin or near text
	Scintilla::WrapVisualLocation visualFlagsLocation;
	// How much indentation to show wrapping
	int visualStartIndent;
	// WrapIndentMode::Fixed, _SAME, _INDENT, _DEEPINDENT
	Scintilla::WrapIndentMode indentMode;
};

struct EdgeProperties {
	int column;
	ColourRGBA colour;
	EdgeProperties(int column_ = 0, ColourRGBA colour_ = ColourRGBA(0)) noexcept :
		column(column_), colour(colour_) {}
	EdgeProperties(Scintilla::uptr_t wParam, Scintilla::sptr_t lParam) noexcept :
		column(static_cast<int>(wParam)), colour(ColourRGBA::FromIpRGB(lParam)) {}
};

// This is an old style enum so that its members can be used directly as indices without casting
enum StyleIndices {
	StyleDefault = static_cast<int>(Scintilla::StylesCommon::Default),
	StyleLineNumber = static_cast<int>(Scintilla::StylesCommon::LineNumber),
	StyleBraceLight = static_cast<int>(Scintilla::StylesCommon::BraceLight),
	StyleBraceBad = static_cast<int>(Scintilla::StylesCommon::BraceBad),
	StyleControlChar = static_cast<int>(Scintilla::StylesCommon::ControlChar),
	StyleIndentGuide = static_cast<int>(Scintilla::StylesCommon::IndentGuide),
	StyleCallTip = static_cast<int>(Scintilla::StylesCommon::CallTip),
	StyleFoldDisplayText = static_cast<int>(Scintilla::StylesCommon::FoldDisplayText),
};

/**
 */
class ViewStyle {
	UniqueStringSet fontNames;
	FontMap fonts;
public:
	std::vector<Style> styles;
	int nextExtendedStyle;
	std::vector<LineMarker> markers;
	int largestMarkerHeight;
	std::vector<Indicator> indicators;
	bool indicatorsDynamic;
	bool indicatorsSetFore;
	bool fontsValid;
	Scintilla::Technology technology;
	int lineHeight;
	int lineOverlap;
	unsigned int maxAscent;
	unsigned int maxDescent;
	XYPOSITION aveCharWidth;
	XYPOSITION spaceWidth;
	XYPOSITION tabWidth;

	SelectionAppearance selection;

	int controlCharSymbol;
	XYPOSITION controlCharWidth;
	ColourRGBA selbar;
	ColourRGBA selbarlight;
	std::optional<ColourRGBA> foldmarginColour;
	std::optional<ColourRGBA> foldmarginHighlightColour;
	bool hotspotUnderline;
	/// Margins are ordered: Line Numbers, Selection Margin, Spacing Margin
	int leftMarginWidth;	///< Spacing margin on left of text
	int rightMarginWidth;	///< Spacing margin on right of text
	MarkerMask maskInLine;	///< Mask for markers to be put into text because there is nowhere for them to go in margin
	MarkerMask maskDrawInText;///< Mask for markers that always draw in text
	std::vector<MarginStyle> ms;
	int fixedColumnWidth;	///< Total width of margins
	bool marginInside;	///< true: margin included in text view, false: separate views
	int textStart;	///< Starting x position of text within the view
	/// 2018-09-04 Changed to a percent value
	int zoomLevel;
	Scintilla::WhiteSpace viewWhitespace;
	Scintilla::TabDrawMode tabDrawMode;
	int whitespaceSize;
	Scintilla::IndentView viewIndentationGuides;
	bool viewEOL;

	CaretAppearance caret;

	CaretLineAppearance caretLine;

	bool someStylesProtected;
	bool someStylesForceCase;
	Scintilla::FontQuality extraFontFlag;
	int extraAscent;
	int extraDescent;
	int marginStyleOffset;
	Scintilla::AnnotationVisible annotationVisible;
	int annotationStyleOffset;
	Scintilla::EOLAnnotationVisible eolAnnotationVisible;
	int eolAnnotationStyleOffset;
	bool braceHighlightIndicatorSet;
	bool braceBadLightIndicatorSet;
	int braceHighlightIndicator;
	int braceBadLightIndicator;
	Scintilla::EdgeVisualStyle edgeState;
	EdgeProperties theEdge;
	std::vector<EdgeProperties> theMultiEdge;
	int marginNumberPadding; // the right-side padding of the number margin
	int ctrlCharPadding; // the padding around control character text blobs
	int lastSegItalicsOffset; // the offset so as not to clip italic characters at EOLs

	using ElementMap = std::map<Scintilla::Element, std::optional<ColourRGBA>>;
	ElementMap elementColours;
	ElementMap elementBaseColours;
	std::set<Scintilla::Element> elementAllowsTranslucent;

	WrapAppearance wrap;

	std::string localeName;

	ViewStyle();
	ViewStyle(const ViewStyle &source);
	ViewStyle(ViewStyle &&) = delete;
	// Can only be copied through copy constructor which ensures font names initialised correctly
	ViewStyle &operator=(const ViewStyle &) = delete;
	ViewStyle &operator=(ViewStyle &&) = delete;
	~ViewStyle();
	void CalculateMarginWidthAndMask() noexcept;
	void Init(size_t stylesSize_ = 256);
	void Refresh(Surface &surface, int tabInChars);
	void ReleaseAllExtendedStyles() noexcept;
	int AllocateExtendedStyles(int numberStyles);
	void CopyStyles(size_t sourceIndex, size_t destStyles);
	void EnsureStyle(size_t index);
	void ResetDefaultStyle();
	void ClearStyles() noexcept;
	void SetStyleFontName(int styleIndex, const char *name);
	void SetFontLocaleName(const char *name);
	bool ProtectionActive() const noexcept;
	int ExternalMarginWidth() const noexcept;
	int SCICALL MarginFromLocation(Point pt) const noexcept;
	bool ValidStyle(size_t styleIndex) const noexcept;
	void CalcLargestMarkerHeight() noexcept;
	int GetFrameWidth() const noexcept;
	bool IsLineFrameOpaque(bool caretActive, bool lineContainsCaret) const;
	std::optional<ColourRGBA> Background(MarkerMask marksOfLine, bool caretActive, bool lineContainsCaret) const;
	bool SelectionTextDrawn() const;
	bool SelectionBackgroundDrawn() const noexcept;
	bool WhitespaceBackgroundDrawn() const;
	ColourRGBA WrapColour() const;

	void AddMultiEdge(Scintilla::uptr_t wParam, Scintilla::sptr_t lParam);

	std::optional<ColourRGBA> ElementColour(Scintilla::Element element) const;
	bool ElementAllowsTranslucent(Scintilla::Element element) const;
	bool ResetElement(Scintilla::Element element);
	bool SetElementColour(Scintilla::Element element, ColourRGBA colour);
	bool ElementIsSet(Scintilla::Element element) const;
	bool SetElementBase(Scintilla::Element element, ColourRGBA colour);

	bool SetWrapState(Scintilla::Wrap wrapState_) noexcept;
	bool SetWrapVisualFlags(Scintilla::WrapVisualFlag wrapVisualFlags_) noexcept;
	bool SetWrapVisualFlagsLocation(Scintilla::WrapVisualLocation wrapVisualFlagsLocation_) noexcept;
	bool SetWrapVisualStartIndent(int wrapVisualStartIndent_) noexcept;
	bool SetWrapIndentMode(Scintilla::WrapIndentMode wrapIndentMode_) noexcept;

	bool WhiteSpaceVisible(bool inIndent) const noexcept;

	enum class CaretShape { invisible, line, block, bar };
	bool IsBlockCaretStyle() const noexcept;
	bool IsCaretVisible() const noexcept;
	bool DrawCaretInsideSelection(bool inOverstrike, bool imeCaretBlockOverride) const noexcept;
	CaretShape CaretShapeForMode(bool inOverstrike, bool drawDrag, bool drawOverstrikeCaret, bool imeCaretBlockOverride) const noexcept;

	bool ZoomIn() noexcept;
	bool ZoomOut() noexcept;

private:
	void AllocStyles(size_t sizeNew);
	void CreateAndAddFont(const FontSpecification &fs);
	FontRealised *Find(const FontSpecification &fs) const;
	void FindMaxAscentDescent() noexcept;
};

}
