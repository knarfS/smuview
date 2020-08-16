#pragma once

// Qt
#include <QSyntaxHighlighter> // Required for inheritance

class QSyntaxStyle;

/**
 * @brief Class, that descrubes highlighter with
 * syntax style.
 */
class QStyleSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

  public:
    /**
     * @brief Constructor.
     * @param document Pointer to text document.
     */
    explicit QStyleSyntaxHighlighter(QTextDocument *document = nullptr);

    // Disable copying
    QStyleSyntaxHighlighter(const QStyleSyntaxHighlighter &) = delete;
    QStyleSyntaxHighlighter &operator=(const QStyleSyntaxHighlighter &) = delete;

    /**
     * @brief Method for setting syntax style.
     * @param style Pointer to syntax style.
     */
    void setSyntaxStyle(QSyntaxStyle *style);

    /**
     * @brief Method for getting syntax style.
     * @return Pointer to syntax style. May be nullptr.
     */
    QSyntaxStyle *syntaxStyle() const;

    /**
     * @brief Method for getting a sequence that marks a comment line.
     * @return QString containing a sequence that marks a comment line.
     * @details Returned value can be empty meaning that this language doesn't
     * support single-line comments
     */
    QString commentLineSequence() const;

    /**
     * @brief Method to set a sequence that marks a comment line.
     * @param commentLineSequence a sequence that marks a comment line. Can be empty.
     */
    void setCommentLineSequence(const QString &commentLineSequence);

    /**
     * @brief Method for getting a sequence that marks a start of a multi line comment block.
     * @return QString containing a sequence that marks a multi line comment block.
     * @details Returned value can be empty meaning that this language doesn't
     * support multi line comments
     */
    QString startCommentBlockSequence() const;

    /**
     * @brief Method to set a sequence that marks a start of a multi line comment block.
     * @param commentLineSequence a sequence that marks a start of a multi line comment block. Can be empty.
     */
    void setStartCommentBlockSequence(const QString &startCommentBlockSequence);

    /**
     * @brief Method for getting a sequence that marks a end of a multi line comment block.
     * @return QString containing a sequence that marks an end multi line comment block.
     * @details Returned value can be empty meaning that this language doesn't
     * support multi line comments.
     */
    QString endCommentBlockSequence() const;

    /**
     * @brief Method to set a sequence that marks an end of a multi line comment block.
     * @param commentLineSequence a sequence that marks an end of a multi line comment block. Can be empty.
     */
    void setEndCommentBlockSequence(const QString &endCommentBlockSequence);

  private:
    QSyntaxStyle *m_syntaxStyle;

  protected:
    QString m_commentLineSequence;
    QString m_startCommentBlockSequence;
    QString m_endCommentBlockSequence;
};
