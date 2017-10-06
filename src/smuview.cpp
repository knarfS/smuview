#include "smuview.h"
#include "ui_smuview.h"

smuview::smuview(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::smuview)
{
    ui->setupUi(this);
}

smuview::~smuview()
{
    delete ui;
}
