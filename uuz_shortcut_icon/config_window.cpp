#include "config_window.h"


using json = nlohmann::json;

Config_window::Config_window(QWidget* parent) :
	QWidget(parent), filter_path_list(Search_content::get_filter().first),
	filter_suffix_list(Search_content::get_filter().second) {
	ui.setupUi(this);

	main_widget = MainWidget::get_mainWidget();
	if (main_widget == nullptr)
		qFatal("main_widget 为空!!!");

	// 先卸载钩子，记得关闭该页面的时候重新加上钩子
	WindowsHookKeyEx::getWindowHook()->unInstallHook();
	WindowsHookMouseEx::getWindowHook()->unInstallHook();

	setAttribute(Qt::WA_ShowModal, true); //模态

#ifdef NDEBUG
	//置顶
	this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
#endif


	init_config();

	init_translate();

	//开机自启配置修改
	connect(ui.checkBox_is_boot_start, &QCheckBox::checkStateChanged, this, &Config_window::slot_checkBoxIsBootStart);
	connect(this, &Config_window::sig_checkBoxIsBootStart, main_widget, &MainWidget::slot_modifyConfig);

	//日志保留天数配置修改
	connect(ui.spinBox_log_day, &QSpinBox::valueChanged, this, &Config_window::slot_spinBoxValueChanged);

	//打开配置文件路径
	connect(ui.btn_openFilePath, &QPushButton::clicked, this, &Config_window::slot_openConfigPath);
	connect(ui.btn_openLogFilePath, &QPushButton::clicked, this, &Config_window::slot_openConfigPath);
	connect(ui.btn_about, &QPushButton::clicked, this, &Config_window::slot_aboutDialog);

	//过滤相关配置修改

	//ctrl和alt和计数器的绑定
	connect(ui.checkBox_Ctrl, &QCheckBox::checkStateChanged, this, &Config_window::slot_checkBoxCtrlChanged);
	connect(ui.checkBox_Alt, &QCheckBox::checkStateChanged, this, &Config_window::slot_checkBoxAltChanged);
	connect(ui.spinBox_Ctrl, QOverload<int>::of(&QSpinBox::valueChanged), this, &Config_window::slot_spinBoxCtrlChanged);
	connect(ui.spinBox_Alt, QOverload<int>::of(&QSpinBox::valueChanged), this, &Config_window::slot_spinBoxAltChanged);



	//快捷键
	ui.lineEdit_shortcut_edit->installEventFilter(this); 

	//清空快捷键输入框按钮
	connect(ui.btn_clear, &QPushButton::clicked, this, &Config_window::slot_clearButton_clicked);
}

Config_window::~Config_window() {}

void Config_window::closeEvent(QCloseEvent* event) {
	WindowsHookKeyEx::getWindowHook()->installHook();
	WindowsHookMouseEx::getWindowHook()->installHook();
#ifdef _DEBUG
	qDebug() << "钩子挂上成功";
#endif
	QWidget::closeEvent(event);
}

