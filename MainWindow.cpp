#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QRegularExpression>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringDecoder>
#include <QStringConverter>
#endif
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QScrollBar>
#include <QFont>
#include <QFontDatabase>
#include <QRandomGenerator>
#include <QDateTime>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>

// 定义SECRET_KEY常量（请根据实际情况修改此值）
const QString MainWindow::SECRET_KEY = "Y+Dh=@!=sJnDnlW3u-aY";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isRunning(false)
    , loopCount(0)
    , intervalTime(180)
    , uploadUrl("http://tt-tools.test/api/sszs/tool-upload")
    , rawInfoSuccess(false)
    , allPlaySuccess(false)
    , inquirySuccess(false)
    , playrankSuccess(false)
    , rawInfoReply(nullptr)
    , allPlayReply(nullptr)
    , inquiryReply(nullptr)
    , uploadReply(nullptr)
    , currentPlayrankReply(nullptr)
    , rawInfoTimer(nullptr)
    , allPlayTimer(nullptr)
    , inquiryTimer(nullptr)
    , uploadTimer(nullptr)
    , pendingRequests(0)
    , currentOnePlayReply(nullptr)
    , totalOnePlayRequests(0)
    , completedOnePlayRequests(0)
    , totalPlayrankRequests(0)
    , completedPlayrankRequests(0)
    , logFile(nullptr)
    , logStream(nullptr)
{
    networkManager = new QNetworkAccessManager(this);
    taskTimer = new QTimer(this);
    taskTimer->setSingleShot(true);
    connect(taskTimer, &QTimer::timeout, this, &MainWindow::executeTask);
    
    // 初始化超时定时器
    rawInfoTimer = new QTimer(this);
    rawInfoTimer->setSingleShot(true);
    connect(rawInfoTimer, &QTimer::timeout, this, [this]() {
        if (rawInfoReply && rawInfoReply->isRunning()) {
            addLog("获取赛程数据超时(5秒)", "warning");
            rawInfoReply->abort();
        }
    });
    
    allPlayTimer = new QTimer(this);
    allPlayTimer->setSingleShot(true);
    connect(allPlayTimer, &QTimer::timeout, this, [this]() {
        if (allPlayReply && allPlayReply->isRunning()) {
            addLog("获取比赛数据超时(5秒)", "warning");
            allPlayReply->abort();
        }
    });
    
    inquiryTimer = new QTimer(this);
    inquiryTimer->setSingleShot(true);
    connect(inquiryTimer, &QTimer::timeout, this, [this]() {
        if (inquiryReply && inquiryReply->isRunning()) {
            addLog("获取查询数据超时(5秒)", "warning");
            inquiryReply->abort();
        }
    });
    
    uploadTimer = new QTimer(this);
    uploadTimer->setSingleShot(true);
    connect(uploadTimer, &QTimer::timeout, this, [this]() {
        if (uploadReply && uploadReply->isRunning()) {
            addLog("上传数据超时(60秒)", "warning");
            uploadReply->abort();
        }
    });
    
    // 初始化日志文件
    initializeLogFile();
    
    setupUI();
    setupStyles();
    
    setWindowTitle("40+成绩发布系统V1.0.1beta");
    resize(1600, 1000);
}

