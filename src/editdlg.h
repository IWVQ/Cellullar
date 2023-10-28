#ifndef EDITDLG_H
#define EDITDLG_H

#include <QDialog>
#include "jshilite.h"

namespace Ui {
class EditDlg;
}

class EditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EditDlg(QWidget *parent = nullptr);
    ~EditDlg();

    void modifyRule(bool yes);
    void saveRule();
    QString transitionRule();
    void showEditDialog(const QString &s);
    void setTransitionRule(QString s);
    void setRunning(bool yes);
    void applyConfig(QJsonDocument *config);
signals:
    void showConfig();
    void showHelp();
private slots:
    void on_textEdit_cursorPositionChanged();

    void on_pushButton_15_toggled(bool checked);

    void on_actionEdtUndo_triggered();

    void on_actionEdtRedo_triggered();

    void on_actionEdtCut_triggered();

    void on_actionEdtCopy_triggered();

    void on_actionEdtPaste_triggered();

    void on_actionEdtSelectAll_triggered();

    void on_actionAccept_triggered();

    void on_actionCancel_triggered();

    void on_pushButton_14_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_12_clicked();

    void on_actionEdtReplace_triggered();

    void on_actionEdtSearch_triggered();

    void on_actionEdtBuild_triggered();

    void on_textEdit_textChanged();

    void on_EditDlg_accepted();

    void on_EditDlg_rejected();

signals:
    void buildTransitionRule(const QString &s);
private:
    Ui::EditDlg *ui;
    JSHighlighter *highlighter;
    QString savedtransitionrule = "";
    bool rulemodified = false;
    QMenu *edtmenu;
    QMenu *edtmenufile;
    QMenu *edtmenuedit;
};

#endif // EDITDLG_H
