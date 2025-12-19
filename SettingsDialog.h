#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTabWidget>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onAccept();
    void onApply();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

    QCheckBox* m_exportOBJCheck;
    QCheckBox* m_exportPNGCheck;
    QCheckBox* m_exportJSONCheck;
    QCheckBox* m_exportMTLCheck;
    QCheckBox* m_exportBlenderCheck;

    QRadioButton* m_formatPNGRadio;
    QRadioButton* m_formatJPEGRadio;
    QButtonGroup* m_formatGroup;

    QRadioButton* m_scale100Radio;
    QRadioButton* m_scale50Radio;
    QRadioButton* m_scale25Radio;
    QButtonGroup* m_scaleGroup;
};

#endif // SETTINGSDIALOG_H
