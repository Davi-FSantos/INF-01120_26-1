#include "mappingrulesdialog.h"
#include "ui_mappingrulesdialog.h"

MappingRulesDialog::MappingRulesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MappingRulesDialog) {
    ui->setupUi(this);
}

MappingRulesDialog::~MappingRulesDialog() {
    delete ui;
}

void MappingRulesDialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
