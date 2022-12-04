/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QShowEvent>
#include <QTextEdit>
#include <QtGui>

#include "findreplaceform.h"
#include "ui_findreplaceform.h"

#define TEXT_TO_FIND "textToFind"
#define TEXT_TO_REPLACE "textToReplace"
#define DOWN_RADIO "downRadio"
#define UP_RADIO "upRadio"
#define CASE_CHECK "caseCheck"
#define WHOLE_CHECK "wholeCheck"
#define REGEXP_CHECK "regexpCheck"

#define DEBUG_FIND 0 // Set to '1' to enable debugging of 'find'

FindReplaceForm::FindReplaceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindReplaceForm),
    textEdit(nullptr)
{
    ui->setupUi(this);

    ui->findButton->setAutoDefault(false);
    ui->replaceButton->setAutoDefault(false);
    ui->replaceAllButton->setAutoDefault(false);

    ui->errorLabel->setText("");

    connect(ui->textToFind, &QLineEdit::textChanged,
        this, &FindReplaceForm::textToFindChanged);
    connect(ui->textToFind, &QLineEdit::textChanged,
        this, &FindReplaceForm::validateRegExp);

    connect(ui->regexCheckBox, &QCheckBox::toggled,
        this, &FindReplaceForm::regexpSelected);

    connect(ui->findButton, &QPushButton::clicked,
        this, QOverload<>::of(&FindReplaceForm::find));

    connect(ui->replaceButton, &QPushButton::clicked,
        this, &FindReplaceForm::replace);
    connect(ui->replaceAllButton, &QPushButton::clicked,
        this, &FindReplaceForm::replaceAll);

    connect(ui->textToFind, &QLineEdit::returnPressed,
        ui->findButton, &QPushButton::click);
    connect(ui->textToReplace, &QLineEdit::returnPressed,
        ui->replaceButton, &QPushButton::click);
}

FindReplaceForm::~FindReplaceForm()
{
    delete ui;
}

void FindReplaceForm::hideReplaceWidgets()
{
    ui->replaceLabel->setVisible(false);
    ui->textToReplace->setVisible(false);
    ui->replaceButton->setVisible(false);
    ui->replaceAllButton->setVisible(false);
}

void FindReplaceForm::setTextEdit(QTextEdit *textEdit_)
{
    if (textEdit != textEdit_)
    {
        disconnect(selectionChangeConnection);
        ui->replaceButton->setEnabled(false);
        textEdit = textEdit_;
        validateRegExp(ui->textToFind->text());
        if (textEdit) {
            selectionChangeConnection =
                connect(textEdit, &QTextEdit::selectionChanged,
                    this, &FindReplaceForm::onSelectionChanged);
        }
    }
}

void FindReplaceForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FindReplaceForm::showEvent(QShowEvent *event)
{
    showError(QString());
    showMessage(QString());
    QWidget::showEvent(event);
}

void FindReplaceForm::textToFindChanged()
{
    ui->findButton->setEnabled(ui->textToFind->text().size() > 0);
    ui->replaceButton->setEnabled(false);
    ui->replaceAllButton->setEnabled(ui->textToFind->text().size() > 0);
}

void FindReplaceForm::regexpSelected(bool sel)
{
    if (sel)
        validateRegExp(ui->textToFind->text());
    else
        validateRegExp("");
}

void FindReplaceForm::onSelectionChanged()
{
    ui->replaceButton->setEnabled(false);
}

void FindReplaceForm::validateRegExp(const QString &text)
{
    if (!ui->regexCheckBox->isChecked() || text.size() == 0)
    {
        ui->errorLabel->setText("");
        return; // nothing to validate
    }

    QRegExp reg(text, (ui->caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));

    if (reg.isValid())
    {
        showError("");
    }
    else
    {
        showError(reg.errorString());
    }
}

void FindReplaceForm::showError(const QString &error)
{
    if (error.isEmpty())
    {
        ui->errorLabel->setText("");
    }
    else
    {
        ui->errorLabel->setText("<span style=\" font-weight:600; color:#ff0000;\">" + error + "</span>");
    }
}

void FindReplaceForm::showMessage(const QString &message)
{
    if (message.isEmpty())
    {
        ui->errorLabel->setText("");
    }
    else
    {
        ui->errorLabel->setText("<span style=\" font-weight:600; color:green;\">" + message + "</span>");
    }
}

void FindReplaceForm::setTextToFind(const QString &strText)
{
    ui->textToFind->selectAll();
    ui->textToFind->insert(strText);
}

void FindReplaceForm::find()
{
    find(ui->downRadioButton->isChecked());
}

