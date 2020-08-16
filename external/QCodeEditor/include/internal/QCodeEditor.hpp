#pragma once

// Qt
#include <QTextEdit> // Required for inheritance

class QCompleter;
class QLineNumberArea;
class QSyntaxStyle;
class QStyleSyntaxHighlighter;
class QFramedTextAttribute;

/**
 * @brief Class, that describes code editor.
 */
class QCodeEditor : public QTextEdit
{
    Q_OBJECT

  public:
    /**
     * @brief The SeverityLevel enum
     * @note the order should be: the bigger the more important
     */
    enum class SeverityLevel
    {
        Hint,
        Information,
        Warning,
        Error
    };

    struct Parenthesis
    {
        QChar left, right;
        bool autoComplete, autoRemove, tabJumpOut;

        Parenthesis(QChar l = '(', QChar r = ')', bool complete = true, bool remove = true, bool jumpout = true)
            : left(l), right(r), autoComplete(complete), autoRemove(remove), tabJumpOut(jumpout)
        {
        }
    };

    /**
     * @brief Constructor.
     * @param widget Pointer to parent widget.
     */
    explicit QCodeEditor(QWidget *widget = nullptr);

    // Disable copying
    QCodeEditor(const QCodeEditor &) = delete;
    QCodeEditor &operator=(const QCodeEditor &) = delete;

    /**
     * @brief Method for getting first visible block
     * index.
     * @return Index.
     */
    int getFirstVisibleBlock();

    /**
     * @brief Method for setting highlighter.
     * @param highlighter Pointer to syntax highlighter.
     */
    void setHighlighter(QStyleSyntaxHighlighter *highlighter);

    /**
     * @brief Method for setting syntax sty.e.
     * @param style Pointer to syntax style.
     */
    void setSyntaxStyle(QSyntaxStyle *style);

    /**
     * @brief Method for setting tab replacing
     * enabled.
     */
    void setTabReplace(bool enabled);

    /**
     * @brief Method for getting is tab replacing enabled.
     * Default value: true
     */
    bool tabReplace() const;

    /**
     * @brief Method for setting amount of spaces, that will
     * replace tab.
     * @param val Number of spaces.
     */
    void setTabReplaceSize(int val);

    /**
     * @brief Method for getting number of spaces, that will
     * replace tab if `tabReplace` is true.
     * Default: 4
     */
    int tabReplaceSize() const;

    /**
     * @brief Method for setting auto indentation enabled.
     */
    void setAutoIndentation(bool enabled);

    /**
     * @brief Method for setting the parentheses.
     */
    void setParentheses(const QVector<Parenthesis> &parentheses);

    /**
     * @brief Method for setting extra bottom margin enabled.
     */
    void setExtraBottomMargin(bool enabled);

    /**
     * @brief Method for getting is auto indentation enabled.
     * Default: true
     */
    bool autoIndentation() const;

    /**
     * @brief Method for setting completer.
     * @param completer Pointer to completer object.
     */
    void setCompleter(QCompleter *completer);

    /**
     * @brief Method for getting completer.
     * @return Pointer to completer.
     */
    QCompleter *completer() const;

    /**
     * @brief squiggle Puts a underline squiggle under text ranges in Editor
     * @param level defines the color of the underline depending upon the severity
     * @param tooltipMessage The tooltip hover message to show when over selection.
     * @note QPair<int, int>: first -> Line number in 1-based indexing
     *                        second -> Character number in 0-based indexing
     */
    void squiggle(SeverityLevel level, QPair<int, int>, QPair<int, int>, const QString &tooltipMessage);

    /**
     * @brief clearSquiggle, Clears complete squiggle from editor
     */
    void clearSquiggle();

  Q_SIGNALS:
    /**
     * @brief Signal, the font is changed by the wheel event.
     */
    void fontChanged(const QFont &newFont);

  public Q_SLOTS:

    /**
     * @brief Slot, that performs insertion of
     * completion info into code.
     * @param s Data.
     */
    void insertCompletion(const QString &s);

    /**
     * @brief Slot, that performs update of
     * internal editor viewport based on line
     * number area width.
     */
    void updateLineNumberAreaWidth(int);

    /**
     * @brief Slot, that performs update of some
     * part of line number area.
     * @param rect Area that has to be updated.
     */
    void updateLineNumberArea(QRect rect);

    /**
     * @brief Slot, that will proceed extra selection
     * for current cursor position.
     */
    void updateExtraSelection1();
    void updateExtraSelection2();

    /**
     * @brief Slot, that will update editor style.
     */
    void updateStyle();

    /**
     * @brief Slot, that indent the selected lines.
     */
    void indent();

    /**
     * @brief Slot, that unindent the selected lines.
     */
    void unindent();

    /**
     * @brief Slot, that swap the selected lines up.
     */
    void swapLineUp();

