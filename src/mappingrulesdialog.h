#ifndef MAPPINGRULESDIALOG_H
#define MAPPINGRULESDIALOG_H

#include <QDialog>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MappingRulesDialog;
}
QT_END_NAMESPACE

class MappingRulesDialog : public QDialog {
    Q_OBJECT

    public:
    explicit MappingRulesDialog(QWidget *parent = nullptr);
    ~MappingRulesDialog() override;

    protected:
    void changeEvent(QEvent *event) override;

    private:
    Ui::MappingRulesDialog *ui;
};

#endif // MAPPINGRULESDIALOG_H
