#include "mainwidget.h"

using json                           = nlohmann::json;
MainWidget* MainWidget::main_widget_ = nullptr;

MainWidget::MainWidget(QWidget* parent) : QWidget(parent) {
  // ui.setupUi(this);
  setObjectName("mainWidget");
  // setWindowFlags(Qt::FramelessWindowHint);
  main_widget_ = this;

  init_coordinate();

  init_shortcutKey();

  init_layout();

  init_search_line();

  connect(this, &MainWidget::sig_moveFocus, this, &MainWidget::slot_moveFocus);

#ifdef _DEBUG
  //24-11-6: 右键菜单的锁定移动功能，用于缓解多屏模式下的复位问题测试
  connect(icons_inner_widget, &Icons_inner_widget::sig_action_show_move, this, &MainWidget::slot_action_show_move);
#endif


  init_tray();

  //去掉最大化最小化。因为下面那个没生效
  setWindowFlags(Qt::Dialog);
#ifdef NDEBUG
  //置顶 禁用最大最小化
  this->setWindowFlags(
    (this->windowFlags() & ~Qt::WindowCloseButtonHint) // 移除关闭按钮
    & ~Qt::WindowMinimizeButtonHint                    // 移除最小化按钮
    & ~Qt::WindowMaximizeButtonHint                    // 移除最大化按钮
    | Qt::WindowStaysOnTopHint                         // 置顶
    | Qt::CustomizeWindowHint                          // 自定窗口
  );

#endif

  // setStyleSheet("MainWidget{background-color: #b7b7b7;} ");


  //在这里设置一下日志天数，懒得创建新东西了
  Logger::getLogger().set_retention_days(json_config["log_retain_day"].get<int>());

  //首次创建配置则直接打开配置界面
  if (isFirstStart) {
      show();
      QTimer::singleShot(0, this, [&]() {
          icons_inner_widget->slot_configWidgetOpen();
      });
	  isFirstStart = false;
  }
}

MainWidget::~MainWidget() {}


void MainWidget::init_search_line() {
  //样式
  QFont font;
  font.setPointSize(16);
  search_line->setFont(font);


  //用于避免定时器
  searchTimer = new QTimer(this);
  searchTimer->setSingleShot(true); // 定时器只执行一次

  //当定时器倒计时结束后触发搜索任务
  connect(searchTimer, &QTimer::timeout, this, [this]() {
    // slot_showStackedWidgetIndex(text.isEmpty() ? 0 : 1);
    emit search_inner_widget->slot_textChange(search_line->text()); // 发出信号进行搜索
  });

  //当搜索栏文本改变后触发定时器倒计时一次
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    searchTimer->start(300); // 启动定时器，300毫秒后触发
  });

  //控制 stacked_widget 显示为 search_inner_widget
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    QString text = this->search_line->text();

    int index = 0;
    if (text.isEmpty()) index = 0; // 文本为空，显示  icons_inner_widget 图标显示窗口
    else index                = 1; // 显示为 search_inner_widget 搜索内容窗口

    slot_showStackedWidgetIndex(index);
  });
}


// 设置布局
void MainWidget::init_layout() {
  // 1. 创建主布局并设置无边距
  v_search_and_grid = new QVBoxLayout(this);
  v_search_and_grid->setContentsMargins(0, 0, 0, 0); // 设置主布局无边距
  setLayout(v_search_and_grid);

  // 2. 创建上部垂直布局，并设置边距和间距
  topLayout = new QVBoxLayout();
  topLayout->setContentsMargins(5, 5, 5, 0); // 设置上部布局与四周的距离
  topLayout->setSpacing(5);                  // 设置上部布局内部组件间距为5

  search_line = new Search_line(this);
  search_line->setFixedSize(800, 60);                                     // 设置上部 QLineEdit 的固定高度为60
  search_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 设置为可扩展，宽度可以填满

  topLayout->addWidget(search_line);

  // 3. 下半部分布局，并设置边距和间距
  stacked_widget = new QStackedWidget(this);
  stacked_widget->setFixedSize(800, 400);
  stacked_widget->setContentsMargins(10, 5, 5, 10); // 设置内边距

  iconsLayout = new QVBoxLayout(stacked_widget);
  iconsLayout->setContentsMargins(10, 5, 5, 10); // 设置内边距
  iconsLayout->setSpacing(0);                    // 设置间距为0

  // 4. 创建内部 widget 并设置大小和颜色,添加进入栈窗口显示
  icons_inner_widget = new Icons_inner_widget(stacked_widget);
  // icons_inner_widget->setStyleSheet("background-color: #123456;"); // 设置背景颜色
  // icons_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(icons_inner_widget); // 添加到布局中

  search_inner_widget = new Search_content(stacked_widget);
  // search_inner_widget->setStyleSheet("background-color: red;"); // 设置背景颜色
  // search_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(search_inner_widget); // 添加到布局中

  stacked_widget->addWidget(icons_inner_widget);
  stacked_widget->addWidget(search_inner_widget);


  // 5. 添加上部和下部布局到主布局
  v_search_and_grid->addLayout(topLayout);      // 将上部布局添加到主布局
  v_search_and_grid->addWidget(stacked_widget); // 将下部widget添加到主布局

  // 自动调整窗口大小
  adjustSize();         // 调整窗口大小以适应内容
  setFixedSize(size()); // 将当前大小设置为固定大小
}

