// QCodeEditor
#include <QCXXHighlighter>
#include <QCodeEditor>
#include <QJSHighlighter>
#include <QJavaHighlighter>
#include <QLineNumberArea>
#include <QPythonHighlighter>
#include <QStyleSyntaxHighlighter>
#include <QSyntaxStyle>

// Qt
#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QCompleter>
#include <QCursor>
#include <QDebug>
#include <QFontDatabase>
#include <QMimeData>
#include <QPaintEvent>
#include <QScrollBar>
#include <QShortcut>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextStream>
#include <QToolTip>

QCodeEditor::QCodeEditor(QWidget *widget)
    : QTextEdit(widget), m_highlighter(nullptr), m_syntaxStyle(nullptr), m_lineNumberArea(new QLineNumberArea(this)),
      m_completer(nullptr), m_autoIndentation(true), m_replaceTab(true), m_extraBottomMargin(true),
      m_tabReplace(QString(4, ' ')), extra1(), extra2(), extra_squiggles(), m_squiggler(),
      m_parentheses({{'(', ')'}, {'{', '}'}, {'[', ']'}, {'\"', '\"'}, {'\'', '\''}})
{
    initFont();
    performConnections();
    setMouseTracking(true);

    setSyntaxStyle(QSyntaxStyle::defaultStyle());
}

void QCodeEditor::initFont()
{
    auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setFixedPitch(true);
    fnt.setPointSize(10);

    setFont(fnt);
}

void QCodeEditor::performConnections()
{
    connect(document(), &QTextDocument::blockCountChanged, this, &QCodeEditor::updateLineNumberAreaWidth);
    connect(document(), &QTextDocument::blockCountChanged, this, &QCodeEditor::updateBottomMargin);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) { m_lineNumberArea->update(); });

    connect(this, &QTextEdit::cursorPositionChanged, this, &QCodeEditor::updateExtraSelection1);
    connect(this, &QTextEdit::selectionChanged, this, &QCodeEditor::updateExtraSelection2);
}

void QCodeEditor::setHighlighter(QStyleSyntaxHighlighter *highlighter)
{
    if (m_highlighter)
    {
        m_highlighter->setDocument(nullptr);
    }

    m_highlighter = highlighter;

    if (m_highlighter)
    {
        m_highlighter->setSyntaxStyle(m_syntaxStyle);
        m_highlighter->setDocument(document());
    }
}

void QCodeEditor::setSyntaxStyle(QSyntaxStyle *style)
{
    m_syntaxStyle = style;

    m_lineNumberArea->setSyntaxStyle(m_syntaxStyle);

    if (m_highlighter)
    {
        m_highlighter->setSyntaxStyle(m_syntaxStyle);
    }

    updateStyle();
}

void QCodeEditor::updateStyle()
{
    if (m_highlighter)
    {
        m_highlighter->rehighlight();
    }

    if (m_syntaxStyle)
    {
        QString backgroundColor = m_syntaxStyle->getFormat("Text").background().color().name();
        QString textColor = m_syntaxStyle->getFormat("Text").foreground().color().name();
        QString selectionBackground = m_syntaxStyle->getFormat("Selection").background().color().name();

        setStyleSheet(QString("QTextEdit { background-color: %1; selection-background-color: %2; color: %3; }")
                          .arg(backgroundColor, selectionBackground, textColor));
    }

    updateExtraSelection1();
    updateExtraSelection2();
}

void QCodeEditor::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);

    updateLineGeometry();
    updateBottomMargin();
}

void QCodeEditor::changeEvent(QEvent *e)
{
    QTextEdit::changeEvent(e);
    if (e->type() == QEvent::FontChange)
        updateBottomMargin();
}

void QCodeEditor::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier)
    {
        const auto sizes = QFontDatabase::standardSizes();
        if (sizes.isEmpty())
        {
            qDebug() << "QFontDatabase::standardSizes() is empty";
            return;
        }
        int newSize = font().pointSize();
        if (e->angleDelta().y() > 0)
            newSize = qMin(newSize + 1, sizes.last());
        else if (e->angleDelta().y() < 0)
            newSize = qMax(newSize - 1, sizes.first());
        if (newSize != font().pointSize())
        {
            QFont newFont = font();
            newFont.setPointSize(newSize);
            setFont(newFont);
            Q_EMIT fontChanged(newFont);
        }
    }
    else
        QTextEdit::wheelEvent(e);
}

