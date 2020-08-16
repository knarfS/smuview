#pragma once

// QCodeEditor
#include <QHighlightBlockRule>
#include <QHighlightRule>
#include <QStyleSyntaxHighlighter> // Required for inheritance

// Qt
#include <QRegularExpression>
#include <QVector>

class QSyntaxStyle;

/**
 * @brief Class, that describes Glsl code
 * highlighter.
 */
class QPythonHighlighter : public QStyleSyntaxHighlighter
{
    Q_OBJECT
  public:
    /**
     * @brief Constructor.
     * @param document Pointer to document.
     */
    explicit QPythonHighlighter(QTextDocument *document = nullptr);

  protected:
    void highlightBlock(const QString &text) override;

  private:
    QVector<QHighlightRule> m_highlightRules;
    QVector<QHighlightBlockRule> m_highlightBlockRules;

    QRegularExpression m_includePattern;
    QRegularExpression m_functionPattern;
    QRegularExpression m_defTypePattern;
};
