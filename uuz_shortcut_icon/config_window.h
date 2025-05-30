#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialog>

#include "mainwidget.h"
#include "ui_config_window.h"
#include "json.hpp"
#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"
#include "Logger.h"
#include <windows.h>

class MainWidget;

class Config_window : public QWidget {
	Q_OBJECT

public:
	Config_window(QWidget* parent = nullptr);
	~Config_window() override;

protected:
	void closeEvent(QCloseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	Ui::Config_windowClass ui;
	void                   init_config();
	void                   init_translate();

	//json所需的key
	const std::string shortcut_key_msg = "shortcut_key_msg";
	const std::string is_boot_start    = "is_boot_start";
	const std::string log_retain_day   = "log_retain_day";
	const std::string Ctrl             = "Ctrl";
	const std::string CtrlCount        = "CtrlCount";
	const std::string Alt              = "Alt";
	const std::string AltCount         = "AltCount";

	//配置
	bool config_is_boot_start;

	//来自主窗口的指针，负责配置修改
	MainWidget* main_widget;
	//来自search_content窗口的过滤引用，负责配置修改
	QStringList& filter_path_list;
	QStringList& filter_suffix_list;

	int state;

	//快捷键
	bool recording = false;
	int first_key_code = -1;

	ShortcutKeyMsg shortcut_msg;

	
signals:
	void sig_checkBoxIsBootStart();

private slots:
	void slot_checkBoxIsBootStart(int state);
	void slot_spinBoxValueChanged(int day);
	void slot_openConfigPath();
	void slot_aboutDialog();
	void slot_checkBoxCtrlChanged(int state);
	void slot_checkBoxAltChanged(int state);
	void slot_spinBoxCtrlChanged(int val);
	void slot_spinBoxAltChanged(int val);
	void slot_clearButton_clicked();
};
