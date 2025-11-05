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
#include <QJsonParseError>
#include <QFile>
#include <QTextStream>
#include <QDir>
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
    void executeTask();

private:
    void setupUI();
    void setupStyles();
    void initializeLogFile();
    void addLog(const QString &message, const QString &logType = "info");
    void updateUIState(bool running);
    bool validateInputs();
    QString detectEncoding(const QByteArray &data);
    QString convertToUtf8(const QByteArray &data, const QString &sourceEncoding);
    void fetchData();
    void uploadData();
    void checkDataAndUpload();
    void scheduleNextTask();

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
    
    // 请求状态
    bool rawInfoSuccess;
    bool allPlaySuccess;
    bool inquirySuccess;
    QNetworkReply *rawInfoReply;
    QNetworkReply *allPlayReply;
    QNetworkReply *inquiryReply;
    QNetworkReply *uploadReply;
    
    int pendingRequests;
    
    // 日志文件
    QFile *logFile;
    QTextStream *logStream;
};

#endif // MAINWINDOW_H

