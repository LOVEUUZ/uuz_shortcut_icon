﻿#pragma once

#include <QWidget>
#include <QString>
#include <QThread>
#include <atomic>
#include "Everything.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QPair>
// #include <QMutex>
#include <QDesktopServices>
#include <QUrl>
#include <QContextMenuEvent>
#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>


class Search_content : public QTableWidget {
    Q_OBJECT

  public:
    Search_content(QWidget* parent = nullptr);
    ~Search_content() override;

    static Search_content* get_searchContentWidget() { return search_content_widget; }

  private:
    static Search_content* search_content_widget;

    std::atomic<bool> isSearching;   // 跟踪搜索状态
    QThread*          searchThread;  // 异步搜索线程
    QString           lastSearch;    // 上次搜索的文本
    int               max_results;   //默认1k5个搜索结果
    DWORD             total_results; //所有结果数量

    //避免在槽函数执行addItems的时候子线程clear导致错误
    // QMutex            mutex;
    std::atomic<bool> is_clear;

    void        performSearch(const QString & text);
    void        resetSearch(); // 重置状态
    static void handleErrors(DWORD errorCode, const QString & text);

    //布局相关
    void init_layout();
    void resizeEvent(QResizeEvent* event) override; //列宽调整


    //过滤条件
    void               init_filter_config();
    QString            file_filter_path;
    QString            file_filter_suffix;
    QFile*             file_config_filter_path;
    QFile*             file_config_filter_suffix;
    static QStringList filter_path_list;
    static QStringList filter_suffix_list;

    //过滤配置文件的获取
  public:
    static QPair<QStringList&, QStringList&> get_filter() {
      return {filter_path_list, filter_suffix_list};
    }

  private:
    //右键点击事件（左键有信号，右键没有）
    void contextMenuEvent(QContextMenuEvent* event) override;

  signals:
    void sig_show(bool is_show);
    void sig_addItem(const QStringList & path_list);

  public slots:
    void slot_textChange(const QString & text);
    void slot_configFilterModify();

  private slots:
    void slot_addItem(const QStringList & path_list);
    void slot_openFile(const QTableWidgetItem* item);
    // void slot_file_(const QListWidgetItem* item);


  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
