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
    
    if (!sslSupported) {
        // 检查 Qt SSL 插件
        QString sslPluginDir = appDir + "/plugins/ssl";
        bool sslPluginExists = QDir(sslPluginDir).exists();
        QFileInfoList sslPlugins;
        if (sslPluginExists) {
            QDir sslDir(sslPluginDir);
            sslPlugins = sslDir.entryInfoList(QStringList() << "*.dll", QDir::Files);
        }
        
        // 根据文件存在情况决定错误级别
        bool hasDlls = cryptoExists && sslExists;
        bool hasPlugins = sslPlugins.size() > 0;
        
        QString errorMsg = QString("SSL/TLS 支持未启用！\n\n")
                          + "SSL 运行时版本: " + (sslVersion.isEmpty() ? "未知" : sslVersion) + "\n"
                          + "SSL 编译版本: " + (sslBuildVersion.isEmpty() ? "未知" : sslBuildVersion) + "\n\n"
                          + "OpenSSL DLL 状态:\n"
                          + "  libcrypto-1_1.dll: " + (cryptoExists ? "✓ 存在" : "✗ 不存在") + "\n"
                          + "  libssl-1_1.dll: " + (sslExists ? "✓ 存在" : "✗ 不存在") + "\n\n"
                          + "Qt SSL 插件状态:\n"
                          + "  插件目录: " + (sslPluginExists ? "✓ 存在" : "✗ 不存在") + " (" + sslPluginDir + ")\n"
                          + "  插件文件: " + (sslPlugins.size() > 0 ? QString("✓ 找到 %1 个插件").arg(sslPlugins.size()) : "✗ 未找到") + "\n\n"
                          + "应用程序目录: " + appDir + "\n\n"
                          + "修复方法:\n"
                          + "1. 确保 OpenSSL DLL 文件存在于应用程序目录\n"
                          + "2. 确保 Qt SSL 插件存在于 plugins\\ssl 目录\n"
                          + "3. 运行 copy_openssl_dll.bat 脚本进行自动修复\n"
                          + "4. 或参考 OPENSSL_MANUAL_FIX.md 进行手动修复\n\n";
        
        if (!hasDlls) {
            errorMsg += "⚠ 警告: OpenSSL DLL 文件缺失，HTTPS 请求将无法正常工作！";
            QMessageBox::critical(nullptr, "SSL/TLS 错误", errorMsg);
        } else if (!hasPlugins) {
            errorMsg += "⚠ 警告: Qt SSL 插件缺失，HTTPS 功能可能受限。\n"
                       + "但 OpenSSL DLL 文件已存在，某些功能可能仍然可用。\n"
                       + "建议修复以获取完整的 SSL/TLS 支持。";
            QMessageBox::warning(nullptr, "SSL/TLS 警告", errorMsg);
        } else {
            errorMsg += "⚠ 警告: SSL/TLS 支持未完全启用，但必要文件似乎存在。\n"
                       + "如果 HTTPS 请求失败，请检查上述文件是否正确。";
            QMessageBox::warning(nullptr, "SSL/TLS 警告", errorMsg);
        }
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