void QCodeEditor::updateLineGeometry()
{
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), m_lineNumberArea->sizeHint().width(), cr.height()));
}

void QCodeEditor::updateBottomMargin()
{
    auto doc = document();
    if (doc->blockCount() > 1)
    {
        // calling QTextFrame::setFrameFormat with an empty document makes the application crash
        auto rf = doc->rootFrame();
        auto format = rf->frameFormat();
        int documentMargin = doc->documentMargin();
        int bottomMargin = m_extraBottomMargin
                               ? qMax(documentMargin, viewport()->height() - fontMetrics().height()) - documentMargin
                               : documentMargin;
        if (format.bottomMargin() != bottomMargin)
        {
            format.setBottomMargin(bottomMargin);
            rf->setFrameFormat(format);
        }
    }
}

void QCodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(m_lineNumberArea->sizeHint().width(), 0, 0, 0);
}

void QCodeEditor::updateLineNumberArea(QRect rect)
{
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->sizeHint().width(), rect.height());
    updateLineGeometry();

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}

void QCodeEditor::updateExtraSelection1()
{
    extra1.clear();

    highlightCurrentLine();
    highlightParenthesis();

    setExtraSelections(extra1 + extra2 + extra_squiggles);
}

void QCodeEditor::updateExtraSelection2()
{
    extra2.clear();

    highlightOccurrences();

    setExtraSelections(extra1 + extra2 + extra_squiggles);
}

void QCodeEditor::indent()
{
    addInEachLineOfSelection(QRegularExpression("^"), m_replaceTab ? m_tabReplace : "\t");
}

void QCodeEditor::unindent()
{
    removeInEachLineOfSelection(QRegularExpression("^(\t| {1," + QString::number(tabReplaceSize()) + "})"), true);
}

void QCodeEditor::swapLineUp()
{
    auto cursor = textCursor();
    auto lines = toPlainText().remove('\r').split('\n');
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();
    bool cursorAtEnd = cursor.position() == selectionEnd;
    cursor.setPosition(selectionStart);
    int lineStart = cursor.blockNumber();
    cursor.setPosition(selectionEnd);
    int lineEnd = cursor.blockNumber();

    if (lineStart == 0)
        return;
    selectionStart -= lines[lineStart - 1].length() + 1;
    selectionEnd -= lines[lineStart - 1].length() + 1;
    lines.move(lineStart - 1, lineEnd);

    cursor.select(QTextCursor::Document);
    cursor.insertText(lines.join('\n'));

    if (cursorAtEnd)
    {
        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(selectionEnd);
        cursor.setPosition(selectionStart, QTextCursor::KeepAnchor);
    }

    setTextCursor(cursor);
}

void QCodeEditor::swapLineDown()
{
    auto cursor = textCursor();
    auto lines = toPlainText().remove('\r').split('\n');
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();
    bool cursorAtEnd = cursor.position() == selectionEnd;
    cursor.setPosition(selectionStart);
    int lineStart = cursor.blockNumber();
    cursor.setPosition(selectionEnd);
    int lineEnd = cursor.blockNumber();

    if (lineEnd == document()->blockCount() - 1)
        return;
    selectionStart += lines[lineEnd + 1].length() + 1;
    selectionEnd += lines[lineEnd + 1].length() + 1;
    lines.move(lineEnd + 1, lineStart);

    cursor.select(QTextCursor::Document);
    cursor.insertText(lines.join('\n'));

    if (cursorAtEnd)
    {
        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(selectionEnd);
        cursor.setPosition(selectionStart, QTextCursor::KeepAnchor);
    }

    setTextCursor(cursor);
}

