/*
This is a C++ port of the following PyQt example
http://diotavelli.net/PyQtWiki/Python%20syntax%20highlighting
C++ port by Frankie Simon (www.kickdrive.de, www.fuh-edv.de)
Adapted for SmuView by Frank Stettner

The following free software license applies for this file ("X11 license"):

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QColor>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>

#include "pythonsyntaxhighlighter.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace scripteditor {

PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument *parent) :
	QSyntaxHighlighter(parent)
{
	keywords_ = QStringList() << "and" << "assert" << "break" << "class" <<
		"continue" << "def" << "del" << "elif" << "else" << "except" <<
		"exec" << "finally" << "for" << "from" << "global" << "if" <<
		"import" << "in" << "is" << "lambda" << "not" << "or" << "pass" <<
		"print" << "raise" << "return" << "try" << "while" << "yield" <<
		"None" << "True" << "False";

	operators_ = QStringList() << "=" <<
		// Comparison
		"==" << "!=" << "<" << "<=" << ">" << ">=" <<
		// Arithmetic
		"\\+" << "-" << "\\*" << "/" << "//" << "%" << "\\*\\*" <<
		// In-place
		"\\+=" << "-=" << "\\*=" << "/=" << "%=" <<
		// Bitwise
		"\\^" << "\\|" << "&" << "~" << ">>" << "<<";

	braces_ = QStringList() << "{" << "}" << "\\(" << "\\)" << "\\[" << "\\]";

	basic_styles_.insert("keyword", get_text_char_format("blue"));
	basic_styles_.insert("operator", get_text_char_format("red"));
	basic_styles_.insert("brace", get_text_char_format("darkGray"));
	basic_styles_.insert("defclass", get_text_char_format("black", "bold"));
	basic_styles_.insert("brace", get_text_char_format("darkGray"));
	basic_styles_.insert("string", get_text_char_format("magenta"));
	basic_styles_.insert("string2", get_text_char_format("darkMagenta"));
	basic_styles_.insert("comment", get_text_char_format("darkGreen", "italic"));
	basic_styles_.insert("self", get_text_char_format("black", "italic"));
	basic_styles_.insert("numbers", get_text_char_format("brown"));

	tri_single_quote_.setPattern("'''");
	tri_double_quote_.setPattern("\"\"\"");

	initialize_rules();
}

void PythonSyntaxHighlighter::initialize_rules()
{
	for (QString currKeyword : keywords_) {
		rules_.append(HighlightingRule(
			QString("\\b%1\\b").arg(currKeyword), 0,
			basic_styles_.value("keyword")));
	}
	for (QString currOperator : operators_) {
		rules_.append(HighlightingRule(
			QString("%1").arg(currOperator), 0,
			basic_styles_.value("operator")));
	}
	for (QString currBrace : braces_) {
		rules_.append(HighlightingRule(
			QString("%1").arg(currBrace), 0,
			basic_styles_.value("brace")));
	}

	// 'self'
	rules_.append(HighlightingRule(
		"\\bself\\b", 0,
		basic_styles_.value("self")));

	// Double-quoted string, possibly containing escape sequences
	// FF: originally in python : r'"[^"\\]*(\\.[^"\\]*)*"'
	rules_.append(HighlightingRule(
		"\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"", 0,
		basic_styles_.value("string")));
	// Single-quoted string, possibly containing escape sequences
	// FF: originally in python : r"'[^'\\]*(\\.[^'\\]*)*'"
	rules_.append(HighlightingRule(
		"'[^'\\\\]*(\\\\.[^'\\\\]*)*'", 0,
		basic_styles_.value("string")));

	// 'def' followed by an identifier
	// FF: originally: r'\bdef\b\s*(\w+)'
	rules_.append(HighlightingRule(
		"\\bdef\\b\\s*(\\w+)", 1,
		basic_styles_.value("defclass")));
	// 'class' followed by an identifier
	// FF: originally: r'\bclass\b\s*(\w+)'
	rules_.append(HighlightingRule(
		"\\bclass\\b\\s*(\\w+)", 1,
		basic_styles_.value("defclass")));

	// From '#' until a newline
	// FF: originally: r'#[^\\n]*'
	rules_.append(HighlightingRule("#[^\\n]*", 0,
		basic_styles_.value("comment")));

	// Numeric literals
	// r'\b[+-]?[0-9]+[lL]?\b'
	rules_.append(HighlightingRule(
		"\\b[+-]?[0-9]+[lL]?\\b", 0,
		basic_styles_.value("numbers")));
	// r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b'
	rules_.append(HighlightingRule(
		"\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0,
		basic_styles_.value("numbers")));
	// r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b'
	rules_.append(HighlightingRule(
		"\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0,
		basic_styles_.value("numbers")));
}

void PythonSyntaxHighlighter::highlightBlock(const QString &text)
{
	for (HighlightingRule curr_rule : rules_)  {
		int idx = curr_rule.pattern.indexIn(text, 0);
		while (idx >= 0) {
			// Get index of Nth match
			idx = curr_rule.pattern.pos(curr_rule.nth);
			int length = curr_rule.pattern.cap(curr_rule.nth).length();
			setFormat(idx, length, curr_rule.format);
			idx = curr_rule.pattern.indexIn(text, idx + length);
		}
	}

	setCurrentBlockState(0);

	// Do multi-line strings
	bool is_in_multilne = match_multiline(
		text, tri_single_quote_, 1, basic_styles_.value("string2"));
	if (!is_in_multilne)
		is_in_multilne = match_multiline(
			text, tri_double_quote_, 2, basic_styles_.value("string2"));
}

bool PythonSyntaxHighlighter::match_multiline(const QString &text,
	const QRegExp &delimiter, const int in_state, const QTextCharFormat &style)
{
	int start = -1;
	int add = -1;
	int end = -1;
	int length = 0;

	// If inside triple-single quotes, start at 0
	if (previousBlockState() == in_state) {
		start = 0;
		add = 0;
	}
	// Otherwise, look for the delimiter on this line
	else {
		start = delimiter.indexIn(text);
		// Move past this match
		add = delimiter.matchedLength();
	}

	// As long as there's a delimiter match on this line...
	while (start >= 0) {
		// Look for the ending delimiter
		end = delimiter.indexIn(text, start + add);
		// Ending delimiter on this line?
		if (end >= add) {
			length = end - start + add + delimiter.matchedLength();
			setCurrentBlockState(0);
		}
		// No; multi-line string
		else {
			setCurrentBlockState(in_state);
			length = text.length() - start + add;
		}
		// Apply formatting and look for next
		setFormat(start, length, style);
		start = delimiter.indexIn(text, start + length);
	}
	// Return True if still inside a multi-line string, False otherwise
	if (currentBlockState() == in_state)
		return true;
	else
		return false;
}

const QTextCharFormat PythonSyntaxHighlighter::get_text_char_format(
	const QString &color_name, const QString &style)
{
	QTextCharFormat char_format;
	QColor color(color_name);
	char_format.setForeground(color);
	if (style.contains("bold", Qt::CaseInsensitive))
		char_format.setFontWeight(QFont::Bold);
	if (style.contains("italic", Qt::CaseInsensitive))
		char_format.setFontItalic(true);
	return char_format;
}

} // namespace scripteditor
} // namespace widgets
} // namespace ui
} // namespace sv
