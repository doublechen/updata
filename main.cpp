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
#include <QFileInfoList>

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
    
    // 只在严重缺失 OpenSSL DLL 时显示错误
    // Qt 5.15.2 主要依赖 OpenSSL DLL，SSL 插件是可选的
    if (!sslSupported && (!cryptoExists || !sslExists)) {
        // 检查 Qt SSL 插件（用于诊断）
        QString sslPluginDir = appDir + "/plugins/ssl";
        QString tlsPluginDir = appDir + "/plugins/tls";
        bool sslPluginExists = QDir(sslPluginDir).exists();
        bool tlsPluginExists = QDir(tlsPluginDir).exists();
        
        QFileInfoList sslPlugins;
        if (sslPluginExists) {
            QDir sslDir(sslPluginDir);
            sslPlugins = sslDir.entryInfoList(QStringList() << "*.dll", QDir::Files);
        }
        if (tlsPluginExists) {
            QDir tlsDir(tlsPluginDir);
            sslPlugins.append(tlsDir.entryInfoList(QStringList() << "*.dll", QDir::Files));
        }
        
        QString errorMsg = QString("SSL/TLS 支持未启用！\n\n")
                          + "SSL 运行时版本: " + (sslVersion.isEmpty() ? "未知" : sslVersion) + "\n"
                          + "SSL 编译版本: " + (sslBuildVersion.isEmpty() ? "未知" : sslBuildVersion) + "\n\n"
                          + "OpenSSL DLL 状态:\n"
                          + "  libcrypto-1_1.dll: " + (cryptoExists ? "✓ 存在" : "✗ 不存在") + "\n"
                          + "  libssl-1_1.dll: " + (sslExists ? "✓ 存在" : "✗ 不存在") + "\n\n"
                          + "Qt SSL/TLS 插件状态:\n"
                          + "  plugins/ssl 目录: " + (sslPluginExists ? "✓ 存在" : "✗ 不存在") + "\n"
                          + "  plugins/tls 目录: " + (tlsPluginExists ? "✓ 存在" : "✗ 不存在") + "\n"
                          + "  插件文件: " + (sslPlugins.size() > 0 ? QString("✓ 找到 %1 个").arg(sslPlugins.size()) : "✗ 未找到") + "\n\n"
                          + "应用程序目录: " + appDir + "\n\n"
                          + "⚠ 关键问题: OpenSSL DLL 文件缺失！\n\n"
                          + "修复方法:\n"
                          + "1. 运行 copy_openssl_dll.bat 脚本进行自动修复\n"
                          + "2. 或参考 OPENSSL_MANUAL_FIX.md 进行手动修复\n"
                          + "3. 从 https://slproweb.com/products/Win32OpenSSL.html 下载 OpenSSL 1.1.1\n\n"
                          + "注意: Qt 5.15.2 主要依赖 OpenSSL DLL 文件，\n"
                          + "SSL 插件 (qopensslbackend.dll) 在某些版本中是可选的。";
        
        QMessageBox::critical(nullptr, "SSL/TLS 错误", errorMsg);
    } else if (!sslSupported) {
        // OpenSSL DLL 存在但 SSL 仍未启用 - 这可能是插件问题，但给出更温和的警告
        qDebug() << "注意: QSslSocket::supportsSsl() 返回 false，但 OpenSSL DLL 文件存在";
        qDebug() << "这可能是因为缺少 Qt SSL 插件，但对于 Qt 5.15.2，这可能不是问题";
        qDebug() << "如果 HTTPS 请求失败，请检查日志并参考 OPENSSL_MANUAL_FIX.md";
        
        // 不显示对话框，只在日志中记录
        // 因为有些 Qt 版本即使 supportsSsl() 返回 false，HTTPS 仍然可以工作
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

