#pragma once

#include <QWidget>

#include <Windows.h>
#include <QPainter>


class HighlightFrame : public QWidget
{
	Q_OBJECT

public:
	HighlightFrame(QWidget* parent = nullptr);
	~HighlightFrame();

	void showOn(HWND hwnd);

private:

protected:
	void paintEvent(QPaintEvent* event) override;

};

