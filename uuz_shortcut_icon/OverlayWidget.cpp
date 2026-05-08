#include "OverlayWidget.h"

OverlayWidget* OverlayWidget::ptr_overlayWidget = nullptr;

OverlayWidget::OverlayWidget(QWidget* parent) : QWidget(parent), lastHwnd(nullptr), is_running(false) {

    setWindowFlags(Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint |
        Qt::Tool);  // 防止出现在任务栏

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);   //允许鼠标穿透

    //Win32 穿透，要不然无法会被该窗口拦截鼠标事件（我也不知道为啥上面那一句不起作用）
    HWND hwnd = (HWND)this->winId();
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    //Qt的窗口只能收到自己窗口的事件，所以要主动轮询获取系统状态
    getSysInfoTimer = new QTimer(this);
    connect(getSysInfoTimer, &QTimer::timeout, this, &OverlayWidget::updateSelection);

    highLigthFrame = new HighlightFrame(this);
}

OverlayWidget::~OverlayWidget() {}

void OverlayWidget::startSelection() {
    is_running = true;
    lastHwnd = nullptr;

    showFullScreen();
    getSysInfoTimer->start(50);
}

void OverlayWidget::stopSelection() {
    is_running = false;

    if (getSysInfoTimer)
        getSysInfoTimer->stop();

    if (highLigthFrame)
        highLigthFrame->hide();

    close();
}


void OverlayWidget::updateSelection() {
    if (!is_running) { return; };

    POINT pt;
    GetCursorPos(&pt);

    HWND hwnd = WindowFromPoint(pt);
    if (!hwnd) return;

    if (hwnd == (HWND)this->winId())
        return;

    // ===== 只在变化时处理 =====
    if (hwnd != lastHwnd) {
        lastHwnd = hwnd;

        //高亮边框
        if (highLigthFrame) {
            highLigthFrame->showOn(hwnd);
        }

    }

    // ===== 左键确认 =====
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        handleMouseClick(hwnd);
        return;
    }

    // ===== 右键取消 | esc取消 =====
    if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) || (GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
        handleCancel();
        return;
    }
}

void OverlayWidget::handleMouseClick(HWND hwnd) {
    getSysInfoTimer->stop();   // 停止轮询（防止继续切换高亮）

    DWORD pid;
    QString name;

    if (!getProcessInfo(hwnd, pid, name)) {
        stopSelection();
        return;
    }

    QString exePath;
    getProcessPath(pid, exePath);

    QString className = getClassNameStr(hwnd);
    QString title = getWindowTitleStr(hwnd);

    // 弹确认框
    QMessageBox box(this);
    box.setWindowTitle("Select window");
    box.setText(QString("PID: %1\nProcess: %2\n\nPath: %3\nClass: %4\nTitle: %5")
        .arg(pid)
        .arg(name)
        .arg(exePath)
        .arg(className)
        .arg(title)
    );
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    if (box.exec() == QMessageBox::Ok) {
        emit sigWindowSelected(name, exePath, className, pid, title);
        stopSelection();
    }
    else {
        getSysInfoTimer->start(50);
    }
}

void OverlayWidget::handleCancel() {
    if (highLigthFrame) {
        highLigthFrame->hide();
    }
    emit sigCancel();
    stopSelection();
}

bool OverlayWidget::getProcessInfo(HWND hwnd, DWORD& pid, QString& name) {
    pid = 0;
    name.clear();

    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) return false;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return false;

    char buffer[MAX_PATH] = { 0 };

    if (GetModuleBaseNameA(hProcess, NULL, buffer, MAX_PATH)) {
        name = QString(buffer);
    }

    CloseHandle(hProcess);

    return true;
}

bool OverlayWidget::getProcessPath(DWORD pid, QString& path) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    WCHAR buffer[MAX_PATH] = { 0 };
    DWORD size = MAX_PATH;

    if (QueryFullProcessImageNameW(hProcess, 0, buffer, &size)) {
        path = QString::fromWCharArray(buffer);
        CloseHandle(hProcess);
        return true;
    }

    CloseHandle(hProcess);
    return false;
}

QString OverlayWidget::getClassNameStr(HWND hwnd) {
    WCHAR buf[256] = { 0 };
    GetClassNameW(hwnd, buf, 256);
    return QString::fromWCharArray(buf);
}

QString OverlayWidget::getWindowTitleStr(HWND hwnd) {
    WCHAR buf[512] = { 0 };
    GetWindowTextW(hwnd, buf, 512);
    return QString::fromWCharArray(buf);
}

void OverlayWidget::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 100)); // 遮罩
}

