#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

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
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

