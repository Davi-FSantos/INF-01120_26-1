#include "mainwindow.h"
#include "ui_qtmidi.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MusicMachine)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
