/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

#include "findreplace_global.h"

namespace Ui
{
class FindReplaceDialog;
}

class QTextEdit;
class QSettings;

/**
 * A find/replace dialog.
 *
 * It relies on a FindReplaceForm object (see that class for the functionalities provided).
 */
class FINDREPLACESHARED_EXPORT FindReplaceDialog : public QDialog
{
    Q_OBJECT
  public:
    FindReplaceDialog(QWidget *parent = 0);
    virtual ~FindReplaceDialog();

    /**
     * Associates the text editor where to perform the search
     * @param textEdit
     */
    void setTextEdit(QTextEdit *textEdit);

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
     * Finds the next or previous occurrence:
     */
    void find();

    /**
     * Finds the next occurrence
     */
    void findNext();

    /**
     * Finds the previous occurrence
     */
    void findPrev();

    /**
     * Show dialog with text to find selected
     */
    void showDialog(const QString &textToFind = QString());

  protected:
    void changeEvent(QEvent *e);

    Ui::FindReplaceDialog *ui;
};

#endif // FINDREPLACEDIALOG_H