void FindReplaceForm::find(bool next)
{
    if (!textEdit)
    {
        showError("No active editor");
        return;
    }

    // backward search
    bool back = !next;

    const QString &toSearch = ui->textToFind->text();

    bool result = false;

    // Check the cursor for wrap:
    textCursor = textEdit->textCursor();
    if (!textCursor.hasSelection())
    {
        if (next && textCursor.atEnd())
        {
            textCursor.movePosition(QTextCursor::Start);
        }
        else if (back && textCursor.atStart())
        {
            textCursor.movePosition(QTextCursor::End);
        }
    }
    textEdit->setTextCursor(textCursor);

    QTextDocument::FindFlags flags;

    if (back)
        flags |= QTextDocument::FindBackward;
    if (ui->caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->wholeCheckBox->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (ui->regexCheckBox->isChecked())
    {
        QRegExp reg(toSearch, (ui->caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));

#if (DEBUG_FIND)
        qDebug() << "searching for regexp: " << reg.pattern();
#endif

        textCursor = textEdit->document()->find(reg, textCursor, flags);
        if (!textCursor.isNull())
            textEdit->setTextCursor(textCursor);
        result = (!textCursor.isNull());
    }
    else
    {
#if (DEBUG_FIND)
        qDebug() << "searching for: " << toSearch;
#endif

        result = textEdit->find(toSearch, flags);
    }

    if (result)
    {
        ui->replaceButton->setEnabled(true);
        showError("");
    }
    else
    {
        showError(tr("no match found", "FindDialog"));
        // move to the end of the document (if searching down)
        //	or the beginning of the document (if searching up)
        //	for the next find.  The next find will wrap the
        //	cursor and this logic will work if the user switches
        //	the direction of the search:
        textCursor = textEdit->textCursor();
        if (next)
        {
            textCursor.movePosition(QTextCursor::End);
        }
        else
        {
            textCursor.movePosition(QTextCursor::Start);
        }
        textEdit->setTextCursor(textCursor);
    }
}

void FindReplaceForm::replace()
{
    if (!textEdit)
    {
        showError("No active editor");
        return;
    }
    if (textEdit->textCursor().hasSelection())
    {
        if (ui->regexCheckBox->isChecked())
            textEdit->textCursor().insertText(textEdit->textCursor().selectedText().replace(
                QRegularExpression(ui->textToFind->text()), ui->textToReplace->text()));
        else
            textEdit->textCursor().insertText(ui->textToReplace->text());
    }
    find();
}

void FindReplaceForm::replaceAll()
{
    if (!textEdit)
    {
        showError("No active editor");
        return;
    }

    if (ui->downRadioButton->isChecked())
        textEdit->moveCursor(QTextCursor::Start);
    else
        textEdit->moveCursor(QTextCursor::End);

    int cnt = 0;
    find();
    textEdit->textCursor().beginEditBlock();
    while (ui->replaceButton->isEnabled())
    {
        replace();
        ++cnt;
    }
    textEdit->textCursor().endEditBlock();

    showMessage(tr("Replaced %1 occurrence(s)", "FindDialog").arg(cnt));
}

void FindReplaceForm::maybeSetTextToFind(const QString &textToFind)
{
    if (!textToFind.isEmpty())
        setTextToFind(textToFind);
    ui->textToFind->setFocus();
    ui->textToFind->selectAll();
}

void FindReplaceForm::writeSettings(QSettings &settings, const QString &prefix)
{
    settings.beginGroup(prefix);
    settings.setValue(TEXT_TO_FIND, ui->textToFind->text());
    settings.setValue(TEXT_TO_REPLACE, ui->textToReplace->text());
    settings.setValue(DOWN_RADIO, ui->downRadioButton->isChecked());
    settings.setValue(UP_RADIO, ui->upRadioButton->isChecked());
    settings.setValue(CASE_CHECK, ui->caseCheckBox->isChecked());
    settings.setValue(WHOLE_CHECK, ui->wholeCheckBox->isChecked());
    settings.setValue(REGEXP_CHECK, ui->regexCheckBox->isChecked());
    settings.endGroup();
}

void FindReplaceForm::readSettings(QSettings &settings, const QString &prefix)
{
    settings.beginGroup(prefix);
    ui->textToFind->setText(settings.value(TEXT_TO_FIND, "").toString());
    ui->textToReplace->setText(settings.value(TEXT_TO_REPLACE, "").toString());
    ui->downRadioButton->setChecked(settings.value(DOWN_RADIO, true).toBool());
    ui->upRadioButton->setChecked(settings.value(UP_RADIO, false).toBool());
    ui->caseCheckBox->setChecked(settings.value(CASE_CHECK, false).toBool());
    ui->wholeCheckBox->setChecked(settings.value(WHOLE_CHECK, false).toBool());
    ui->regexCheckBox->setChecked(settings.value(REGEXP_CHECK, false).toBool());
    settings.endGroup();
}