void QCodeEditor::deleteLine()
{
    auto cursor = textCursor();
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();
    cursor.setPosition(selectionStart);
    int lineStart = cursor.blockNumber();
    cursor.setPosition(selectionEnd);
    int lineEnd = cursor.blockNumber();
    int columnNumber = textCursor().columnNumber();
    cursor.movePosition(QTextCursor::Start);
    if (lineEnd == document()->blockCount() - 1)
    {
        if (lineStart == 0)
        {
            cursor.select(QTextCursor::Document);
        }
        else
        {
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineStart - 1);
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        }
    }
    else
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineStart);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, lineEnd - lineStart + 1);
    }
    cursor.removeSelectedText();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                        qMin(columnNumber, cursor.block().text().length()));
    setTextCursor(cursor);
}

void QCodeEditor::duplicate()
{
    auto cursor = textCursor();
    if (cursor.hasSelection()) // duplicate the selection
    {
        auto text = cursor.selectedText();
        bool cursorAtEnd = cursor.selectionEnd() == cursor.position();
        cursor.insertText(text + text);
        if (cursorAtEnd)
        {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, text.length());
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, text.length());
        }
        else
        {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, text.length());
        }
    }
    else // duplicate the current line
    {
        int column = cursor.columnNumber();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        auto text = cursor.selectedText();
        cursor.insertText(text + "\n" + text);
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, column);
    }
    setTextCursor(cursor);
}

void QCodeEditor::toggleComment()
{
    if (m_highlighter == nullptr)
        return;
    QString comment = m_highlighter->commentLineSequence();
    if (comment.isEmpty())
        return;

    if (!removeInEachLineOfSelection(QRegularExpression("^\\s*(" + comment + " ?)"), false))
    {
        addInEachLineOfSelection(QRegularExpression("\\S|^\\s*$"), comment + " ");
    }
}

void QCodeEditor::toggleBlockComment()
{
    if (m_highlighter == nullptr)
        return;
    QString commentStart = m_highlighter->startCommentBlockSequence();
    QString commentEnd = m_highlighter->endCommentBlockSequence();

    if (commentStart.isEmpty() || commentEnd.isEmpty())
        return;

    auto cursor = textCursor();
    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
    bool cursorAtEnd = cursor.position() == endPos;
    auto text = cursor.selectedText();
    int pos1, pos2;
    if (text.indexOf(commentStart) == 0 && text.length() >= commentStart.length() + commentEnd.length() &&
        text.lastIndexOf(commentEnd) + commentEnd.length() == text.length())
    {
        insertPlainText(text.mid(commentStart.length(), text.length() - commentStart.length() - commentEnd.length()));
        pos1 = startPos;
        pos2 = endPos - commentStart.length() - commentEnd.length();
    }
    else
    {
        insertPlainText(commentStart + text + commentEnd);
        pos1 = startPos;
        pos2 = endPos + commentStart.length() + commentEnd.length();
    }
    if (cursorAtEnd)
    {
        cursor.setPosition(pos1);
        cursor.setPosition(pos2, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(pos2);
        cursor.setPosition(pos1, QTextCursor::KeepAnchor);
    }
    setTextCursor(cursor);
}

void QCodeEditor::highlightParenthesis()
{
    auto currentSymbol = charUnderCursor();
    auto prevSymbol = charUnderCursor(-1);

    for (auto &p : m_parentheses)
    {
        int direction;

        QChar counterSymbol;
        QChar activeSymbol;
        auto position = textCursor().position();

        if (p.left == currentSymbol)
        {
            direction = 1;
            counterSymbol = p.right;
            activeSymbol = currentSymbol;
        }
        else if (p.right == prevSymbol)
        {
            direction = -1;
            counterSymbol = p.left;
            activeSymbol = prevSymbol;
            position--;
        }
        else
        {
            continue;
        }

        auto counter = 1;

        while (counter != 0 && position > 0 && position < (document()->characterCount() - 1))
        {
            // Moving position
            position += direction;

            auto character = document()->characterAt(position);
            // Checking symbol under position
            if (character == activeSymbol)
            {
                ++counter;
            }
            else if (character == counterSymbol)
            {
                --counter;
            }
        }

        auto format = m_syntaxStyle->getFormat("Parentheses");

        // Found
        if (counter == 0)
        {
            ExtraSelection selection{};

            auto directionEnum = direction < 0 ? QTextCursor::MoveOperation::Left : QTextCursor::MoveOperation::Right;

            selection.format = format;
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            selection.cursor.movePosition(directionEnum, QTextCursor::MoveMode::MoveAnchor,
                                          qAbs(textCursor().position() - position));

            selection.cursor.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, 1);

            extra1.append(selection);

            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            selection.cursor.movePosition(directionEnum, QTextCursor::MoveMode::KeepAnchor, 1);

            extra1.append(selection);
        }

        break;
    }
}