void MainWidget::init_shortcutKey() {
  //键盘捕获部分
  ctrlPressTimer = new QTimer(this);
  ctrlPressTimer->setInterval(300); // 0.3秒
  ctrlPressTimer->setSingleShot(true);
  ctrlPressCount   = 0;
  ctrlReleaseCount = 0;
  connect(ctrlPressTimer, &QTimer::timeout, [&]() {
    ctrlPressCount   = 0;
    ctrlReleaseCount = 0;
  });

  setKeyEvent();

  //鼠标钩子
  setMouseEvent();

  clear_show_count = new QTimer(this);
  connect(clear_show_count, &QTimer::timeout, this, [&]() {
    widget_show_count = 0;
  });
  clear_show_count->start(250); // 250ms 周期
}


void MainWidget::setKeyEvent() {
  //注册给钩子那边调用的函数
  std::function<void(KeyEvent)> func = [&](const KeyEvent & keyEvent) {
#ifdef _DEBUG
    std::cout << keyEvent.key << " - " << keyEvent.isPressed << "\n";
#endif

  //  static bool ctrlOrAltIsPressed = false;
  //  if (keyEvent.key == shortcut_key_value_1 || ctrlOrAltIsPressed) { //左右alt和ctrl
  //    ctrlOrAltIsPressed = true;
  //    if (keyEvent.key == shortcut_key_value_1 && !keyEvent.isPressed) {  //松开ctrl或alt则结束判断
	 //   // 定时器，延时修改第一个键的当前状态，因为当手速很快的时候还没来得及按下第二个键，第一个已经松开了
  //      QTimer::singleShot(100, this, [this]() {
  //          ctrlOrAltIsPressed = false;
  //       });
  //      return;  
  //    }
  //    if (keyEvent.key == shortcut_key_value_2 && keyEvent.isPressed && ctrlOrAltIsPressed) {   //第二个按键符合且是按下触发
  //      if (isHidden()) {
  //        this->raise();
  //        //this->activateWindow();
  //        show();
  //      } else {
	 //  	  hide();
  //      }
  //    } else if (keyEvent.key == shortcut_key_value_2 && !keyEvent.isPressed) {   //第二个按键符合并松开
  //      return;
  //    }
  //  }
  //  else {
		//hide(); //隐藏主窗口
  //  }
	static std::vector<int> vec_curKey;
    if (keyEvent.isPressed) {
        // 按下添加
		if (std::find(vec_curKey.begin(), vec_curKey.end(), keyEvent.key) == vec_curKey.end()) {    //重复检测
            vec_curKey.push_back(keyEvent.key);
        }
    }
    else {
        // 松开时移除该键
        auto it = std::find(vec_curKey.begin(), vec_curKey.end(), keyEvent.key);
        if (it != vec_curKey.end()) {
            vec_curKey.erase(it);
        }
    }

#ifdef _DEBUG
    std::cout << "[";
    for (size_t i = 0; i < vec_curKey.size(); ++i) {
        std::cout << vec_curKey[i];
        if (i != vec_curKey.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
#endif


	static bool isCtrlCheck = false; //配置是否勾选了Ctrl键作为触发键
	static bool isAltCheck = false;
	static int  CtrlCount = 0; //配置中存储的Ctrl键按下触发次数
	static int  AltCount = 0;
	static std::pair<int, std::vector<int>> pair_key; //用于存储当前按键配置
	static bool isCtrlPress = false; //当前是否按下Ctrl键
	static bool isAltPress = false;

    //启动初始化配置缓存或者修改配置文件后重置缓存
    if (is_changed) {
        isCtrlCheck = json_config["Ctrl"].get<bool>();
        isAltCheck = json_config["Alt"].get<bool>();
        CtrlCount = json_config["CtrlCount"].get<int>();
        AltCount = json_config["AltCount"].get<int>();
        if (json_config.contains("shortcut_key_msg")) {
            const auto& shortcut = json_config["shortcut_key_msg"];
            if (shortcut.contains("key_value_total") && shortcut.contains("key_value_serial_number")) {
                pair_key.first = shortcut["key_value_total"].get<int>(); //total 作为 key
                pair_key.second = shortcut["key_value_serial_number"].get<std::vector<int>>(); //每个按键的序列作为 value
            }
        }
		is_changed = false;
    }

	
    if (!isVisible()) {
        //非单ctrl和alt按键触发
        if (pair_key.second.size() != 0) {
            //每次触发事件都将当前队列中的内容与配置存储的按键序列进行对比
            if (vec_curKey.size() == pair_key.second.size()) {
                for (int i = 0; i < pair_key.second.size(); ++i) {
                    if (std::find(vec_curKey.begin(), vec_curKey.end(), pair_key.second[i]) == vec_curKey.end()) {
                        return;  // 只要有一个没找到就直接返回
                    }
                }
                show();
                return;
            }
        }

        //单独的ctrl和alt按键触发
        if (keyEvent.key == 162 || keyEvent.key == 163 || keyEvent.key == 164 || keyEvent.key == 165) {
            bool isCtrl = (keyEvent.key == 162 || keyEvent.key == 163);
            bool isAlt = (keyEvent.key == 164 || keyEvent.key == 165);

            static std::atomic<int> ctrl_press_count{ 0 };
            static int alt_press_count = 0;

            static bool ctrl_is_down = false;
            static bool alt_is_down = false;

            //qDebug() << "key = " << keyEvent.key << "  ==> " << keyEvent.isPressed;

            if (keyEvent.isPressed) {
                if (isCtrlCheck && isCtrl && !ctrl_is_down) {
                    ctrl_is_down = true; // 标记已按下，防止重复计数
                }
                if (isAltCheck && isAlt && !alt_is_down) {
                    alt_is_down = true;
                }
            }
            else {
                if (isCtrlCheck && isCtrl && ctrl_is_down) {
                    ctrl_is_down = false;
                    ctrl_press_count++;

                    int timeout = 200 * CtrlCount;
                    QTimer::singleShot(timeout, this, [this]() {
                        //qDebug() << "ctrl_press_count = " << ctrl_press_count << " 清零";
                        ctrl_press_count = 0;
                        });

                    if (ctrl_press_count >= CtrlCount && CtrlCount > 0) {
                        //qDebug() << "ctrl_press_count = " << ctrl_press_count << " 清零并显示";

                        //2025.9.1 尝试用于缓解按住ctrl不放的情况下窗口疯狂隐藏和显示
                        widget_show_count++;
                        qDebug() << "widget_show_count = " << widget_show_count;
                        if (widget_show_count >= 2) {
                          qDebug() << "widget_show_count >= 2, 不显示窗口";
                          return;
                        }
                        ctrl_press_count = 0;
                        show();
                        return;
                    }
                }

                if (isAltCheck && isAlt && alt_is_down) {
                    alt_is_down = false;
                    alt_press_count++;

                    int timeout = 200 * AltCount;
                    QTimer::singleShot(timeout, this, [this]() {
                        alt_press_count = 0;
                        });

                    if (alt_press_count >= AltCount && AltCount > 0) {
                        alt_press_count = 0;
                        show();
                        return;
                    }
                }
            }
        }
    }
    else {
        // 2. 窗口已显示，任意按键按下隐藏窗口
        if (keyEvent.isPressed && !this->search_line->hasFocus()) {
            hide();
            return;
        }
    }


  };

  windowsKeyHookEx = WindowsHookKeyEx::getWindowHook();
  windowsKeyHookEx->installHook();
  windowsKeyHookEx->setFunc(func);
}

void MainWidget::setMouseEvent() {
  //鼠标事件捕获部分
  windowsMouseHook = WindowsHookMouseEx::getWindowHook();
  ////回调注册方法,当鼠标不在程序内部点击的时候，隐藏程序窗口
  std::function<void()> func = [&]() {
    // 获取鼠标的全局坐标
    auto mouse_coordinate = QCursor::pos();
    // 获取窗口左上角的全局坐标
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    // 创建一个包含窗口大小的矩形，并向上扩展 45px(大概是windows自带的最上面那一条的高度)
    QSize size_contain_top_edge_size = size();
    size_contain_top_edge_size.setHeight(size_contain_top_edge_size.height() + 45);

    // 调整窗口左上角的坐标
    globalPos.setY(globalPos.y() - 45);

    QRect globalRect(globalPos, size_contain_top_edge_size);

    // 判断鼠标坐标是否在窗口的全局矩形内
    if (!globalRect.contains(mouse_coordinate)) {
      emit sig_moveFocus(nullptr); // 如果不在窗口内，发出信号
    }
  };


  windowsMouseHook->setFunc(func);

#ifdef _DEBUG
  windowsMouseHook->installHook(); //安装鼠标钩子。当窗口显示的时候安装，隐藏的时候卸载
#endif
}

void MainWidget::showEvent(QShowEvent* event) {
  //判断鼠标当前在哪一个屏幕上，然后显示窗口显示在该屏幕记录的坐标
  QPoint          cursorPos = QCursor::pos();
  QList<QScreen*> screens   = QGuiApplication::screens();
  for (int i = 0; i < screens.size(); ++i) {
    QScreen* screen = screens[i];
    if (screen->geometry().contains(cursorPos)) {
      this->move(screens_coordinate[i]);
    }
  }

  windowsMouseHook->installHook(); //安装鼠标钩子

  //TODO 暂时禁用，好像我根本用不到。。。而且还会扰乱新逻辑，唉，开摆
  // 延迟 300ms 后强制输入框获取焦点
  //QTimer::singleShot(500, this, [this]() {
    //windows api，强制输入框获取焦点，参考：https://learn.microsoft.com/zh-cn/windows/win32/api/winuser/nf-winuser-setfocus
   // SetFocus((HWND)this->search_line->winId());
   //});
  this->search_line->setEnabled(false);
  QTimer::singleShot(500, this, [this]() {
      this->search_line->setEnabled(true);
   });

  QWidget::showEvent(event);
}

void MainWidget::hideEvent(QHideEvent* event) {
  emit sig_isShowNotice(false);
  windowsMouseHook->unInstallHook(); //卸载鼠标钩子

  search_line->clear(); //清空搜索栏，并让界面重回图标界面

  QWidget::hideEvent(event);
}

#ifdef _DEBUG
static bool is_move_lock = false;
#endif

void MainWidget::moveEvent(QMoveEvent* event) {
  // if (is_move_lock) {
  if (QApplication::mouseButtons() & Qt::LeftButton) {
    //当鼠标左键按下的时候移动才会写入配置文件，用于缓解多屏幕模式下的坐标复位问题
#ifdef _DEBUG
        qDebug() << "移动窗口";
#endif

    // 多屏幕检测
    QPoint cursorPos = QCursor::pos();
    // int index = -1;
    QList<QScreen*> screens = QGuiApplication::screens();
    for (int i = 0; i < 10; ++i) {
      QScreen* screen = screens[i];
      if (screen->geometry().contains(cursorPos)) {
#ifdef _DEBUG
       qDebug() << "当前在屏幕 ==> " << i;
#endif

        QPoint coordinate;

        //判断当前屏幕是否记录了坐标，没有则添加新坐标并更新json
        auto it = screens_coordinate.find(i);
        if (it != screens_coordinate.end()) {
          coordinate.setX((screen->size().width()) / 2 - (this->size().width() / 2));
          coordinate.setY((screen->size().height()) / 2 - (this->size().height() / 2));
          screens_coordinate[i]                     = coordinate;
          json_config["coordinate"][i]["screen_id"] = i;
          json_config["coordinate"][i]["x"]         = coordinate.x();
          json_config["coordinate"][i]["y"]         = coordinate.y();
        }

        coordinate                                                               = this->pos();
        screens_coordinate[json_config["coordinate"][i]["screen_id"].get<int>()] = coordinate;

        json_config["coordinate"][i]["x"] = coordinate.x();
        json_config["coordinate"][i]["y"] = coordinate.y();

#ifdef _DEBUG
                qInfo() << "";
                qWarning() << "显示坐标修改: " << "id = " << i;

                qWarning() << "coordinate （" << coordinate.x()
                    << "," << coordinate.y() << ")";

                qWarning() << "json （" << json_config["coordinate"][i]["x"].get<int>()
                    << "," << json_config["coordinate"][i]["y"].get<int>() << ")";

                qInfo() << "";
#endif

        break;
      }
    }


    // 写入文件
    file_config->resize(0); // 清空文件
    QTextStream config_content(file_config);
    config_content << QString::fromStdString(json_config.dump(4));
  }

  QWidget::moveEvent(event); // 调用基类的 moveEvent
}


void MainWidget::init_coordinate() {
  file_path = QCoreApplication::applicationDirPath() + "/config/shortcut_mouse_config.json";
  // 获取目录路径
  QDir dir = QFileInfo(file_path).absoluteDir();
  // 检查目录是否存在，如果不存在则创建它及其父文件夹
  if (!dir.mkpath(".")) {
    qWarning() << "Cannot create directory:" << dir.path();
    return;
  }

  file_config = new QFile(file_path);
  // 可读可写模式打开文件
  file_config->open(QIODevice::ReadWrite | QIODevice::Text);

  // 读取文件内容
  file_config->seek(0);
  QString qstr_config_content = file_config->readAll();
  std::string str_config_content;

  if (qstr_config_content.isEmpty()) {
	isFirstStart = true;
    //2025.6.2 添加自定义快捷键唤醒
    json_config["Ctrl"] = true;
    json_config["CtrlCount"] = 2;
    json_config["Alt"] = false;
    json_config["AltCount"] = 2;

    //添加日志的记录，避免读取保留天数的时候出错
    json_config["log_retain_day"] = 7;

	//2025.3.20: 预先创建10个屏幕的坐标记录，避免鼠标拖动窗口跨越屏幕的瞬间似乎json库会空指针
    json_config["coordinate"] = nlohmann::json::array();

    // 获取所有屏幕信息
    QList<QScreen*> screens = QGuiApplication::screens();
    int screenCount = screens.size();

    for (int i = 0; i < 10; ++i) {
        QPoint coordinate;

        if (i < screenCount) {
            // 真实存在的屏幕
            QRect screenGeometry = screens[i]->geometry();
            coordinate.setX(screenGeometry.center().x() - this->width() / 2);
            coordinate.setY(screenGeometry.center().y() - this->height() / 2);
        }
        else if (screenCount > 0) {
			// 预备，使用第一个屏幕的中心
            QRect firstScreenGeometry = screens[0]->geometry();
            coordinate.setX(firstScreenGeometry.center().x() - this->width() / 2);
            coordinate.setY(firstScreenGeometry.center().y() - this->height() / 2);
        }
        
        json_config["coordinate"].push_back({
            {"screen_id", i},
            {"x", coordinate.x()},
            {"y", coordinate.y()}
            });

        screens_coordinate[i] = coordinate;
    }


    QTextStream text_stream(file_config);
    text_stream << QString::fromStdString(json_config.dump(4));
    return;
  }
  else {
	  //2025.6.2: 新增了一些配置项，检查是否存在，如果不存在打开配置界面,后续在这里追加
      str_config_content = qstr_config_content.toStdString();
      json_config = json::parse(str_config_content);
      if (!json_config.contains("Ctrl") ||
          !json_config.contains("CtrlCount") ||
          !json_config.contains("Alt") ||
          !json_config.contains("AltCount")) {
          isFirstStart = true;
      }

      
  }

  str_config_content = qstr_config_content.toStdString();
  json_config        = json::parse(str_config_content);

  for (int i = 0; i < json_config["coordinate"].size(); ++i) {
    QPoint coordinate;
    coordinate.setX(json_config["coordinate"][i]["x"].get<int>());
    coordinate.setY(json_config["coordinate"][i]["y"].get<int>());
    screens_coordinate[json_config["coordinate"][i]["screen_id"].get<int>()] = coordinate;
  }

  // 多屏幕检测，判断鼠标当前在哪，然后将
  QPoint          cursorPos = QCursor::pos();
  QList<QScreen*> screens   = QGuiApplication::screens();
  for (int i = 0; i < screens.size(); ++i) {
    QScreen* screen = screens[i];
    if (screen->geometry().contains(cursorPos)) {
#ifdef _DEBUG
      qDebug() << "当前在屏幕 ==> " << i;
#endif

      this->move(screens_coordinate[i]);
      break;
    }
  }


  // 连接应用程序退出信号到保存坐标的槽函数
  // connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [&]() {
  // file_config->resize(0); // 清空文件
  // file_config->seek(0);
  // QTextStream config_content(file_config);
  // qDebug() << QString::fromStdString(json_config.dump(4));
  // config_content << QString::fromStdString(json_config.dump(4));
  // }
  // );
}

void MainWidget::closeEvent(QCloseEvent* event) {
  this->hide();
  event->ignore(); // 忽略关闭事件，避免消息传递导致程序退出
}

/**初始化系统右下角托盘*/
void MainWidget::init_tray() {
  trayIcon.setIcon(QIcon(":/res/Resource/uuz_tray.ico"));

  // 添加菜单项
  auto act_show = new QAction("Show", &trayMenu);
  // auto act_enable = new QAction("enable/disable", &trayMenu);
  auto act_exit = new QAction("Exit", &trayMenu);
  auto act_restart = new QAction("Restart", &trayMenu);

  trayMenu.addAction(act_show);
  trayMenu.addAction(act_exit);
  trayMenu.addAction(act_restart);

  // 设置托盘图标的上下文菜单
  trayIcon.setContextMenu(&trayMenu);

  //双击显示界面
  connect(&trayIcon, &QSystemTrayIcon::activated, this, &MainWidget::slot_onTrayIconActivated);

  // 连接菜单项的信号和槽
  connect(act_show, &QAction::triggered, [&]() {
    this->show();
  });

  // connect(act_enable, &QAction::triggered, [&]() {
  //   static bool enable = true;
  //   if (enable) {
  //     
  //   }
  //   else {
  //     
  //   }
  // });

  connect(act_exit, &QAction::triggered, [&]() {
    QApplication::quit();
  });

  //重启
  connect(act_restart, &QAction::triggered, [&]() {
    QStringList arguments = QCoreApplication::arguments();
    QString program = arguments.at(0);
    QProcess::startDetached(program, arguments);
    QCoreApplication::quit();
  });

  // 显示托盘图标
  trayIcon.show();
}

/**槽，托盘点击事件,控制显示或者隐藏*/
void MainWidget::slot_onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::DoubleClick) {
    if (this->isHidden()) {
      show();
    }
    else {
      hide();
    }
    //
    // raise();          // 确保窗口在其他窗口之上
    // activateWindow(); // 激活窗口
  }
}

/**槽，主要是设置界面修改json触发这里*/
void MainWidget::slot_modifyConfig() {
  file_config->resize(0); // 清空文件
  QTextStream config_content(file_config);
  config_content << QString::fromStdString(json_config.dump(4));
  is_changed = true; //标记配置文件已修改，重新加载配置文件
}

void MainWidget::slot_showConfigWidget() {
  
}

#ifdef _DEBUG
/** 设定为 当锁定状态下也可以移动，但是不会记录进入配置，只限于本次显示 */
void MainWidget::slot_action_show_move(bool & is_lock) {
  if (is_lock) {
    //锁定
    is_lock = false;
    is_move_lock = true;
  }
  else {
    //可移动
    is_lock = true;
    is_move_lock = false;
  }
}
#endif


void MainWidget::slot_showStackedWidgetIndex(const int index) const {
  stacked_widget->setCurrentIndex(index);
}

void MainWidget::slot_moveFocus(QWidget* widget) {
  if (widget != nullptr) {
    widget->setFocus();
  }
  else {
    this->hide(); //隐藏主窗口
  }
}
