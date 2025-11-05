#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include <QSslSocket>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("数据上传工具");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DataUploadTool");
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5: 设置UTF-8编码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
    // Qt6: 默认使用UTF-8,无需设置
#endif
    
    // 检查 SSL/TLS 支持
    bool sslSupported = QSslSocket::supportsSsl();
    QString sslVersion = QSslSocket::sslLibraryVersionString();
    QString sslBuildVersion = QSslSocket::sslLibraryBuildVersionString();
    
    qDebug() << "SSL 支持:" << sslSupported;
    qDebug() << "SSL 运行时版本:" << sslVersion;
    qDebug() << "SSL 编译版本:" << sslBuildVersion;
    
    // 检查 OpenSSL DLL 文件是否存在
    QString appDir = QApplication::applicationDirPath();
    QString cryptoDll = appDir + "/libcrypto-1_1.dll";
    QString sslDll = appDir + "/libssl-1_1.dll";
    
    bool cryptoExists = QFile::exists(cryptoDll);
    bool sslExists = QFile::exists(sslDll);
    
    qDebug() << "OpenSSL DLL 检查:";
    qDebug() << "  libcrypto-1_1.dll:" << (cryptoExists ? "存在" : "不存在") << "(" << cryptoDll << ")";
    qDebug() << "  libssl-1_1.dll:" << (sslExists ? "存在" : "不存在") << "(" << sslDll << ")";
    
    if (!sslSupported) {
        QString errorMsg = QString("SSL/TLS 支持未启用！\n\n")
                          + "SSL 运行时版本: " + (sslVersion.isEmpty() ? "未知" : sslVersion) + "\n"
                          + "SSL 编译版本: " + (sslBuildVersion.isEmpty() ? "未知" : sslBuildVersion) + "\n\n"
                          + "OpenSSL DLL 状态:\n"
                          + "  libcrypto-1_1.dll: " + (cryptoExists ? "✓ 存在" : "✗ 不存在") + "\n"
                          + "  libssl-1_1.dll: " + (sslExists ? "✓ 存在" : "✗ 不存在") + "\n\n"
                          + "应用程序目录: " + appDir + "\n\n"
                          + "HTTPS 请求将无法正常工作。";
        
        QMessageBox::critical(nullptr, "SSL/TLS 错误", errorMsg);
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