void QCodeEditor::highlightCurrentLine()
{
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection{};

        selection.format = m_syntaxStyle->getFormat("CurrentLine");
        selection.format.setForeground(QBrush());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extra1.append(selection);
    }
}

void QCodeEditor::highlightOccurrences()
{
    auto cursor = textCursor();
    if (cursor.hasSelection())
    {
        auto text = cursor.selectedText();
        if (QRegularExpression(
                R"((?:[_a-zA-Z][_a-zA-Z0-9]*)|(?<=\b|\s|^)(?i)(?:(?:(?:(?:(?:\d+(?:'\d+)*)?\.(?:\d+(?:'\d+)*)(?:e[+-]?(?:\d+(?:'\d+)*))?)|(?:(?:\d+(?:'\d+)*)\.(?:e[+-]?(?:\d+(?:'\d+)*))?)|(?:(?:\d+(?:'\d+)*)(?:e[+-]?(?:\d+(?:'\d+)*)))|(?:0x(?:[0-9a-f]+(?:'[0-9a-f]+)*)?\.(?:[0-9a-f]+(?:'[0-9a-f]+)*)(?:p[+-]?(?:\d+(?:'\d+)*)))|(?:0x(?:[0-9a-f]+(?:'[0-9a-f]+)*)\.?(?:p[+-]?(?:\d+(?:'\d+)*))))[lf]?)|(?:(?:(?:[1-9]\d*(?:'\d+)*)|(?:0[0-7]*(?:'[0-7]+)*)|(?:0x[0-9a-f]+(?:'[0-9a-f]+)*)|(?:0b[01]+(?:'[01]+)*))(?:u?l{0,2}|l{0,2}u?)))(?=\b|\s|$))")
                .match(text)
                .captured() == text)
        {
            auto doc = document();
            cursor = doc->find(text, 0, QTextDocument::FindWholeWords | QTextDocument::FindCaseSensitively);
            while (!cursor.isNull())
            {
                if (cursor != textCursor())
                {
                    QTextEdit::ExtraSelection e;
                    e.cursor = cursor;
                    e.format.setBackground(m_syntaxStyle->getFormat("Selection").background());
                    extra2.push_back(e);
                }
                cursor = doc->find(text, cursor, QTextDocument::FindWholeWords | QTextDocument::FindCaseSensitively);
            }
        }
    }
}

void QCodeEditor::paintEvent(QPaintEvent *e)
{
    updateLineNumberArea(e->rect());
    QTextEdit::paintEvent(e);
}

int QCodeEditor::getFirstVisibleBlock()
{
    // Detect the first block for which bounding rect - once translated
    // in absolute coordinated - is contained by the editor's text area

    // Costly way of doing but since "blockBoundingGeometry(...)" doesn't
    // exists for "QTextEdit"...

    QTextCursor curs = QTextCursor(document());
    curs.movePosition(QTextCursor::Start);
    for (int i = 0; i < document()->blockCount(); ++i)
    {
        QTextBlock block = curs.block();

        QRect r1 = viewport()->geometry();
        QRect r2 = document()
                       ->documentLayout()
                       ->blockBoundingRect(block)
                       .translated(viewport()->geometry().x(),
                                   viewport()->geometry().y() - verticalScrollBar()->sliderPosition())
                       .toRect();

        if (r1.intersects(r2))
        {
            return i;
        }

        curs.movePosition(QTextCursor::NextBlock);
    }

    return 0;
}