/**读取配置并赋值*/
void Config_window::init_config() {
	json& json_config = main_widget->get_jsonConfig();

	//开机启动填充
	if (!json_config.contains(is_boot_start)) {
		json_config[is_boot_start] = false;
		emit sig_checkBoxIsBootStart();
	}

	if (json_config.at(is_boot_start).get<bool>()) {
		config_is_boot_start = true;
		ui.checkBox_is_boot_start->setCheckState(Qt::Checked);
	}
	else {
		config_is_boot_start = false;
		ui.checkBox_is_boot_start->setCheckState(Qt::Unchecked);
	}

	//todo 设置语言  翻译暂时没做


	//日志保留天数填充
	if (!json_config.contains(log_retain_day)) {
		json_config[log_retain_day] = 7;
		emit sig_checkBoxIsBootStart();
	}
	ui.spinBox_log_day->setValue(json_config[log_retain_day].get<int>());


	//两个文本编辑器内容填充
	QString filter_path = filter_path_list.join("\n");
	QString filter_suffix = filter_suffix_list.join("\n");

	// 检查并设置默认值
	if (!json_config.contains("Ctrl")) json_config["Ctrl"] = true; // 默认 Ctrl 勾选
	if (!json_config.contains("Alt")) json_config["Alt"] = false;
	if (!json_config.contains("CtrlCount")) json_config["CtrlCount"] = 2; // 默认 Ctrl 连按 2 次
	if (!json_config.contains("AltCount")) json_config["AltCount"] = 2;

	ui.textEdit_filterPath->setText(filter_path);
	ui.textEdit_filterSuffix->setText(filter_suffix);

	bool isCtrlChecked = json_config[Ctrl].get<bool>();
	bool isAltChecked = json_config[Alt].get<bool>();
	ui.checkBox_Ctrl->setCheckState(isCtrlChecked ? Qt::Checked : Qt::Unchecked);
	ui.checkBox_Alt->setCheckState(isAltChecked ? Qt::Checked : Qt::Unchecked);
	// 如果 Ctrl 或 Alt 任意一个为 选中，则禁用 lineEdit
	bool enable = !(isCtrlChecked || isAltChecked);
	ui.lineEdit_shortcut_edit->setEnabled(enable);
	if (!enable) ui.lineEdit_shortcut_edit->clear();

	ui.spinBox_Ctrl->setValue(json_config[CtrlCount].get<int>());
	ui.spinBox_Alt->setValue(json_config[AltCount].get<int>());

	const auto& shortcut = json_config[shortcut_key_msg];
	std::vector<std::string> vec_keyName;
	QString keysText;

	if (shortcut.contains("str_key_list")) {
		vec_keyName = shortcut["str_key_list"].get<std::vector<std::string>>();
	}
	for (size_t i = 0; i < vec_keyName.size(); ++i) {
		keysText += QString::fromStdString(vec_keyName[i]);
		if (i != vec_keyName.size() - 1) {
			keysText += "+";
		}
	}

	ui.lineEdit_shortcut_edit->setText(keysText);

}	

/**初始化翻译相关*/
void Config_window::init_translate() {
	ui.checkBox_is_boot_start->setText(tr("开启自启动"));
	ui.label_setLanguage->setText(tr("设置语言"));
	ui.label_log_text->setText(tr("日志保留天数"));
	ui.btn_about->setText(tr("关于"));
	ui.btn_openLogFilePath->setText(tr("打开日志文件"));
	ui.btn_openFilePath->setText(tr("打开配置文件目录"));
	ui.label_filterPath->setText(tr("过滤路径"));
	ui.label_filterSuffix->setText(tr("过滤后缀"));
}


/** 开机自启设置 */
void Config_window::slot_checkBoxIsBootStart(int state) {
	json& json_config = main_widget->get_jsonConfig();
	//未勾选,取消开机启动
	if (state == 0 && config_is_boot_start) {
		json_config[is_boot_start] = false;
		config_is_boot_start = false;
		{
			QString targetFilePath = QCoreApplication::applicationFilePath();
			QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup";
			QString shortcutFilePath = startupDir + "/" + QFileInfo(targetFilePath).baseName() + ".lnk";

			QFile shortcutFile(shortcutFilePath);
			if (shortcutFile.exists() && shortcutFile.remove()) {
				qInfo() << "取消开机启动设置成功";
			}
			else {
				qWarning() << "取消开机启动设置失败";
				QMessageBox::warning(this, tr("警告"), tr("取消开机启动设置失败"));
			}
		}
	}
	//勾选，设置开机启动
	else if (state == 2 && !config_is_boot_start) {
		json_config["is_boot_start"] = true;
		config_is_boot_start = true;
		{
			const QString& targetFilePath = QCoreApplication::applicationFilePath();
			const QString& shortcutFilePath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) +
				"/Startup"
				+ "/" + QFileInfo(targetFilePath).baseName() + ".lnk";
			const QString& description = "";
			QFile           shortcutFile(shortcutFilePath);

			if (shortcutFile.exists()) {
				qInfo() << "开机启动设置成功.";
			}

			QString vbsScript = QDir::temp().absoluteFilePath("createShortcut.vbs");
			QFile   file(vbsScript);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				QTextStream out(&file);
				QString     targetPath = targetFilePath;
				QString     shortcutPath = shortcutFilePath;
				targetPath.replace("/", "\\");
				shortcutPath.replace("/", "\\");

				out << "Set oWS = WScript.CreateObject(\"WScript.Shell\")\n";
				out << "sLinkFile = \"" << shortcutPath << "\"\n";
				out << "Set oLink = oWS.CreateShortcut(sLinkFile)\n";
				out << "oLink.TargetPath = \"" << targetPath << "\"\n";
				out << "oLink.WorkingDirectory = \"" << QFileInfo(targetPath).absolutePath().replace("/", "\\") << "\"\n";
				if (!description.isEmpty()) {
					out << "oLink.Description = \"" << description << "\"\n";
				}
				out << "oLink.Save\n";
				file.close();
			}
			else {
				qWarning() << "Failed to create VBS file.开机启动设置失败";
			}

			QProcess process;
			process.start("wscript", QStringList() << vbsScript);
			process.waitForFinished();
			file.remove();
		}
	}

	emit sig_checkBoxIsBootStart();
}

