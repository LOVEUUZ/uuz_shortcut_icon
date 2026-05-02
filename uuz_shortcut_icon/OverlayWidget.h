#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMessageBox>
#include "HighlightFrame.h"

#include <Windows.h>
#include <Psapi.h>

//遮罩层，实际上唯一的用处是直观的表示已经开始窗口选取功能
class OverlayWidget : public QWidget
{
	Q_OBJECT

public:
	OverlayWidget(QWidget* parent = nullptr);
	~OverlayWidget();

	void startSelection();
	void stopSelection();

	static OverlayWidget* ptr_overlayWidget;

private:
	QTimer* getSysInfoTimer;
	HWND    lastHwnd;
	bool    is_running;

	HighlightFrame* highLigthFrame;

	void updateSelection();
	void handleMouseClick(HWND hwnd);
	void handleCancel();
	bool getProcessInfo(HWND hwnd, DWORD& pid, QString& name);

protected:
	void paintEvent(QPaintEvent* event) override;


signals:
	void sigWindowSelected(DWORD pid, QString name);
	void sigCancel();


};