MainWindow::~MainWindow()
{
    // 关闭日志文件
    if (logStream) {
        logStream->flush();
        delete logStream;
        logStream = nullptr;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
}

void MainWindow::initializeLogFile()
{
    // 创建 logs 目录
    QString appDir = QApplication::applicationDirPath();
    QString logsDir = appDir + "/logs";
    
    QDir dir;
    if (!dir.exists(logsDir)) {
        if (dir.mkpath(logsDir)) {
            qDebug() << "创建 logs 目录:" << logsDir;
        } else {
            qDebug() << "创建 logs 目录失败:" << logsDir;
            return;
        }
    }
    
    // 生成日志文件名：yyyy-MM-dd-HH-mm-ss.log
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
    QString logFileName = logsDir + "/" + timestamp + ".log";
    
    // 创建日志文件
    logFile = new QFile(logFileName);
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        logStream = new QTextStream(logFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // Qt5: 使用QTextCodec设置UTF-8编码
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        if (codec) {
            logStream->setCodec(codec);
        }
        logStream->setGenerateByteOrderMark(true);  // 添加 UTF-8 BOM，让 Windows 记事本正确识别
#else
        // Qt6: 使用setEncoding
        logStream->setEncoding(QStringConverter::Utf8);
        logStream->setGenerateByteOrderMark(true);  // 添加 UTF-8 BOM，让 Windows 记事本正确识别
#endif
        
        // 写入文件头 - 确保使用UTF-8编码
        QString header = QString::fromUtf8("========================================\n"
                                           "40+成绩发布系统 - 日志文件\n"
                                           "启动时间: %1\n"
                                           "日志文件: %2\n"
                                           "========================================\n")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                        .arg(logFileName);
        *logStream << header;
        logStream->flush();
        
        qDebug() << "日志文件已创建:" << logFileName;
    } else {
        qDebug() << "无法创建日志文件:" << logFileName;
        delete logFile;
        logFile = nullptr;
    }
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建分割器
    splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->setChildrenCollapsible(false);
    
    // 左侧面板
    leftPanel = new QWidget();
    leftPanel->setFixedWidth(380);
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(20);
    
    // Logo 区域
    logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setScaledContents(false);
    
    // 尝试加载 logo.png
    // 优先从嵌入的资源文件加载（打包在 exe 中）
    QString logoPath = ":/logo.png";
    QPixmap logo(logoPath);
    
    if (logo.isNull()) {
        // 如果资源文件加载失败，尝试从外部文件加载
        logoPath = QApplication::applicationDirPath() + "/logo.png";
        if (QFile::exists(logoPath)) {
            logo = QPixmap(logoPath);
        }
    }
    
    if (logo.isNull()) {
        // 尝试从当前目录加载
        logoPath = QDir::currentPath() + "/logo.png";
        if (QFile::exists(logoPath)) {
            logo = QPixmap(logoPath);
        }
    }
    
    if (!logo.isNull()) {
        // 设置最大尺寸，保持宽高比
        logo = logo.scaled(320, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logoLabel->setPixmap(logo);
    } else {
        // 如果所有方式都失败，显示占位文本
        logoLabel->setText("40+");
        logoLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #007bff;");
    }
    logoLabel->setObjectName("logoLabel");
    logoLabel->setMinimumHeight(80);
    
    // 输入区域
    inputSection = new QWidget();
    inputLayout = new QGridLayout(inputSection);
    inputLayout->setSpacing(18);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    
    // 左侧布局 - 先添加logo
    leftLayout->addWidget(logoLabel);
    leftLayout->addSpacing(10);
    
    // HTTP地址输入
    labelHttpAddress = new QLabel("HTTP地址");
    txtHttpAddress = new QLineEdit();
    txtHttpAddress->setPlaceholderText("请输入HTTP地址");
    txtHttpAddress->setText("http://127.0.0.1");
    
    // 间隔时间输入
    labelInterval = new QLabel("间隔时间(秒)");
    txtInterval = new QLineEdit();
    txtInterval->setPlaceholderText("请输入间隔时间（最小180秒）");
    txtInterval->setText("180");
    
    // API Key输入
    labelApiKey = new QLabel("赛事ID");
    txtApiKey = new QLineEdit();
    txtApiKey->setPlaceholderText("请输入赛事ID");
    
    // 添加到布局
    inputLayout->addWidget(labelHttpAddress, 0, 0);
    inputLayout->addWidget(txtHttpAddress, 1, 0);
    inputLayout->addWidget(labelInterval, 2, 0);
    inputLayout->addWidget(txtInterval, 3, 0);
    inputLayout->addWidget(labelApiKey, 4, 0);
    inputLayout->addWidget(txtApiKey, 5, 0);
    
    // 按钮区域
    btnStart = new QPushButton("开始执行");
    btnStart->setObjectName("btnStart");
    btnStart->setMinimumHeight(50);
    
    btnStop = new QPushButton("取消执行");
    btnStop->setObjectName("btnStop");
    btnStop->setMinimumHeight(50);
    
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    
    // 左侧布局 - 从上到下排列，不垂直居中
    leftLayout->addWidget(inputSection);
    leftLayout->addSpacing(20);
    leftLayout->addWidget(btnStart);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(btnStop);
    leftLayout->addStretch();  // 只在底部添加弹性空间，保持内容靠上
    
    // 右侧面板 - 日志区域
    logArea = new QTextEdit();
    logArea->setReadOnly(true);
    logArea->setFont(QFont("Consolas", 10));
    
    // 添加到分割器
    splitter->addWidget(leftPanel);
    splitter->addWidget(logArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    mainLayout->addWidget(splitter);
    
    // 初始状态
    updateUIState(false);
}

void MainWindow::setupStyles()
{
    // 设置样式表 - Bootstrap 风格
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f8f9fa;

            font-family: "Consolas", "Courier New", monospace;
        }
        
        QWidget#leftPanel {
            background-color: white;
            border-radius: 8px;
            padding: 5px;
        }
        
        QLabel#logoLabel {
            background-color: transparent;
            border: none;
            padding: 10px;
        }
        
        QLabel {
            font-weight: 600;
            color: #000000;
            font-size: 18px;
            margin-bottom: 6px;
            font-family: "Microsoft YaHei", "Courier New", monospace;
        }
        
        QLineEdit {
            padding: 12px 14px;
            font-size: 18px;
            color: #000000;
            font-family: "Consolas", "Courier New", monospace;
            font-weight: 400;
            background-color: #ffffff;
            border: 1px solid #ced4da;
            border-radius: 4px;
            selection-background-color: #007bff;
            selection-color: white;
        }
        
        QLineEdit:focus {
            border-color: #80bdff;
            outline: none;
            border-width: 2px;
        }
        
        QLineEdit:hover:!focus {
            border-color: #adb5bd;
        }
        
        QLineEdit:disabled {
            background-color: #e9ecef;
            color: #6c757d;
            border-color: #ced4da;
        }
        
        QTextEdit#logArea {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border-radius: 8px;
            padding: 15px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 14px;
            line-height: 1.6;
            border: 1px solid #343a40;
        }
    )";
    
    // 设置其他组件的 objectName
    logArea->setObjectName("logArea");
    leftPanel->setObjectName("leftPanel");
    
    // 应用主窗口样式表
    setStyleSheet(styleSheet);
    
    // 直接在按钮上设置样式（更可靠的方法）
    QString btnStartStyle = R"(
        QPushButton {
            color: white;
            background-color: #00c853;
            border: 2px solid #00c853;
            padding: 14px 24px;
            font-size: 18px;
            font-weight: 600;
            border-radius: 6px;

            font-family: "Consolas", "Courier New", monospace;
        }
        QPushButton:hover {
            background-color: #00b248;
            border: 2px solid #009624;
        }
        QPushButton:pressed {
            background-color: #009624;
            border: 2px solid #008018;
        }
        QPushButton:disabled {
            background-color: #9e9e9e;
            border: 2px solid #9e9e9e;
            color: #e0e0e0;
        }
    )";
    
    QString btnStopStyle = R"(
        QPushButton {
            color: white;
            background-color: #ff1744;
            border: 2px solid #ff1744;
            padding: 14px 24px;
            font-size: 18px;
            font-weight: 600;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #f50057;
            border: 2px solid #e91e63;
        }
        QPushButton:pressed {
            background-color: #e91e63;
            border: 2px solid #c2185b;
        }
        QPushButton:disabled {
            background-color: #9e9e9e;
            border: 2px solid #9e9e9e;
            color: #e0e0e0;
        }
    )";
    
    btnStart->setStyleSheet(btnStartStyle);
    btnStop->setStyleSheet(btnStopStyle);
}

void MainWindow::addLog(const QString &message, const QString &logType)
{
    QString color;
    QString prefix = "";
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    
    if (logType == "time") {
        color = "#569cd6";
    } else if (logType == "success") {
        color = "#4ec9b0";
    } else if (logType == "error") {
        color = "#f48771";
    } else if (logType == "warning") {
        color = "#dcdcaa";
    } else {
        color = "#9cdcfe";
    }
    
    if (logType == "time") {
        prefix = "[" + timestamp + "] ";
    }
    
    QString html = QString("<div style='color: %1; margin-bottom: 4px; word-wrap: break-word;'>%2%3</div>")
                   .arg(color)
                   .arg(prefix)
                   .arg(message.toHtmlEscaped());
    
    logArea->append(html);
    
    // 自动滚动到底部
    QScrollBar *scrollBar = logArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 同时写入日志文件
    if (logStream) {
        QString logText;
        if (logType == "time") {
            logText = prefix + message;
        } else {
            logText = "[" + timestamp + "] [" + logType.toUpper() + "] " + message;
        }
        *logStream << logText << "\n";
        logStream->flush(); // 立即刷新到文件
    }
}