void Config_window::slot_spinBoxValueChanged(int day) {
	json& json_config = main_widget->get_jsonConfig();
	json_config[log_retain_day] = day;
	Logger::getLogger().set_retention_days(7);

	emit sig_checkBoxIsBootStart();
}

void Config_window::slot_openConfigPath() {
	QString filePath_tmp = QCoreApplication::applicationDirPath();

	auto objName = sender()->objectName();
	if (objName == "btn_openLogFilePath") {
		filePath_tmp += "/log";
	}
	else if (objName == "btn_openFilePath") {
		filePath_tmp += "/config";
	}
	QUrl    fileUrl = QUrl::fromLocalFile(filePath_tmp);
	QDesktopServices::openUrl(fileUrl);
	qInfo() << "打开配置文件路径: " << filePath_tmp;
}

void Config_window::slot_aboutDialog() {
	QDialog aboutDialog(this);
	aboutDialog.setWindowTitle(tr("关于"));
	aboutDialog.setMinimumSize(500, 500);

	QVBoxLayout* mainLayout = new QVBoxLayout(&aboutDialog);

	QLabel* linkLabel = new QLabel(&aboutDialog);
	linkLabel->setText("<a href=\"https://github.com/LOVEUUZ/shortcut_key\">https://github.com/LOVEUUZ/shortcut_key</a>");
	linkLabel->setOpenExternalLinks(true);

	QPushButton* closeButton = new QPushButton(tr("关闭"), &aboutDialog);
	connect(closeButton, &QPushButton::clicked, &aboutDialog, &QDialog::accept);

	mainLayout->addWidget(linkLabel);
	mainLayout->addWidget(closeButton);

	aboutDialog.setLayout(mainLayout);
	aboutDialog.exec();
}



#ifdef _DEBUG
static void printStdVector(const std::vector<uint32_t>& vec) {
	QStringList list;
	for (auto v : vec) {
		list << QString::number(v);
	}
	qDebug() << "按键序列:" << list.join(", ");
}
#endif



static int qtKeyToVk(QKeyEvent* event) {
	int vk = event->nativeVirtualKey();
	int sc = event->nativeScanCode();

	// 根据扫描码区分左右Ctrl和左右Alt（以常见扫描码为例）
	// 左Ctrl扫描码：0x1D
	// 右Ctrl扫描码：0xE01D
	// 左Alt扫描码：0x38
	// 右Alt扫描码：0xE038

	if (vk == VK_CONTROL) {
		if (sc == 0x1D) return VK_LCONTROL;    //左Ctrl 162
		else if (sc == 0xE01D || sc == 0x9D) return VK_RCONTROL; //右Ctrl 163
	}
	else if (vk == VK_MENU) {
		if (sc == 0x38) return VK_LMENU;      //左Alt 164
		else if (sc == 0xE038) return VK_RMENU; //右Alt 165
	}
	return vk;
}

