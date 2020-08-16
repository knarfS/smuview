#pragma once

#include <QHighlightRule>
#include <QStyleSyntaxHighlighter> // Required for inheritance

// Qt
#include <QRegularExpression>
#include <QVector>

class QSyntaxStyle;

/**
 * @brief Derived to implement highlighting of JavaScript code.
 */
class QJSHighlighter : public QStyleSyntaxHighlighter
{
    Q_OBJECT

  public:
    /**
     * @brief Constructs a new instance of a JavaScript highlighter.
     * @param document The text document to be highlighted.
     * This may be a null pointer.
     */
    explicit QJSHighlighter(QTextDocument *document = nullptr);

  protected:
    void highlightBlock(const QString &text) override;

  private:
    QVector<QHighlightRule> m_highlightRules;

    QRegularExpression m_commentStartPattern;
    QRegularExpression m_commentEndPattern;
};
