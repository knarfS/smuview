// QCodeEditor
#include <QStyleSyntaxHighlighter>

QStyleSyntaxHighlighter::QStyleSyntaxHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document), m_syntaxStyle(nullptr), m_commentLineSequence(), m_startCommentBlockSequence(),
      m_endCommentBlockSequence()
{
}

void QStyleSyntaxHighlighter::setSyntaxStyle(QSyntaxStyle *style)
{
    m_syntaxStyle = style;
}

QSyntaxStyle *QStyleSyntaxHighlighter::syntaxStyle() const
{
    return m_syntaxStyle;
}

QString QStyleSyntaxHighlighter::commentLineSequence() const
{
    return m_commentLineSequence;
}

void QStyleSyntaxHighlighter::setCommentLineSequence(const QString &commentLineSequence)
{
    m_commentLineSequence = commentLineSequence;
}

QString QStyleSyntaxHighlighter::startCommentBlockSequence() const
{
    return m_startCommentBlockSequence;
}

void QStyleSyntaxHighlighter::setStartCommentBlockSequence(const QString &startCommentBlockSequence)
{
    m_startCommentBlockSequence = startCommentBlockSequence;
}

QString QStyleSyntaxHighlighter::endCommentBlockSequence() const
{
    return m_endCommentBlockSequence;
}

void QStyleSyntaxHighlighter::setEndCommentBlockSequence(const QString &endCommentBlockSequence)
{
    m_endCommentBlockSequence = endCommentBlockSequence;
}
