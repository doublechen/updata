#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QRegularExpression>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringDecoder>
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isRunning(false)
    , loopCount(0)
    , intervalTime(60)
    , uploadUrl("https://tools.40plus.cn/api/updata")
    , rawInfoSuccess(false)
    , allPlaySuccess(false)
    , inquirySuccess(false)
    , rawInfoReply(nullptr)
    , allPlayReply(nullptr)
    , inquiryReply(nullptr)
    , uploadReply(nullptr)
    , pendingRequests(0)
{
    networkManager = new QNetworkAccessManager(this);
    taskTimer = new QTimer(this);
    taskTimer->setSingleShot(true);
    connect(taskTimer, &QTimer::timeout, this, &MainWindow::executeTask);
    
    setupUI();
    setupStyles();
    
    setWindowTitle("40+成绩发布系统");
    resize(1600, 1000);
}

MainWindow::~MainWindow()
{
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
    txtInterval->setPlaceholderText("请输入间隔时间");
    txtInterval->setText("60");
    
    // API Key输入
    labelApiKey = new QLabel("赛事ID");
    txtApiKey = new QLineEdit();
    txtApiKey->setPlaceholderText("请输入调用key");
    
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
        prefix = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] ";
    }
    
    QString html = QString("<div style='color: %1; margin-bottom: 4px; word-wrap: break-word;'>%2%3</div>")
                   .arg(color)
                   .arg(prefix)
                   .arg(message.toHtmlEscaped());
    
    logArea->append(html);
    
    // 自动滚动到底部
    QScrollBar *scrollBar = logArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
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
    
    if (apiKey.isEmpty()) {
        addLog("错误: 请输入调用key", "error");
        return false;
    }
    
    return true;
}

void MainWindow::onStartClicked()
{
    if (!validateInputs()) {
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
    pendingRequests = 3;
    
    // 本地接口超时时间：5秒
    const int localTimeout = 5000;
    
    // 获取rawinfo数据
    QUrl rawInfoUrl(httpAddress + "/rawinfo");
    QNetworkRequest rawInfoRequest(rawInfoUrl);
    rawInfoRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    rawInfoReply = networkManager->get(rawInfoRequest);
    connect(rawInfoReply, &QNetworkReply::finished, this, &MainWindow::onRawInfoFinished);
    
    // 设置超时定时器
    QTimer::singleShot(localTimeout, this, [this]() {
        if (rawInfoReply && rawInfoReply->isRunning()) {
            addLog("获取赛程数据超时(5秒)", "warning");
            rawInfoReply->abort();
        }
    });
    
    // 获取allplay数据
    QUrl allPlayUrl(httpAddress + "/allplay");
    QNetworkRequest allPlayRequest(allPlayUrl);
    allPlayRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    allPlayReply = networkManager->get(allPlayRequest);
    connect(allPlayReply, &QNetworkReply::finished, this, &MainWindow::onAllPlayFinished);
    
    // 设置超时定时器
    QTimer::singleShot(localTimeout, this, [this]() {
        if (allPlayReply && allPlayReply->isRunning()) {
            addLog("获取比赛数据超时(5秒)", "warning");
            allPlayReply->abort();
        }
    });
    
    // 获取InquiryPage数据
    QUrl inquiryUrl(httpAddress + "/InquiryPage");
    QNetworkRequest inquiryRequest(inquiryUrl);
    inquiryRequest.setRawHeader("User-Agent", "DataUploadTool/1.0");
    inquiryRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    inquiryReply = networkManager->post(inquiryRequest, QByteArray());
    connect(inquiryReply, &QNetworkReply::finished, this, &MainWindow::onInquiryFinished);
    
    // 设置超时定时器
    QTimer::singleShot(localTimeout, this, [this]() {
        if (inquiryReply && inquiryReply->isRunning()) {
            addLog("获取比赛数据超时(5秒)", "warning");
            inquiryReply->abort();
        }
    });
}

void MainWindow::onRawInfoFinished()
{
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
    } else {
        addLog("获取赛程数据失败: " + rawInfoReply->errorString(), "error");
        rawInfoSuccess = false;
    }
    
    rawInfoReply->deleteLater();
    rawInfoReply = nullptr;
    
    pendingRequests--;
    if (pendingRequests == 0) {
        checkDataAndUpload();
    }
}

void MainWindow::onAllPlayFinished()
{
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
    if (pendingRequests == 0) {
        checkDataAndUpload();
    }
}

void MainWindow::onInquiryFinished()
{
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
        inquiryData = convertToUtf8(data, encoding);
        
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
    if (pendingRequests == 0) {
        checkDataAndUpload();
    }
}

void MainWindow::checkDataAndUpload()
{
    if (rawInfoSuccess && allPlaySuccess && inquirySuccess) {
        addLog("数据获取成功，准备上传", "success");
        uploadData();
    } else {
        addLog("数据获取失败，跳过本次上传", "warning");
        scheduleNextTask();
    }
}

void MainWindow::uploadData()
{
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
    
    // inquiry字段
    QHttpPart inquiryPart;
    inquiryPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                         QVariant("form-data; name=\"inquiry\""));
    inquiryPart.setBody(inquiryData.trimmed().toUtf8());
    multiPart->append(inquiryPart);
    
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
    QTimer::singleShot(60000, this, [this]() {
        if (uploadReply && uploadReply->isRunning()) {
            addLog("上传数据超时(60秒)", "warning");
            uploadReply->abort();
        }
    });
}

void MainWindow::onUploadFinished()
{
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