/** 捕获快捷键 */
bool Config_window::eventFilter(QObject* obj, QEvent* event) {
	if (obj == ui.lineEdit_shortcut_edit) {
		// 禁用输入法
		ui.lineEdit_shortcut_edit->setAttribute(Qt::WA_InputMethodEnabled, false);

		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			int vk_key = qtKeyToVk(key_event);

			if (vk_key == 0) return true; // 跳过无效键

			if (!recording) {
				recording = true;
				first_key_code = vk_key;
				shortcut_msg = ShortcutKeyMsg(); // 清空旧数据
			}

			if (shortcut_msg.key_value_serial_number.size() >= 3)
				return true;

			if (std::find(shortcut_msg.key_value_serial_number.begin(),
				shortcut_msg.key_value_serial_number.end(),
				vk_key) == shortcut_msg.key_value_serial_number.end()) {

				shortcut_msg.key_value_serial_number.push_back(vk_key);
				shortcut_msg.key_value_total += vk_key;

				QString text = QKeySequence(key_event->key()).toString(QKeySequence::NativeText).toUpper();
				if (text.isEmpty()) {
					switch (vk_key) {
					case Qt::Key_Space: text = "SPACE"; break;
					case Qt::Key_Tab:   text = "TAB"; break;
					default:            text = QString("VK_%1").arg(vk_key); break;
					}
				}

				shortcut_msg.str_key_list.push_back(text.toStdString());

				// 更新 UI 显示
				QStringList key_strings;
				for (const auto& s : shortcut_msg.str_key_list)
					key_strings << QString::fromStdString(s);
				ui.lineEdit_shortcut_edit->setText(key_strings.join('+'));
			}

			return true;
		}

		if (event->type() == QEvent::KeyRelease) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			int qt_key = key_event->key();
			int vk_key = qtKeyToVk(key_event);

			if (vk_key == 0) return true;

			if (recording && vk_key == first_key_code) {
				recording = false;
				first_key_code = -1;

#ifdef _DEBUG
				qDebug() << "记录结束，键值总和:" << shortcut_msg.key_value_total;
				printStdVector(shortcut_msg.key_value_serial_number);
#endif
			}

			// 写入配置
			json& json_config = main_widget->get_jsonConfig();
			json_config[shortcut_key_msg] = shortcut_msg;
			emit sig_checkBoxIsBootStart();
			return true;
		}
	}

	return QWidget::eventFilter(obj, event);
}


void Config_window::slot_checkBoxCtrlChanged(int state) {
	json& json_config = main_widget->get_jsonConfig();

	if (state == Qt::Checked) {
		json_config["Ctrl"] = true;
		json_config["Alt"] = false;

		ui.checkBox_Alt->setCheckState(Qt::Unchecked);
		ui.lineEdit_shortcut_edit->setEnabled(false); // 禁用输入框
		ui.lineEdit_shortcut_edit->clear();
	}
	else if (state == Qt::Unchecked) {
		json_config["Ctrl"] = false;

		// 如果 Alt 也未勾选，启用输入框
		if (ui.checkBox_Alt->checkState() == Qt::Unchecked) {
			ui.lineEdit_shortcut_edit->setEnabled(true);
		}
	}

	emit sig_checkBoxIsBootStart();
}

void Config_window::slot_checkBoxAltChanged(int state) {
	json& json_config = main_widget->get_jsonConfig();

	if (state == Qt::Checked) {
		json_config["Alt"] = true;
		json_config["Ctrl"] = false;

		ui.checkBox_Ctrl->setCheckState(Qt::Unchecked);
		ui.lineEdit_shortcut_edit->setEnabled(false); // 禁用输入框
		ui.lineEdit_shortcut_edit->clear();
	}
	else if (state == Qt::Unchecked) {
		json_config["Alt"] = false;

		// 如果 Ctrl 也未勾选，启用输入框
		if (ui.checkBox_Ctrl->checkState() == Qt::Unchecked) {
			ui.lineEdit_shortcut_edit->setEnabled(true);
		}
	}

	emit sig_checkBoxIsBootStart();
}


void Config_window::slot_spinBoxCtrlChanged(int val) {
	json& json_config = main_widget->get_jsonConfig();
	json_config[CtrlCount] = val;
	emit sig_checkBoxIsBootStart();
}

void Config_window::slot_spinBoxAltChanged(int val) {
	json& json_config = main_widget->get_jsonConfig();
	json_config[AltCount] = val;
	emit sig_checkBoxIsBootStart();
}

void Config_window::slot_clearButton_clicked(){
	ui.lineEdit_shortcut_edit->clear();
	shortcut_msg = ShortcutKeyMsg(); // 创建一个新的空数据覆盖旧数据
	recording = false;
	first_key_code = -1;

	json& json_config = main_widget->get_jsonConfig();
	if (json_config.contains(shortcut_key_msg)) {
		json_config.erase(shortcut_key_msg);
	}

	emit sig_checkBoxIsBootStart();
}
