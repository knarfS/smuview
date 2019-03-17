/*
This is a C++ port of the following PyQt example
http://diotavelli.net/PyQtWiki/Python%20syntax%20highlighting
C++ port by Frankie Simon (docklight.de, www.fuh-edv.de)
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

#ifndef SV_UI_WIDGETS_SCRIPTEDITOR_PYTHONSYNTAXHIGHLIGHTER_HPP
#define SV_UI_WIDGETS_SCRIPTEDITOR_PYTHONSYNTAXHIGHLIGHTER_HPP

#include <QHash>
#include <QList>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>

namespace sv {
namespace ui {
namespace widgets {
namespace scripteditor {

/**
 * Container to describe a highlighting rule. Based on a regular expression,
 * a relevant match and the format.
 */
class HighlightingRule
{

public:
	HighlightingRule(const QString &pattern_str, int n,
			const QTextCharFormat &matching_format) :
		original_rule_str(pattern_str),
		pattern(QRegExp(pattern_str)),
		nth(n),
		format(matching_format)
	{
	}

	QString original_rule_str;
	QRegExp pattern;
	int nth;
	QTextCharFormat format;

};

/**
 * Implementation of highlighting for Python code.
 */
class PythonSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	PythonSyntaxHighlighter(QTextDocument *parent = 0);

protected:
	void highlightBlock(const QString &text) override;

private:
	void initialize_rules();

	/**
	 * Highlighst multi-line strings, returns true if after processing we are
	 * still within the multi-line section.
	 */
	bool match_multiline(const QString &text, const QRegExp &delimiter,
		const int in_state, const QTextCharFormat &style);
	const QTextCharFormat get_text_char_format(const QString &color_name,
		const QString &style = QString());

	QStringList keywords_;
	QStringList operators_;
	QStringList braces_;
	QHash<QString, QTextCharFormat> basic_styles_;
	QList<HighlightingRule> rules_;
	QRegExp tri_single_quote_;
	QRegExp tri_double_quote_;

};

} // namespace scripteditor
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // SV_UI_WIDGETS_SCRIPTEDITOR_PYTHONSYNTAXHIGHLIGHTER_HPP