bool QCodeEditor::proceedCompleterBegin(QKeyEvent *e)
{
    if (m_completer && m_completer->popup()->isVisible())
    {
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return true; // let the completer do default behavior
        default:
            break;
        }
    }

    // todo: Replace with modifiable QShortcut
    auto isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

    return !(!m_completer || !isShortcut);
}

void QCodeEditor::proceedCompleterEnd(QKeyEvent *e)
{
    auto ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

    if (!m_completer || (ctrlOrShift && e->text().isEmpty()) || e->key() == Qt::Key_Delete)
    {
        return;
    }

    static QString eow(R"(~!@#$%^&*()_+{}|:"<>?,./;'[]\-=)");

    auto isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
    auto completionPrefix = wordUnderCursor();

    if (!isShortcut && (e->text().isEmpty() || completionPrefix.length() < 2 || eow.contains(e->text().right(1))))
    {
        m_completer->popup()->hide();
        return;
    }

    if (completionPrefix != m_completer->completionPrefix())
    {
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }

    auto cursRect = cursorRect();
    cursRect.setWidth(m_completer->popup()->sizeHintForColumn(0) +
                      m_completer->popup()->verticalScrollBar()->sizeHint().width());

    m_completer->complete(cursRect);
}

void QCodeEditor::keyPressEvent(QKeyEvent *e)
{
    auto completerSkip = proceedCompleterBegin(e);

    if (!completerSkip)
    {
        if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && e->modifiers() != Qt::NoModifier)
        {
            QKeyEvent pureEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
            if (e->modifiers() == Qt::ControlModifier)
            {
                moveCursor(QTextCursor::EndOfBlock);
                keyPressEvent(&pureEnter);
                return;
            }
            else if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
            {
                if (textCursor().blockNumber() == 0)
                {
                    moveCursor(QTextCursor::StartOfBlock);
                    insertPlainText("\n");
                    moveCursor(QTextCursor::PreviousBlock);
                    moveCursor(QTextCursor::EndOfBlock);
                }
                else
                {
                    moveCursor(QTextCursor::PreviousBlock);
                    moveCursor(QTextCursor::EndOfBlock);
                    keyPressEvent(&pureEnter);
                }
                return;
            }
            else if (e->modifiers() == Qt::ShiftModifier)
            {
                keyPressEvent(&pureEnter);
                return;
            }
        }

        if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier)
        {
            if (textCursor().hasSelection())
            {
                indent();
                return;
            }

            auto c = charUnderCursor();
            for (auto p : qAsConst(m_parentheses))
            {
                if (p.tabJumpOut && c == p.right)
                {
                    moveCursor(QTextCursor::NextCharacter);
                    return;
                }
            }

            if (m_replaceTab)
            {
                insertPlainText(m_tabReplace);
                return;
            }
        }

        if (e->key() == Qt::Key_Backtab && e->modifiers() == Qt::ShiftModifier)
        {
            unindent();
            return;
        }

        if (e->key() == Qt::Key_Delete && e->modifiers() == Qt::ShiftModifier)
        {
            deleteLine();
            return;
        }

        // Auto indentation

        QString indentationSpaces = QRegularExpression("^\\s*")
                                        .match(document()->findBlockByNumber(textCursor().blockNumber()).text())
                                        .captured();

        // Have Qt Edior like behaviour, if {|} and enter is pressed indent the two
        // parenthesis
        if (m_autoIndentation && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
            e->modifiers() == Qt::NoModifier && charUnderCursor(-1) == '{' && charUnderCursor() == '}')
        {
            insertPlainText("\n" + indentationSpaces + (m_replaceTab ? m_tabReplace : "\t") + "\n" + indentationSpaces);

            for (int i = 0; i <= indentationSpaces.length(); ++i)
                moveCursor(QTextCursor::MoveOperation::Left);

            return;
        }

        // Auto-indent for single "{" without "}"
        if (m_autoIndentation && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
            e->modifiers() == Qt::NoModifier && charUnderCursor(-1) == '{')
        {
            insertPlainText("\n" + indentationSpaces + (m_replaceTab ? m_tabReplace : "\t"));
            setTextCursor(textCursor()); // scroll to the cursor
            return;
        }

        if (e->key() == Qt::Key_Backspace && e->modifiers() == Qt::NoModifier && !textCursor().hasSelection())
        {
            auto pre = charUnderCursor(-1);
            auto nxt = charUnderCursor();
            for (auto p : qAsConst(m_parentheses))
            {
                if (p.autoRemove && p.left == pre && p.right == nxt)
                {
                    auto cursor = textCursor();
                    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
                    cursor.removeSelectedText();
                    setTextCursor(textCursor()); // scroll to the cursor
                    return;
                }
            }

            if (textCursor().columnNumber() <= indentationSpaces.length() && textCursor().columnNumber() >= 1 &&
                !m_tabReplace.isEmpty())
            {
                auto cursor = textCursor();
                int realColumn = 0, newIndentLength = 0;
                for (int i = 0; i < cursor.columnNumber(); ++i)
                {
                    if (indentationSpaces[i] != '\t')
                        ++realColumn;
                    else
                    {
                        realColumn =
                            (realColumn + m_tabReplace.length()) / m_tabReplace.length() * m_tabReplace.length();
                    }
                    if (realColumn % m_tabReplace.length() == 0 && i < cursor.columnNumber() - 1)
                    {
                        newIndentLength = i + 1;
                    }
                }
                cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor,
                                    cursor.columnNumber() - newIndentLength);
                cursor.removeSelectedText();
                setTextCursor(textCursor()); // scroll to the cursor
                return;
            }
        }

        for (auto p : qAsConst(m_parentheses))
        {
            if (p.autoComplete)
            {
                auto cursor = textCursor();
                if (cursor.hasSelection())
                {
                    if (p.left == e->text())
                    {
                        // Add parentheses for selection
                        int startPos = cursor.selectionStart();
                        int endPos = cursor.selectionEnd();
                        bool cursorAtEnd = cursor.position() == endPos;
                        auto text = p.left + cursor.selectedText() + p.right;
                        insertPlainText(text);
                        if (cursorAtEnd)
                        {
                            cursor.setPosition(startPos + 1);
                            cursor.setPosition(endPos + 1, QTextCursor::KeepAnchor);
                        }
                        else
                        {
                            cursor.setPosition(endPos + 1);
                            cursor.setPosition(startPos + 1, QTextCursor::KeepAnchor);
                        }
                        setTextCursor(cursor);
                        return;
                    }
                }
                else
                {
                    if (p.right == e->text())
                    {
                        auto symbol = charUnderCursor();

                        if (symbol == p.right)
                        {
                            moveCursor(QTextCursor::NextCharacter);
                            return;
                        }
                    }

                    if (p.left == e->text())
                    {
                        insertPlainText(QString(p.left) + p.right);
                        moveCursor(QTextCursor::PreviousCharacter);
                        return;
                    }
                }
            }
        }

        if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && e->modifiers() == Qt::NoModifier)
        {
            insertPlainText("\n" + indentationSpaces.left(textCursor().columnNumber()));
            setTextCursor(textCursor()); // scroll to the cursor
            return;
        }

        if (e->key() == Qt::Key_Escape && textCursor().hasSelection())
        {
            auto cursor = textCursor();
            cursor.clearSelection();
            setTextCursor(cursor);
        }

        QTextEdit::keyPressEvent(e);
    }

    proceedCompleterEnd(e);
}

