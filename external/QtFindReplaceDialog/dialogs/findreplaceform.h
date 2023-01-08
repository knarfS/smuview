/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#ifndef FINDREPLACEFORM_H
#define FINDREPLACEFORM_H

#include <QTextCursor>
#include <QWidget>

#include "findreplace_global.h"

namespace Ui
{
class FindReplaceForm;
}

class QTextEdit;
class QSettings;
class QEvent;
class QShowEvent;

/**
 * The form for the find/replace dialog.  The form presents the typical
 * widgets you find in standard find/replace dialogs, and it acts on a QTextEdit.
 *
 * \image html Screenshot-FindReplace.png
 *
 * You need to set the QTextEdit explicitly, using the method setTextEdit(QTextEdit *textEdit).
 *
 * For instance
 * \code
 * m_findReplaceDialog = new FindReplaceDialog(this);
 * m_findReplaceDialog->setModal(false);
 * m_findReplaceDialog->setTextEdit(ui->textEdit);
 * \endcode
 *
 * The find functionalities is available even if the find dialog is not shown: if something
 * to search for was already specified, the application can call the methods findNext() and
 * findPrev() (e.g., by connecting them to menu items).
 *
 * In case a regular expression is used as the search term, the form also checks whether the
 * expression is a valid regular expression (You may want to take a look at the syntax of regular expressions:
 * http://doc.trolltech.com/qregexp.html).
 *
 * The form provides also functionalities to save and restore its state using a QSettings object (i.e.,
 * the last word searched for, the options of the form, etc.) via the methods writeSettings()
 * and readSettings().
 *
 * You can take a look at the \ref examples page.
 */
class FINDREPLACESHARED_EXPORT FindReplaceForm : public QWidget
{
    Q_OBJECT
  public:
    FindReplaceForm(QWidget *parent = 0);
    virtual ~FindReplaceForm();

    /**
     * Associates the text editor where to perform the search
     * @param textEdit_
     */
    void setTextEdit(QTextEdit *textEdit_);

    /// hides replace widgets from the form
    void hideReplaceWidgets();

    /**
     * Writes the state of the form to the passed settings.
     * @param settings
     * @param prefix the prefix to insert in the settings
     */
    virtual void writeSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");

    /**
     * Reads the state of the form from the passed settings.
     * @param settings
     * @param prefix the prefix to look for in the settings
     */
    virtual void readSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");

  public Q_SLOTS:
    /**
     * Sets the current textToFind (used to set it from specialized current selection, etc)
     */
    void setTextToFind(const QString &strText);

    /**
     * performs the find task
     * @param next whether to find the next or the previous
     * occurrence
     */
    void find(bool next);

    /**
     * Finds the next occurrence
     */
    void find();

    /**
     * Finds the next occurrence
     */
    void findNext()
    {
        find(true);
    }

    /**
     * Finds the previous occurrence
     */
    void findPrev()
    {
        find(false);
    }

    /**
     * Replaces the found occurrences and goes to the next occurrence
     */
    void replace();

    /**
     * Replaces all the found occurrences
     */
    void replaceAll();

    /**
     * Set text to find if not empty and select it
     */
    void maybeSetTextToFind(const QString &textToFind = QString());

  protected:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent *event);

    /// shows an error in the dialog
    void showError(const QString &error);

    /// shows a message in the dialog
    void showMessage(const QString &message);

  protected Q_SLOTS:
    /// when the text edit contents changed
    void textToFindChanged();

    /// checks whether the passed text is a valid regexp
    void validateRegExp(const QString &text);

    /// the regexp checkbox was selected
    void regexpSelected(bool sel);

    // the selection of the textedit changed
    void onSelectionChanged();

  protected:
    Ui::FindReplaceForm *ui;

    /// for searching into the text
    QTextCursor textCursor;

    /// the text editor (possibly) associated with this form
    QTextEdit *textEdit;

    // connection to textedit selection change
    QMetaObject::Connection selectionChangeConnection;
};

#endif // FINDREPLACEFORM_H
