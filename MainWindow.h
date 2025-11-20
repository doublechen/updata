#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QSplitter>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCryptographicHash>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringDecoder>
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onRawInfoFinished();
    void onAllPlayFinished();
    void onInquiryFinished();
    void onUploadFinished();
    void onOnePlayFinished();
    void onPlayrankFinished();
    void executeTask();

private:
    void setupUI();
    void setupStyles();
    void initializeLogFile();
    void addLog(const QString &message, const QString &logType = "info");
    void updateUIState(bool running);
    bool validateInputs();
    bool checkKey(const QString &key);
    QString detectEncoding(const QByteArray &data);
    QString convertToUtf8(const QByteArray &data, const QString &sourceEncoding);
    void fetchData();
    void uploadData();
    void checkDataAndUpload();
    void scheduleNextTask();
    void processAllPlayData();
    void fetchOnePlayData();
    void fetchPlayrankData();
    QString processInquiryHtml(const QString &html);

    // UI组件
    QWidget *centralWidget;
    QSplitter *splitter;
    
    // 左侧面板
    QWidget *leftPanel;
    QVBoxLayout *leftLayout;
    QWidget *inputSection;
    QGridLayout *inputLayout;
    
    QLabel *logoLabel;
    
    QLabel *labelHttpAddress;
    QLineEdit *txtHttpAddress;
    
    QLabel *labelInterval;
    QLineEdit *txtInterval;
    
    QLabel *labelApiKey;
    QLineEdit *txtApiKey;
    
    QPushButton *btnStart;
    QPushButton *btnStop;
    
    // 右侧面板
    QTextEdit *logArea;
    
    // 网络和定时器
    QNetworkAccessManager *networkManager;
    QTimer *taskTimer;
    
    // 状态变量
    bool isRunning;
    int loopCount;
    QString httpAddress;
    int intervalTime;
    QString apiKey;
    QString uploadUrl;
    
    // 数据缓存
    QString rawInfoData;
    QString allPlayData;
    QString inquiryData;
    QString playranksData;
    
    // 请求状态
    bool rawInfoSuccess;
    bool allPlaySuccess;
    bool inquirySuccess;
    bool playrankSuccess;
    bool onePlaySuccess; // onePlay请求完成标志
    bool isUploading; // 防止重复上传标志
    QNetworkReply *rawInfoReply;
    QNetworkReply *allPlayReply;
    QNetworkReply *inquiryReply;
    QNetworkReply *uploadReply;
    QList<QNetworkReply*> activePlayrankReplies; // 并行请求的playrank回复列表
    
    // 超时定时器
    QTimer *rawInfoTimer;
    QTimer *allPlayTimer;
    QTimer *inquiryTimer;
    QTimer *uploadTimer;
    
    int pendingRequests;
    
    // oneplay请求相关
    QJsonDocument allPlayJson;
    QList<QNetworkReply*> activeOnePlayReplies; // 并行请求的onePlay回复列表
    struct OnePlayRequest {
        int matchIndex;
        int playIndex;
        QString pid;
    };
    QList<OnePlayRequest> onePlayQueue; // 待处理的请求队列
    int totalOnePlayRequests; // 总请求数
    int completedOnePlayRequests; // 已完成的请求数
    
    // playrank请求相关
    QList<QString> playrankQueue; // 待处理的key队列
    QList<QString> playrankResults; // 收集的playrank响应数据
    int totalPlayrankRequests; // 总请求数
    int completedPlayrankRequests; // 已完成的请求数
    
    // 日志文件
    QFile *logFile;
    QTextStream *logStream;
    
    // 密钥验证常量
    static const QString SECRET_KEY;
    static const QString UPLOAD_URL;
    static const QString VERSION;
};

#endif // MAINWINDOW_H