void QCodeEditor::setAutoIndentation(bool enabled)
{
    m_autoIndentation = enabled;
}

void QCodeEditor::setParentheses(const QVector<Parenthesis> &parentheses)
{
    m_parentheses = parentheses;
}

void QCodeEditor::setExtraBottomMargin(bool enabled)
{
    m_extraBottomMargin = enabled;
    updateBottomMargin();
}

bool QCodeEditor::autoIndentation() const
{
    return m_autoIndentation;
}

void QCodeEditor::setTabReplace(bool enabled)
{
    m_replaceTab = enabled;
}

bool QCodeEditor::tabReplace() const
{
    return m_replaceTab;
}

void QCodeEditor::setTabReplaceSize(int val)
{
    m_tabReplace.fill(' ', val);
#if QT_VERSION >= 0x050B00
    setTabStopDistance(fontMetrics().horizontalAdvance(QString(val * 1000, ' ')) / 1000.0);
#elif QT_VERSION == 0x050A00
    setTabStopDistance(fontMetrics().width(QString(val * 1000, ' ')) / 1000.0);
#else
    setTabStopWidth(fontMetrics().width(QString(val * 1000, ' ')) / 1000.0);
#endif
}

int QCodeEditor::tabReplaceSize() const
{
    return m_tabReplace.size();
}

