// QCodeEditor
#include <QJavaHighlighter>
#include <QLanguage>
#include <QSyntaxStyle>

// Qt
#include <QFile>

QJavaHighlighter::QJavaHighlighter(QTextDocument *document)
    : QStyleSyntaxHighlighter(document), m_highlightRules(), m_commentStartPattern(QRegularExpression(R"(/\*)")),
      m_commentEndPattern(QRegularExpression(R"(\*/)"))
{
    Q_INIT_RESOURCE(qcodeeditor_resources);

    QFile fl(":/languages/java.xml");

    if (!fl.open(QIODevice::ReadOnly))
    {
        return;
    }

    QLanguage language(&fl);

    if (!language.isLoaded())
    {
        return;
    }

    auto keys = language.keys();
    for (auto &&key : keys)
    {
        auto names = language.names(key);
        for (auto &&name : names)
        {
            m_highlightRules.append({QRegularExpression(QString(R"(\b%1\b)").arg(name)), key});
        }
    }

    // Numbers
    m_highlightRules.append(
        {QRegularExpression(
             R"((?<=\b|\s|^)(?i)(?:(?:[0-9]+\.[0-9]*(?:e[+-]?[0-9]+)?[fd]?)|(?:\.[0-9]+(?:e[+-]?[0-9]+)?[fd]?)|(?:[0-9]+(?:e[+-]?[0-9]+)[fd]?)|(?:[0-9]+(?:e[+-]?[0-9]+)?[fd])|(?:(?:(?:0x[0-9a-f]+\.?)|(?:0x[0-9a-f]*\.[0-9a-f]+))p[+-]?[0-9]+[fd]?)|(?:0)|(?:[1-9][0-9]*)|(?:0x[0-9a-f]+)|(?:0[0-7]+))(?=\b|\s|$))"),
         "Number"});

    // Strings
    m_highlightRules.append({QRegularExpression(R"("[^\n"]*")"), "String"});

    // Single line
    m_highlightRules.append({QRegularExpression(R"(//[^\n]*)"), "Comment"});

    // Comment sequences for toggling support
    m_commentLineSequence = "//";
    m_startCommentBlockSequence = "/*";
    m_endCommentBlockSequence = "*/";
}

void QJavaHighlighter::highlightBlock(const QString &text)
{
    for (auto &rule : m_highlightRules)
    {
        auto matchIterator = rule.pattern.globalMatch(text);

        while (matchIterator.hasNext())
        {
            auto match = matchIterator.next();

            setFormat(match.capturedStart(), match.capturedLength(), syntaxStyle()->getFormat(rule.formatName));
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        startIndex = text.indexOf(m_commentStartPattern);
    }

    while (startIndex >= 0)
    {
        auto match = m_commentEndPattern.match(text, startIndex);

        int endIndex = match.capturedStart();
        int commentLength = 0;

        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }

        setFormat(startIndex, commentLength, syntaxStyle()->getFormat("Comment"));
        startIndex = text.indexOf(m_commentStartPattern, startIndex + commentLength);
    }
}
