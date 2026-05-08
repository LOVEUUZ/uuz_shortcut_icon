#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMessageBox>
#include "HighlightFrame.h"

#include <Windows.h>
#include <Psapi.h>

//遮罩层，实际上唯一的用处是直观的表示已经开始窗口选取功能
class OverlayWidget : public QWidget {
	Q_OBJECT

public:
	OverlayWidget(QWidget* parent = nullptr);
	~OverlayWidget();

	void startSelection();
	void stopSelection();

	static OverlayWidget* ptr_overlayWidget;

private:
	QTimer* getSysInfoTimer;			//定时器用于轮询当前鼠标位置来获取窗口信息并更新高亮框的位置
	HWND    lastHwnd;					//最后一次获取的窗口handle
	bool    is_running;

	HighlightFrame* highLigthFrame;		//高亮边框

	void updateSelection();				//核心，定时器触发，获取当前鼠标位置的窗口信息并更新高亮框的位置
	void handleMouseClick(HWND hwnd);
	void handleCancel();
	bool getProcessInfo(HWND hwnd, DWORD& pid, QString& name);
	bool getProcessPath(DWORD pid, QString& path);
	QString getClassNameStr(HWND hwnd);
	QString getWindowTitleStr(HWND hwnd);

protected:
	void paintEvent(QPaintEvent* event) override;


signals:
	/*
	* name 程序名称(非任务管理器显示的)
	* exePath 可执行文件完整路径
	* className 窗口类名，暂时没用(同一程序基本固定,可区分同路径不同窗口类型)
	* pid 仅调试用
	* title 仅调试用
	*/
	void sigWindowSelected(QString name, QString exePath, QString className, DWORD pid, QString title);
	void sigCancel();

};