    /**
     * @brief Slot, that swap the selected lines down.
     */
    void swapLineDown();

    /**
     * @brief Slot, that delete the selected lines.
     */
    void deleteLine();

    /**
     * @brief Slot, that duplicate the current line or the selection.
     */
    void duplicate();

    /**
     * @brief Slot, that toggle single line comment of the
     * selected lines.
     */
    void toggleComment();

    /**
     * @brief Slot, that toggle block comment of the selection.
     */
    void toggleBlockComment();

  protected:
    /**
     * @brief Method, that's called on any text insertion of
     * mimedata into editor. If it's text - it inserts text
     * as plain text.
     */
    void insertFromMimeData(const QMimeData *source) override;

    /**
     * @brief Method, that's called on editor painting. This
     * method if overloaded for line number area redraw.
     */
    void paintEvent(QPaintEvent *e) override;

    /**
     * @brief Method, that's called on any widget resize.
     * This method if overloaded for line number area
     * resizing.
     */
    void resizeEvent(QResizeEvent *e) override;

    /**
     * @brief Method, update the bottom margin when the font changes.
     */
    void changeEvent(QEvent *e) override;

    /**
     * @brief Method, update the font size when the wheel is rotated with Ctrl pressed
     */
    void wheelEvent(QWheelEvent *e) override;

    /**
     * @brief Method, that's called on any key press, posted
     * into code editor widget. This method is overloaded for:
     *
     * 1. Completion
     * 2. Tab to spaces
     * 3. Low indentation
     * 4. Auto parenthesis
     */
    void keyPressEvent(QKeyEvent *e) override;

    /**
     * @brief Method, that's called on focus into widget.
     * It's required for setting this widget to set
     * completer.
     */
    void focusInEvent(QFocusEvent *e) override;

    /**
     * @brief Method for tooltip generation
     */
    bool event(QEvent *e) override;

  private Q_SLOTS:
    /**
     * @brief Slot, that updates the bottom margin.
     */
    void updateBottomMargin();

  private:
    /**
     * @brief Method for initializing default
     * monospace font.
     */
    void initFont();

    /**
     * @brief Method for performing connection
     * of objects.
     */
    void performConnections();

    /**
     * @brief Method for updating geometry of line number area.
     */
    void updateLineGeometry();

    /**
     * @brief Method, that performs completer processing.
     * Returns true if event has to be dropped.
     * @param e Pointer to key event.
     * @return Shall event be dropped.
     */
    bool proceedCompleterBegin(QKeyEvent *e);
    void proceedCompleterEnd(QKeyEvent *e);

    /**
     * @brief Method for getting character under
     * cursor.
     * @param offset Offset to cursor.
     */
    QChar charUnderCursor(int offset = 0) const;

    /**
     * @brief Method for getting word under
     * cursor.
     * @return Word under cursor.
     */
    QString wordUnderCursor() const;

    /**
     * @brief Method, that adds highlighting of
     * currently selected line to extra selection list.
     */
    void highlightCurrentLine();

    /**
     * @brief Method, that adds highlighting of
     * parenthesis if available.
     */
    void highlightParenthesis();

    void highlightOccurrences();

    /**
     * @brief Method for remove the first group of regex
     * in each line of the selection.
     * @param regex remove its first group
     * @param force if true, remove regardless of whether
     * all lines are begun with regex; if false remove
     * only when all lines are begun with regex.
     * @return if regex is removed
     */
    bool removeInEachLineOfSelection(const QRegularExpression &regex, bool force);

    /**
     * @brief Method for add the str at the begin of regex
     * in each line of the selection.
     * @param regex add at the begin of its match
     * @param str string to add
     */
    void addInEachLineOfSelection(const QRegularExpression &regex, const QString &str);

    /**
     * @brief The SquiggleInformation struct, Line number will be index of vector+1;
     */
    struct SquiggleInformation
    {
        SquiggleInformation() = default;

        SquiggleInformation(QPair<int, int> start, QPair<int, int> stop, const QString &text)
            : m_startPos(start), m_stopPos(stop), m_tooltipText(text)
        {
        }

        QPair<int, int> m_startPos;
        QPair<int, int> m_stopPos;
        QString m_tooltipText;
    };

    QStyleSyntaxHighlighter *m_highlighter;
    QSyntaxStyle *m_syntaxStyle;
    QLineNumberArea *m_lineNumberArea;
    QCompleter *m_completer;

    bool m_autoIndentation;
    bool m_replaceTab;
    bool m_extraBottomMargin;
    QString m_tabReplace;

    QList<QTextEdit::ExtraSelection> extra1, extra2, extra_squiggles;

    QVector<SquiggleInformation> m_squiggler;

    QVector<Parenthesis> m_parentheses;
};
