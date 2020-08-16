#pragma once

// QCodeEditor
#include <QStyleSyntaxHighlighter> // Required for inheritance

// Qt
#include <QRegularExpression>
#include <QVector>

/**
 * @brief Class, that describes XML code
 * highlighter.
 */
class QXMLHighlighter : public QStyleSyntaxHighlighter
{
    Q_OBJECT
  public:
    /**
     * @brief Constructor.
     * @param document Pointer to document.
     */
    explicit QXMLHighlighter(QTextDocument *document = nullptr);

  protected:
    void highlightBlock(const QString &text) override;

  private:
    void highlightByRegex(const QTextCharFormat &format, const QRegularExpression &regex, const QString &text);

    QVector<QRegularExpression> m_xmlKeywordRegexes;
    QRegularExpression m_xmlElementRegex;
    QRegularExpression m_xmlAttributeRegex;
    QRegularExpression m_xmlValueRegex;
    QRegularExpression m_xmlCommentBeginRegex;
    QRegularExpression m_xmlCommentEndRegex;
};
