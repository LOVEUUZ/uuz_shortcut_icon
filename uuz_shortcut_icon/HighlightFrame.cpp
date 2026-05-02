#include "HighlightFrame.h"

HighlightFrame::HighlightFrame(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint |
        Qt::Tool);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

HighlightFrame::~HighlightFrame() {}

void HighlightFrame::showOn(HWND hwnd) {
    RECT rc;
    GetWindowRect(hwnd, &rc);

    setGeometry(rc.left, rc.top,
        rc.right - rc.left,
        rc.bottom - rc.top);

    show();
    raise();
}

void HighlightFrame::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QPen pen(Qt::red, 3);
    p.setPen(pen);
    p.drawRect(rect().adjusted(1, 1, -1, -1));
}