void QCodeEditor::setCompleter(QCompleter *completer)
{
    if (m_completer)
    {
        disconnect(m_completer, nullptr, this, nullptr);
    }

    m_completer = completer;

    if (!m_completer)
    {
        return;
    }

    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::CompletionMode::PopupCompletion);

    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, &QCodeEditor::insertCompletion);
}

void QCodeEditor::focusInEvent(QFocusEvent *e)
{
    if (m_completer)
    {
        m_completer->setWidget(this);
    }

    QTextEdit::focusInEvent(e);
}

bool QCodeEditor::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto *helpEvent = dynamic_cast<QHelpEvent *>(event);
        auto point = helpEvent->pos();
        point.setX(point.x() - m_lineNumberArea->geometry().right());
        QTextCursor cursor = cursorForPosition(point);

        auto lineNumber = cursor.blockNumber() + 1;

        QTextCursor copyCursor(cursor);
        copyCursor.movePosition(QTextCursor::StartOfBlock);

        auto blockPositionStart = cursor.positionInBlock() - copyCursor.positionInBlock();
        QPair<int, int> positionOfTooltip{lineNumber, blockPositionStart};

        QString text;
        for (auto const &e : qAsConst(m_squiggler))
        {
            if (e.m_startPos <= positionOfTooltip && e.m_stopPos >= positionOfTooltip)
            {
                if (text.isEmpty())
                    text = e.m_tooltipText;
                else
                    text += "; " + e.m_tooltipText;
            }
        }

        if (text.isEmpty())
            QToolTip::hideText();
        else
            QToolTip::showText(helpEvent->globalPos(), text);

        return true;
    }
    return QTextEdit::event(event);
}

void QCodeEditor::insertCompletion(const QString &s)
{
    if (m_completer->widget() != this)
    {
        return;
    }

    auto tc = textCursor();
    tc.select(QTextCursor::SelectionType::WordUnderCursor);
    tc.insertText(s);
    setTextCursor(tc);
}

QCompleter *QCodeEditor::completer() const
{
    return m_completer;
}

void QCodeEditor::squiggle(SeverityLevel level, QPair<int, int> start, QPair<int, int> stop,
                           const QString &tooltipMessage)
{
    if (stop < start)
        return;

    SquiggleInformation info(start, stop, tooltipMessage);
    m_squiggler.push_back(info);

    auto cursor = textCursor();

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, start.first - 1);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start.second);

    if (stop.first > start.first)
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, stop.first - start.first);

    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, stop.second);

    QTextCharFormat newcharfmt = currentCharFormat();
    newcharfmt.setFontUnderline(true);

    switch (level)
    {
    case SeverityLevel::Error:
        newcharfmt.setUnderlineColor(m_syntaxStyle->getFormat("Error").underlineColor());
        newcharfmt.setUnderlineStyle(m_syntaxStyle->getFormat("Error").underlineStyle());
        break;
    case SeverityLevel::Warning:
        newcharfmt.setUnderlineColor(m_syntaxStyle->getFormat("Warning").underlineColor());
        newcharfmt.setUnderlineStyle(m_syntaxStyle->getFormat("Warning").underlineStyle());
        break;
    case SeverityLevel::Information:
        newcharfmt.setUnderlineColor(m_syntaxStyle->getFormat("Warning").underlineColor());
        newcharfmt.setUnderlineStyle(QTextCharFormat::DotLine);
        break;
    case SeverityLevel::Hint:
        newcharfmt.setUnderlineColor(m_syntaxStyle->getFormat("Text").foreground().color());
        newcharfmt.setUnderlineStyle(QTextCharFormat::DotLine);
    }

    extra_squiggles.push_back({cursor, newcharfmt});

    m_lineNumberArea->lint(level, start.first, stop.first);

    setExtraSelections(extra1 + extra2 + extra_squiggles);
}