void MainWindow::updateUIState(bool running)
{
    txtHttpAddress->setEnabled(!running);
    txtInterval->setEnabled(!running);
    txtApiKey->setEnabled(!running);
    btnStart->setEnabled(!running);
    btnStop->setEnabled(running);
}

bool MainWindow::validateInputs()
{
    httpAddress = txtHttpAddress->text().trimmed();
    QString intervalStr = txtInterval->text().trimmed();
    apiKey = txtApiKey->text().trimmed();
    
    if (httpAddress.isEmpty()) {
        addLog("错误: 请输入HTTP地址", "error");
        return false;
    }
    
    bool ok;
    intervalTime = intervalStr.toInt(&ok);
    if (!ok || intervalTime <= 0) {
        addLog("错误: 请输入有效的间隔时间（数字）", "error");
        return false;
    }
    
    if (intervalTime < 180) {
        addLog("错误: 间隔时间不得低于180秒", "error");
        return false;
    }
    
    if (apiKey.isEmpty()) {
        addLog("错误: 请输入赛事ID", "error");
        return false;
    }
    
    return true;
}

bool MainWindow::checkKey(const QString &key)
{
    // 按'-'分割key
    QStringList keylist = key.split('-');
    
    // 检查是否正好有2部分
    if (keylist.size() != 2) {
        return false;
    }
    
    // 计算 md5(keylist[1] + SECRET_KEY) 的前8个字符
    QString input = keylist[1] + SECRET_KEY;
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5);
    QString hashHex = hash.toHex().toLower();
    QString first8Chars = hashHex.left(8);
    
    // 检查第一部分是否等于MD5的前8个字符
    if (keylist[0] == first8Chars) {
        return true;
    }
    
    return false;
}

void MainWindow::onStartClicked()
{
    if (!validateInputs()) {
        return;
    }
    
    // 检查key的合法性
    if (!checkKey(apiKey)) {
        addLog("错误: 赛事ID格式不正确或验证失败", "error");
        QMessageBox::warning(this, "验证失败", "赛事ID格式不正确或验证失败，请检查后重试。");
        return;
    }
    
    isRunning = true;
    loopCount = 0;
    updateUIState(true);
    
    addLog("========================================", "info");
    addLog("开始执行数据获取和上传任务", "info");
    addLog("HTTP地址: " + httpAddress, "info");
    addLog("间隔时间: " + QString::number(intervalTime) + " 秒", "info");
    // addLog("上传接口: " + uploadUrl, "info");
    addLog("========================================", "info");
    
    executeTask();
}

void MainWindow::onStopClicked()
{
    isRunning = false;
    
    // 停止所有超时定时器
    rawInfoTimer->stop();
    allPlayTimer->stop();
    inquiryTimer->stop();
    uploadTimer->stop();
    
    // 取消所有正在进行的请求
    if (rawInfoReply) {
        rawInfoReply->abort();
        rawInfoReply->deleteLater();
        rawInfoReply = nullptr;
    }
    if (allPlayReply) {
        allPlayReply->abort();
        allPlayReply->deleteLater();
        allPlayReply = nullptr;
    }
    if (inquiryReply) {
        inquiryReply->abort();
        inquiryReply->deleteLater();
        inquiryReply = nullptr;
    }
    if (uploadReply) {
        uploadReply->abort();
        uploadReply->deleteLater();
        uploadReply = nullptr;
    }
    
    // 取消当前的oneplay请求并清空队列
    if (currentOnePlayReply) {
        currentOnePlayReply->abort();
        currentOnePlayReply->deleteLater();
        currentOnePlayReply = nullptr;
    }
    onePlayQueue.clear();
    totalOnePlayRequests = 0;
    completedOnePlayRequests = 0;
    
    // 清理playrank相关
    if (currentPlayrankReply) {
        currentPlayrankReply->abort();
        currentPlayrankReply->deleteLater();
        currentPlayrankReply = nullptr;
    }
    playrankQueue.clear();
    playrankResults.clear();
    totalPlayrankRequests = 0;
    completedPlayrankRequests = 0;
    playrankSuccess = false;
    playranksData.clear();
    
    taskTimer->stop();
    
    updateUIState(false);
    
    addLog("", "info");
    addLog("任务已停止", "warning");
}

QString MainWindow::detectEncoding(const QByteArray &data)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5: 使用QTextCodec
    // 尝试检测编码
    // 首先尝试UTF-8
    QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
    if (!utf8Codec) {
        return "UTF-8";
    }
    QString utf8Text = utf8Codec->toUnicode(data);
    
    // 检查是否有乱码特征
    bool hasGarbled = false;
    if (utf8Text.contains("Ã") || utf8Text.contains("Â") || utf8Text.contains("€")) {
        hasGarbled = true;
    }
    
    if (!hasGarbled) {
        return "UTF-8";
    }
    
    // 尝试GBK
    QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
    if (gbkCodec) {
        QString gbkText = gbkCodec->toUnicode(data);
        if (!gbkText.isEmpty()) {
            return "GBK";
        }
    }
    
    // 尝试GB2312
    QTextCodec *gb2312Codec = QTextCodec::codecForName("GB2312");
    if (gb2312Codec) {
        QString gb2312Text = gb2312Codec->toUnicode(data);
        if (!gb2312Text.isEmpty()) {
            return "GB2312";
        }
    }
    
    return "UTF-8"; // 默认返回UTF-8
