#ifndef SMUVIEW_H
#define SMUVIEW_H

#include <QMainWindow>

namespace Ui {
class smuview;
}

class smuview : public QMainWindow
{
    Q_OBJECT

public:
    explicit smuview(QWidget *parent = 0);
    ~smuview();

private:
    Ui::smuview *ui;
};

#endif // SMUVIEW_H