void QCodeEditor::clearSquiggle()
{
    if (m_squiggler.empty())
        return;

    m_squiggler.clear();
    extra_squiggles.clear();

    m_lineNumberArea->clearLint();

    setExtraSelections(extra1 + extra2);
}

QChar QCodeEditor::charUnderCursor(int offset) const
{
    auto block = textCursor().blockNumber();
    auto index = textCursor().positionInBlock();
    auto text = document()->findBlockByNumber(block).text();

    index += offset;

    if (index < 0 || index >= text.size())
    {
        return {};
    }

    return text[index];
}

QString QCodeEditor::wordUnderCursor() const
{
    auto tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void QCodeEditor::insertFromMimeData(const QMimeData *source)
{
    insertPlainText(source->text());
}

bool QCodeEditor::removeInEachLineOfSelection(const QRegularExpression &regex, bool force)
{
    auto cursor = textCursor();
    auto lines = toPlainText().remove('\r').split('\n');
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();
    bool cursorAtEnd = cursor.position() == selectionEnd;
    cursor.setPosition(selectionStart);
    int lineStart = cursor.blockNumber();
    cursor.setPosition(selectionEnd);
    int lineEnd = cursor.blockNumber();
    QString newText;
    QTextStream stream(&newText);
    int deleteTotal = 0, deleteFirst = 0;
    for (int i = lineStart; i <= lineEnd; ++i)
    {
        auto line = lines[i];
        auto match = regex.match(line).captured(1);
        int len = match.length();
        if (len == 0 && !force)
            return false;
        if (i == lineStart)
            deleteFirst = len;
        deleteTotal += len;
        stream << line.remove(line.indexOf(match), len);
        if (i != lineEnd)
#if QT_VERSION >= 0x50E00
            stream << Qt::endl;
#else
            stream << endl;
#endif
    }
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineStart);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, lineEnd - lineStart);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.insertText(newText);
    cursor.setPosition(qMax(0, selectionStart - deleteFirst));
    if (cursor.blockNumber() < lineStart)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineStart - cursor.blockNumber());
        cursor.movePosition(QTextCursor::StartOfBlock);
    }
    int pos = cursor.position();
    cursor.setPosition(selectionEnd - deleteTotal);
    if (cursor.blockNumber() < lineEnd)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineEnd - cursor.blockNumber());
        cursor.movePosition(QTextCursor::StartOfBlock);
    }
    int pos2 = cursor.position();
    if (cursorAtEnd)
    {
        cursor.setPosition(pos);
        cursor.setPosition(pos2, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(pos2);
        cursor.setPosition(pos, QTextCursor::KeepAnchor);
    }
    setTextCursor(cursor);
    return true;
}

void QCodeEditor::addInEachLineOfSelection(const QRegularExpression &regex, const QString &str)
{
    auto cursor = textCursor();
    auto lines = toPlainText().remove('\r').split('\n');
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();
    bool cursorAtEnd = cursor.position() == selectionEnd;
    cursor.setPosition(selectionStart);
    int lineStart = cursor.blockNumber();
    cursor.setPosition(selectionEnd);
    int lineEnd = cursor.blockNumber();
    QString newText;
    QTextStream stream(&newText);
    for (int i = lineStart; i <= lineEnd; ++i)
    {
        auto line = lines[i];
        stream << line.insert(line.indexOf(regex), str);
        if (i != lineEnd)
#if QT_VERSION >= 0x50E00
            stream << Qt::endl;
#else
            stream << endl;
#endif
    }
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineStart);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, lineEnd - lineStart);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.insertText(newText);
    int pos = selectionStart + str.length();
    int pos2 = selectionEnd + str.length() * (lineEnd - lineStart + 1);
    if (cursorAtEnd)
    {
        cursor.setPosition(pos);
        cursor.setPosition(pos2, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(pos2);
        cursor.setPosition(pos, QTextCursor::KeepAnchor);
    }
    setTextCursor(cursor);
}