#else
    // Qt6: 使用QStringDecoder
    // 首先尝试UTF-8
    QStringDecoder utf8Decoder(QStringDecoder::Utf8);
    QString utf8Text = utf8Decoder.decode(data);
    
    if (utf8Decoder.hasError()) {
        // UTF-8解码失败，尝试GBK
        QStringDecoder gbkDecoder("GBK");
        QString gbkText = gbkDecoder.decode(data);
        if (!gbkDecoder.hasError() && !gbkText.isEmpty()) {
            return "GBK";
        }
        
        // 尝试GB2312
        QStringDecoder gb2312Decoder("GB2312");
        QString gb2312Text = gb2312Decoder.decode(data);
        if (!gb2312Decoder.hasError() && !gb2312Text.isEmpty()) {
            return "GB2312";
        }
    }
    
    // 检查是否有乱码特征
    bool hasGarbled = false;
    if (utf8Text.contains("Ã") || utf8Text.contains("Â") || utf8Text.contains("€")) {
        hasGarbled = true;
    }
    
    if (hasGarbled) {
        // 尝试GBK
        QStringDecoder gbkDecoder("GBK");
        QString gbkText = gbkDecoder.decode(data);
        if (!gbkDecoder.hasError() && !gbkText.isEmpty()) {
            return "GBK";
        }
    }
    
    return "UTF-8";
#endif
}

QString MainWindow::convertToUtf8(const QByteArray &data, const QString &sourceEncoding)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5: 使用QTextCodec
    QTextCodec *codec = QTextCodec::codecForName(sourceEncoding.toUtf8());
    if (codec) {
        QString text = codec->toUnicode(data);
        return text;
    }
    return QString::fromUtf8(data);
#else
    // Qt6: 使用QStringDecoder
    QStringDecoder decoder(sourceEncoding.toUtf8());
    QString text = decoder.decode(data);
    if (decoder.hasError()) {
        // 解码失败，尝试UTF-8
        return QString::fromUtf8(data);
    }
    return text;
#endif
}

void MainWindow::fetchData()
{
    rawInfoSuccess = false;
    allPlaySuccess = false;
    inquirySuccess = false;
    playrankSuccess = false;
    playranksData.clear();
    playrankQueue.clear();
    playrankResults.clear();
    totalPlayrankRequests = 0;
    completedPlayrankRequests = 0;
    
    // 判断是否需要获取 inquiry 数据（只在第一次获取）
    bool needInquiry = (loopCount == 1);
    pendingRequests = needInquiry ? 3 : 2;
    
    // 本地接口超时时间：5秒
    const int localTimeout = 5000;
    
    // 获取rawinfo数据
    QUrl rawInfoUrl(httpAddress + "/rawinfo");
    QNetworkRequest rawInfoRequest(rawInfoUrl);
    rawInfoRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    rawInfoReply = networkManager->get(rawInfoRequest);
    connect(rawInfoReply, &QNetworkReply::finished, this, &MainWindow::onRawInfoFinished);
    
    // 设置超时定时器
    rawInfoTimer->start(localTimeout);
    
    // 获取allplay数据
    QUrl allPlayUrl(httpAddress + "/allplay");
    QNetworkRequest allPlayRequest(allPlayUrl);
    allPlayRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    allPlayReply = networkManager->get(allPlayRequest);
    connect(allPlayReply, &QNetworkReply::finished, this, &MainWindow::onAllPlayFinished);
    
    // 设置超时定时器
    allPlayTimer->start(localTimeout);
    
    // 只在第一次获取InquiryPage数据
    if (needInquiry) {
        QUrl inquiryUrl(httpAddress + "/InquiryPage");
        QNetworkRequest inquiryRequest(inquiryUrl);
        inquiryRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
        inquiryRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        inquiryReply = networkManager->post(inquiryRequest, QByteArray());
        connect(inquiryReply, &QNetworkReply::finished, this, &MainWindow::onInquiryFinished);
        
        // 设置超时定时器
        inquiryTimer->start(localTimeout);
        // addLog("首次执行，获取 inquiry 数据", "info");
    } else {
        // 不是第一次，跳过 inquiry，直接标记为成功
        inquirySuccess = true;
        // addLog("使用缓存的 inquiry 数据", "info");
    }
}

void MainWindow::onRawInfoFinished()
{
    // 停止超时定时器
    rawInfoTimer->stop();
    
    if (!isRunning) {
        if (rawInfoReply) {
            rawInfoReply->deleteLater();
            rawInfoReply = nullptr;
        }
        return;
    }
    
    if (rawInfoReply->error() == QNetworkReply::NoError) {
        QByteArray data = rawInfoReply->readAll();
        QString encoding = detectEncoding(data);
        rawInfoData = convertToUtf8(data, encoding);
        
        // addLog("成功获取数据（" + encoding + "解码），数据长度: " + QString::number(rawInfoData.length()) + " 字符", "success");
        rawInfoSuccess = true;
        
        if (encoding != "UTF-8") {
            // addLog("检测到源服务器使用" + encoding + "编码，已转换为UTF-8", "info");
        }
        
        // 解析rawInfo JSON，提取match数组的key字段
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(rawInfoData.toUtf8(), &parseError);
        
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QJsonArray matchArray = obj.value("match").toArray();
            
            // 清空队列和结果
            playrankQueue.clear();
            playrankResults.clear();
            
            // 提取每个match的key字段
            for (int i = 0; i < matchArray.size(); i++) {
                QJsonObject matchObj = matchArray[i].toObject();
                QString key = matchObj.value("key").toString();
                if (!key.isEmpty()) {
                    playrankQueue.append(key);
                }
            }
            
            totalPlayrankRequests = playrankQueue.size();
            completedPlayrankRequests = 0;
            
            if (totalPlayrankRequests > 0) {
                // 开始循环调用playrank接口
                fetchPlayrankData();
            } else {
                // 没有需要请求的key，直接标记为完成
                playrankSuccess = true;
                playranksData = "[]"; // 空数组
            }
        } else {
            addLog("解析rawInfo JSON失败: " + parseError.errorString(), "error");
            playrankSuccess = false;
            playranksData = "[]";
        }
    } else {
        addLog("获取赛程数据失败: " + rawInfoReply->errorString(), "error");
        rawInfoSuccess = false;
        playrankSuccess = false;
        playranksData = "[]";
    }
    
    rawInfoReply->deleteLater();
    rawInfoReply = nullptr;
    
    pendingRequests--;
    // addLog(QString("rawInfo完成，剩余请求: %1").arg(pendingRequests), "info");
    if (pendingRequests == 0) {
        // 三个主要接口都获取完成后，处理allPlayData
        processAllPlayData();
    }
}

void MainWindow::onAllPlayFinished()
{
    // 停止超时定时器
    allPlayTimer->stop();
    
    if (!isRunning) {
        if (allPlayReply) {
            allPlayReply->deleteLater();
            allPlayReply = nullptr;
        }
        return;
    }
    
    if (allPlayReply->error() == QNetworkReply::NoError) {
        QByteArray data = allPlayReply->readAll();
        QString encoding = detectEncoding(data);
        allPlayData = convertToUtf8(data, encoding);
        
        // 清理JSON数据 - 替换可能导致解析失败的模式
        allPlayData.replace(":,", ":\"\",");   // 替换 :, 为 :"",
        allPlayData.replace(":}", ":\"\"}");   // 替换 :} 为 :""}
        allPlayData.replace(":]", ":\"\"]");   // 替换 :] 为 :""]
        
        // addLog("成功获取数据（" + encoding + "解码），数据长度: " + QString::number(allPlayData.length()) + " 字符", "success");
        allPlaySuccess = true;
        
        if (encoding != "UTF-8") {
            // addLog("检测到源服务器使用" + encoding + "编码，已转换为UTF-8", "info");
        }
    } else {
        addLog("获取比赛数据失败: " + allPlayReply->errorString(), "error");
        allPlaySuccess = false;
    }
    
    allPlayReply->deleteLater();
    allPlayReply = nullptr;
    
    pendingRequests--;
    // addLog(QString("allPlay完成，剩余请求: %1").arg(pendingRequests), "info");
    if (pendingRequests == 0) {
        // 三个主要接口都获取完成后，处理allPlayData
        processAllPlayData();
    }
}

void MainWindow::onInquiryFinished()
{
    // 停止超时定时器
    inquiryTimer->stop();
    
    if (!isRunning) {
        if (inquiryReply) {
            inquiryReply->deleteLater();
            inquiryReply = nullptr;
        }
        return;
    }
    
    if (inquiryReply->error() == QNetworkReply::NoError) {
        QByteArray data = inquiryReply->readAll();
        QString encoding = detectEncoding(data);
        QString htmlData = convertToUtf8(data, encoding);
        
        // 将 HTML 数据转换为 JSON 格式
        inquiryData = processInquiryHtml(htmlData);
        
        // addLog("成功获取数据（" + encoding + "解码），数据长度: " + QString::number(inquiryData.length()) + " 字符", "success");
        inquirySuccess = true;
        
        if (encoding != "UTF-8") {
            // addLog("检测到源服务器使用" + encoding + "编码，已转换为UTF-8", "info");
        }
    } else {
        addLog("获取比赛数据失败: " + inquiryReply->errorString(), "error");
        inquirySuccess = false;
    }
    
    inquiryReply->deleteLater();
    inquiryReply = nullptr;
    
    pendingRequests--;
    // addLog(QString("inquiry完成，剩余请求: %1").arg(pendingRequests), "info");
    if (pendingRequests == 0) {
        // 三个主要接口都获取完成后，处理allPlayData
        processAllPlayData();
    }
}

void MainWindow::checkDataAndUpload()
{
    if (rawInfoSuccess && allPlaySuccess && inquirySuccess && playrankSuccess) {
        addLog("数据获取成功，准备上传", "success");
        uploadData();
    } else {
        // 如果playrank还在进行中，等待完成
        if (rawInfoSuccess && allPlaySuccess && inquirySuccess && !playrankSuccess && completedPlayrankRequests < totalPlayrankRequests) {
            // playrank还在处理中，等待完成
            return;
        }
        addLog("数据获取失败，跳过本次上传", "warning");
        scheduleNextTask();
    }
}

void MainWindow::uploadData()
{
    // 统计数据量大小
    qint64 rawInfoSize = rawInfoData.trimmed().toUtf8().size();
    qint64 allPlaySize = allPlayData.trimmed().toUtf8().size();
    qint64 inquirySize = (loopCount == 1) ? inquiryData.trimmed().toUtf8().size() : 0;
    qint64 totalSize = rawInfoSize + allPlaySize + inquirySize;
    
    // 转换为KB（保留2位小数）
    double rawInfoKB = rawInfoSize / 1024.0;
    double allPlayKB = allPlaySize / 1024.0;
    double inquiryKB = inquirySize / 1024.0;
    double totalKB = totalSize / 1024.0;
    
    // 输出统计信息
    addLog(QString("数据量统计:"), "info");
    // addLog(QString("  - rawinfo: %1 KB").arg(rawInfoKB, 0, 'f', 2), "info");
    // addLog(QString("  - allplay: %1 KB").arg(allPlayKB, 0, 'f', 2), "info");
    // if (loopCount == 1) {
    //     addLog(QString("  - inquiry: %1 KB").arg(inquiryKB, 0, 'f', 2), "info");
    // }
    addLog(QString("  - 总计: %1 KB").arg(totalKB, 0, 'f', 2), "success");
    
    // 生成multipart边界
    QString boundary = "----WebKitFormBoundary" + 
                       QDateTime::currentDateTime().toString("yyyyMMddhhmmss") +
                       QString::number(QRandomGenerator::global()->bounded(65535), 16);
    
    // 构建multipart/form-data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->setBoundary(boundary.toUtf8());
    
    // key字段
    QHttpPart keyPart;
    keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                     QVariant("form-data; name=\"key\""));
    keyPart.setBody(apiKey.toUtf8());
    multiPart->append(keyPart);
    
    // rawinfo字段
    QHttpPart rawinfoPart;
    rawinfoPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                         QVariant("form-data; name=\"rawinfo\""));
    rawinfoPart.setBody(rawInfoData.trimmed().toUtf8());
    multiPart->append(rawinfoPart);
    
    // allplay字段
    QHttpPart allplayPart;
    allplayPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                         QVariant("form-data; name=\"allplay\""));
    allplayPart.setBody(allPlayData.trimmed().toUtf8());
    multiPart->append(allplayPart);
    
    // inquiry字段（只在第一次上传）
    if (loopCount == 1 && !inquiryData.isEmpty()) {
        QHttpPart inquiryPart;
        inquiryPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                             QVariant("form-data; name=\"inquiry\""));
        inquiryPart.setBody(inquiryData.trimmed().toUtf8());
        multiPart->append(inquiryPart);
    }
    
    // playranks字段
    if (!playranksData.isEmpty()) {
        QHttpPart playranksPart;
        playranksPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                               QVariant("form-data; name=\"playranks\""));
        playranksPart.setBody(playranksData.toUtf8());
        multiPart->append(playranksPart);
    }
    
    // addLog("准备上传multipart/form-data", "info");
    // addLog("rawinfo长度: " + QString::number(rawInfoData.trimmed().length()) + " 字符", "info");
    // addLog("allplay长度: " + QString::number(allPlayData.trimmed().length()) + " 字符", "info");
    // addLog("inquiry长度: " + QString::number(inquiryData.trimmed().length()) + " 字符", "info");
    
    // 发送上传请求
    QUrl uploadUrlWithKey = QUrl(uploadUrl);
    QUrlQuery query;
    query.addQueryItem("key", apiKey);
    uploadUrlWithKey.setQuery(query);
    
    QNetworkRequest uploadRequest(uploadUrlWithKey);
    uploadRequest.setHeader(QNetworkRequest::ContentTypeHeader, 
                           "multipart/form-data; boundary=" + boundary);
    uploadRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    
    uploadReply = networkManager->post(uploadRequest, multiPart);
    multiPart->setParent(uploadReply); // 删除reply时自动删除multiPart
    
    connect(uploadReply, &QNetworkReply::finished, this, &MainWindow::onUploadFinished);
    
    // 服务器接口超时时间：60秒
    uploadTimer->start(60000);
}

void MainWindow::onUploadFinished()
{
    // 停止超时定时器
    uploadTimer->stop();
    
    if (!isRunning) {
        if (uploadReply) {
            uploadReply->deleteLater();
            uploadReply = nullptr;
        }
        return;
    }
    
    if (uploadReply->error() == QNetworkReply::NoError) {
        QByteArray response = uploadReply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            addLog("响应解析失败，原始响应: " + QString::fromUtf8(response), "error");
        } else {
            QJsonObject obj = doc.object();
            int code = obj.value("code").toInt(1); // 默认为失败
            QString msg = obj.value("msg").toString();
            if (code == 0) {
                addLog("上传成功，服务器响应: " + (msg.isEmpty() ? QString::fromUtf8(response) : msg), "success");
            } else {
                addLog("上传失败: " + (msg.isEmpty() ? QString::fromUtf8(response) : msg), "error");
            }
        }
    } else {
        QByteArray response = uploadReply->readAll();
        QString responseText = QString::fromUtf8(response);
        addLog("上传失败: " + uploadReply->errorString() + ", 响应: " + responseText, "error");
    }
    
    uploadReply->deleteLater();
    uploadReply = nullptr;
    
    scheduleNextTask();
}

void MainWindow::scheduleNextTask()
{
    if (isRunning) {
        addLog("等待 " + QString::number(intervalTime) + " 秒后继续...", "info");
        taskTimer->start(intervalTime * 1000);
        addLog("定时器已设置，将在 " + QString::number(intervalTime) + " 秒后继续", "info");
    } else {
        addLog("任务已停止，不再继续执行", "warning");
    }
}

void MainWindow::executeTask()
{
    if (!isRunning) {
        addLog("任务已停止，退出执行", "warning");
        return;
    }
    
    loopCount++;
    addLog("", "info");
    addLog("第 " + QString::number(loopCount) + " 次执行", "time");
    
    fetchData();
}

QString MainWindow::processInquiryHtml(const QString &html)
{
    // 解析 HTML 表格，提取 pid 和 groupno，构建 map 结构
    QJsonObject inqueryMap;
    
    // 使用正则表达式查找 tbody 中的所有 tr
    QRegularExpression tbodyRegex("<tbody[^>]*>(.*?)</tbody>", 
                                  QRegularExpression::DotMatchesEverythingOption | 
                                  QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch tbodyMatch = tbodyRegex.match(html);
    
    if (!tbodyMatch.hasMatch()) {
        addLog("数据格式错误", "warning");
        return "{}";
    }
    
    QString tbodyContent = tbodyMatch.captured(1);
    
    // 提取所有的 tr 标签
    QRegularExpression trRegex("<tr[^>]*>(.*?)</tr>", 
                               QRegularExpression::DotMatchesEverythingOption | 
                               QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator trIterator = trRegex.globalMatch(tbodyContent);
    
    int rowCount = 0;
    int validCount = 0;
    
    while (trIterator.hasNext()) {
        QRegularExpressionMatch trMatch = trIterator.next();
        QString trContent = trMatch.captured(1);
        rowCount++;
        
        // 提取所有的 td 标签内容
        QRegularExpression tdRegex("<td[^>]*>(.*?)</td>", 
                                   QRegularExpression::DotMatchesEverythingOption | 
                                   QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator tdIterator = tdRegex.globalMatch(trContent);
        
        QStringList tdContents;
        while (tdIterator.hasNext()) {
            QRegularExpressionMatch tdMatch = tdIterator.next();
            QString tdContent = tdMatch.captured(1).trimmed();
            // 移除 HTML 标签
            tdContent.remove(QRegularExpression("<[^>]*>"));
            // 移除前后空白
            tdContent = tdContent.trimmed();
            tdContents.append(tdContent);
        }
        
        // 检查是否有足够的列（至少需要 19 列：index 0-18）
        if (tdContents.size() >= 19) {
            QString pid = tdContents[1];      // 第 2 列（index 1）
            QString groupno = tdContents[18]; // 第 19 列（index 18）
            
            // 如果 pid 和 groupno 都不为空，添加到 map
            if (!pid.isEmpty() && !groupno.isEmpty()) {
                // 尝试将 groupno 转换为整数
                bool ok;
                int groupnoInt = groupno.toInt(&ok);
                if (ok) {
                    inqueryMap[pid] = groupnoInt;
                } else {
                    // 如果无法转换为整数，保持字符串形式
                    inqueryMap[pid] = groupno;
                }
                validCount++;
            }
        }
    }
    
    // addLog(QString("inquiry 数据处理: 共 %1 行，提取 %2 条有效数据")
    //        .arg(rowCount).arg(validCount), "info");
    
    // 转换为 JSON 字符串
    QJsonDocument doc(inqueryMap);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void MainWindow::processAllPlayData()
{
    // addLog("=== 开始处理allplay数据 ===", "info");
    
    // 如果allplay数据获取失败，直接进入检查和上传流程
    if (!allPlaySuccess) {
        // addLog("allplay数据获取失败，跳过处理", "warning");
        checkDataAndUpload();
        return;
    }
    
    // 解析allPlayData为JSON
    QJsonParseError parseError;
    allPlayJson = QJsonDocument::fromJson(allPlayData.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError || !allPlayJson.isObject()) {
        // addLog("解析allplay数据失败: " + parseError.errorString(), "error");
        checkDataAndUpload();
        return;
    }
    
    QJsonObject allPlayObj = allPlayJson.object();
    QJsonArray matchArray = allPlayObj.value("match").toArray();
    
    // 清空队列
    onePlayQueue.clear();
    totalOnePlayRequests = 0;
    completedOnePlayRequests = 0;
    
    // 统计各种状态的比赛数量
    int totalPlays = 0;
    int completedPlays = 0;  // state == 3
    int pendingPlays = 0;    // state != 3
    
    // 遍历所有match和play，构建请求队列
    for (int matchIdx = 0; matchIdx < matchArray.size(); ++matchIdx) {
        QJsonObject matchObj = matchArray[matchIdx].toObject();
        QJsonArray playArray = matchObj.value("play").toArray();
        
        for (int playIdx = 0; playIdx < playArray.size(); ++playIdx) {
            QJsonObject playObj = playArray[playIdx].toObject();
            int state = playObj.value("state").toInt();
            totalPlays++;
            
            // 如果state == 3（已完成的比赛），需要请求oneplay接口获取成绩
            if (state == 3) {
                completedPlays++;
                QString pid = playObj.value("pid").toString();
                OnePlayRequest req;
                req.matchIndex = matchIdx;
                req.playIndex = playIdx;
                req.pid = pid;
                onePlayQueue.append(req);
            } else {
                pendingPlays++;
            }
        }
    }
    
    // 输出统计信息
    // addLog(QString("比赛统计: 总计 %1 场，已完成 %2 场，待更新 %3 场")
    //        .arg(totalPlays).arg(completedPlays).arg(pendingPlays), "info");
    
    totalOnePlayRequests = onePlayQueue.size();
    
    // 如果没有需要请求的play，直接进入上传流程
    if (totalOnePlayRequests == 0) {
        addLog("没有需要更新的比赛数据", "info");
        checkDataAndUpload();
        return;
    }
    
    // addLog("发现 " + QString::number(totalOnePlayRequests) + " 场比赛需要更新详细数据（顺序执行）", "info");
    
    // 开始处理队列中的第一个请求
    fetchOnePlayData();
}

void MainWindow::fetchOnePlayData()
{
    // 如果队列为空，说明所有请求都已完成
    if (onePlayQueue.isEmpty()) {
        return;
    }
    
    // 如果已经有正在进行的请求，不要重复发送
    if (currentOnePlayReply && currentOnePlayReply->isRunning()) {
        return;
    }
    
    // 从队列头部取出一个请求
    OnePlayRequest req = onePlayQueue.takeFirst();
    
    // 构建请求URL（不带参数）
    QUrl onePlayUrl(httpAddress + "/oneplay");
    QNetworkRequest request(onePlayUrl);
    request.setRawHeader("User-Agent", "DataUploadTool/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    // 构建POST body，将pid放在body中
    QByteArray postData;
    postData.append("pid=");
    postData.append(req.pid.toUtf8());
    
    // 发送POST请求，带上body数据
    currentOnePlayReply = networkManager->post(request, postData);
    
    // 将 matchIndex 和 playIndex 存储为 reply 的动态属性
    currentOnePlayReply->setProperty("matchIndex", req.matchIndex);
    currentOnePlayReply->setProperty("playIndex", req.playIndex);
    currentOnePlayReply->setProperty("pid", req.pid);
    
    // 连接完成信号
    connect(currentOnePlayReply, &QNetworkReply::finished, this, &MainWindow::onOnePlayFinished);
    
    // 显示进度
    // addLog(QString("正在请求比赛详细数据 (%1/%2) PID: %3")
    //        .arg(completedOnePlayRequests + 1)
    //        .arg(totalOnePlayRequests)
    //        .arg(req.pid), "info");
}

void MainWindow::onOnePlayFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 从动态属性中获取索引
    int matchIdx = reply->property("matchIndex").toInt();
    int playIdx = reply->property("playIndex").toInt();
    QString pid = reply->property("pid").toString();
    
    // 处理响应
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString encoding = detectEncoding(data);
        QString scoreData = convertToUtf8(data, encoding);
        
        // 输出原始数据用于调试（限制长度避免日志过长）
        QString debugData = scoreData.left(200);
        if (scoreData.length() > 200) {
            debugData += "...";
        }
        // addLog(QString("PID: %1 原始数据: %2").arg(pid).arg(debugData), "info");
        
        // 清理JSON数据 - 替换可能导致解析失败的模式
        // 替换 :, 为 :"",
        scoreData.replace(":,", ":\"\",");
        // 替换 :} 为 :""} (对象末尾的空值)
        scoreData.replace(":}", ":\"\"}");
        // 替换 :] 为 :""] (数组末尾的空值)
        scoreData.replace(":]", ":\"\"]");
        
        // 解析JSON
        QJsonParseError parseError;
        QJsonDocument scoreDoc = QJsonDocument::fromJson(scoreData.toUtf8(), &parseError);
        
        if (parseError.error == QJsonParseError::NoError && scoreDoc.isObject()) {
            QJsonObject scoreObj = scoreDoc.object();
            QJsonObject scorePlayObj = scoreObj.value("play").toObject();
            
            // 更新allPlayJson中的数据
            QJsonObject allPlayObj = allPlayJson.object();
            QJsonArray matchArray = allPlayObj.value("match").toArray();
            
            if (matchIdx >= 0 && matchIdx < matchArray.size()) {
                QJsonObject matchObj = matchArray[matchIdx].toObject();
                QJsonArray playArray = matchObj.value("play").toArray();
                
                if (playIdx >= 0 && playIdx < playArray.size()) {
                    QJsonObject playObj = playArray[playIdx].toObject();
                    
                    // 更新score字段
                    if (scorePlayObj.contains("score") && !scorePlayObj.value("score").isNull()) {
                        QJsonValue scoreValue = scorePlayObj.value("score");
                        // 将score转为JSON字符串存储
                        QJsonDocument scoreSubDoc;
                        if (scoreValue.isObject()) {
                            scoreSubDoc = QJsonDocument(scoreValue.toObject());
                        } else if (scoreValue.isArray()) {
                            scoreSubDoc = QJsonDocument(scoreValue.toArray());
                        }
                        playObj["score"] = QString::fromUtf8(scoreSubDoc.toJson(QJsonDocument::Compact));
                    }
                    
                    // 更新member字段
                    if (scorePlayObj.contains("member") && !scorePlayObj.value("member").isNull()) {
                        QJsonValue memberValue = scorePlayObj.value("member");
                        // 将member转为JSON字符串存储
                        QJsonDocument memberSubDoc;
                        if (memberValue.isObject()) {
                            memberSubDoc = QJsonDocument(memberValue.toObject());
                        } else if (memberValue.isArray()) {
                            memberSubDoc = QJsonDocument(memberValue.toArray());
                        }
                        playObj["member"] = QString::fromUtf8(memberSubDoc.toJson(QJsonDocument::Compact));
                    }
                    
                    // 更新回数组
                    playArray[playIdx] = playObj;
                    matchObj["play"] = playArray;
                    matchArray[matchIdx] = matchObj;
                    allPlayObj["match"] = matchArray;
                    allPlayJson.setObject(allPlayObj);
                    
                    // addLog(QString("成功更新比赛数据 (%1/%2) PID: %3")
                    //        .arg(completedOnePlayRequests + 1)
                    //        .arg(totalOnePlayRequests)
                    //        .arg(pid), "success");
                }
            }
        } else {
            // 显示详细的解析错误信息
            // QString errorMsg = QString("解析比赛数据失败 PID: %1 - %2 (位置: %3)")
            //                   .arg(pid)
            //                   .arg(parseError.errorString())
            //                   .arg(parseError.offset);
            // addLog(errorMsg, "warning");
            
            // 显示错误位置附近的数据
            int offset = parseError.offset;
            int start = qMax(0, offset - 50);
            int length = qMin(100, scoreData.length() - start);
            QString context = scoreData.mid(start, length);
            // addLog(QString("错误位置附近的数据: ...%1...").arg(context), "warning");
        }
    } else {
        // addLog(QString("获取比赛数据失败 PID: %1 - %2").arg(pid).arg(reply->errorString()), "warning");
    }
    
    // 清理当前请求
    reply->deleteLater();
    currentOnePlayReply = nullptr;
    
    // 计数器增加
    completedOnePlayRequests++;
    
    // 检查是否所有请求都完成
    if (completedOnePlayRequests >= totalOnePlayRequests) {
        // addLog("所有比赛详细数据已处理完成", "success");
        
        // 将更新后的JSON转回字符串
        allPlayData = QString::fromUtf8(allPlayJson.toJson(QJsonDocument::Compact));
        
        // 继续上传流程
        checkDataAndUpload();
    } else {
        // 继续处理队列中的下一个请求
        fetchOnePlayData();
    }
}

void MainWindow::fetchPlayrankData()
{
    // 如果队列为空，说明所有请求都已完成
    if (playrankQueue.isEmpty()) {
        return;
    }
    
    // 如果已经有正在进行的请求，不要重复发送
    if (currentPlayrankReply && currentPlayrankReply->isRunning()) {
        return;
    }
    
    // 从队列头部取出一个key
    QString key = playrankQueue.takeFirst();
    
    // 构建请求URL
    QUrl playrankUrl(httpAddress + "/playrank");
    QNetworkRequest request(playrankUrl);
    request.setRawHeader("User-Agent", "DataUploadTool/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    // 构建POST body，将key放在body中
    QByteArray postData;
    postData.append("key=");
    postData.append(QUrl::toPercentEncoding(key));
    
    // 发送POST请求，带上body数据
    currentPlayrankReply = networkManager->post(request, postData);
    
    // 将key存储为reply的动态属性
    currentPlayrankReply->setProperty("key", key);
    
    // 连接完成信号
    connect(currentPlayrankReply, &QNetworkReply::finished, this, &MainWindow::onPlayrankFinished);
}

void MainWindow::onPlayrankFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 从动态属性中获取key
    QString key = reply->property("key").toString();
    
    // 处理响应
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString encoding = detectEncoding(data);
        QString playrankData = convertToUtf8(data, encoding);
        
        // 将响应数据添加到结果列表
        playrankResults.append(playrankData);
    } else {
        // 请求失败，添加空字符串或错误标记
        addLog("获取playrank数据失败，key: " + key + ", 错误: " + reply->errorString(), "warning");
        playrankResults.append(""); // 添加空字符串保持索引一致
    }
    
    // 清理当前请求
    reply->deleteLater();
    currentPlayrankReply = nullptr;
    
    // 计数器增加
    completedPlayrankRequests++;
    
    // 检查是否所有请求都完成
    if (completedPlayrankRequests >= totalPlayrankRequests) {
        // 将所有结果打包成JSON数组
        QJsonArray playranksArray;
        for (const QString &result : playrankResults) {
            if (!result.isEmpty()) {
                // 尝试解析为JSON，如果成功则添加解析后的对象，否则添加为字符串
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8(), &parseError);
                if (parseError.error == QJsonParseError::NoError) {
                    if (doc.isArray()) {
                        // 如果是数组，将数组中的每个元素添加到playranksArray
                        QJsonArray resultArray = doc.array();
                        for (const QJsonValue &value : resultArray) {
                            playranksArray.append(value);
                        }
                    } else if (doc.isObject()) {
                        playranksArray.append(doc.object());
                    } else {
                        playranksArray.append(result);
                    }
                } else {
                    // 解析失败，作为字符串添加
                    playranksArray.append(result);
                }
            }
        }
        
        // 转换为JSON字符串
        QJsonDocument playranksDoc(playranksArray);
        playranksData = QString::fromUtf8(playranksDoc.toJson(QJsonDocument::Compact));
        
        playrankSuccess = true;
        // addLog("所有playrank数据已处理完成", "success");
        
        // 检查是否可以上传（如果其他数据都已准备好）
        checkDataAndUpload();
    } else {
        // 继续处理下一个请求
        fetchPlayrankData();
    }
}

