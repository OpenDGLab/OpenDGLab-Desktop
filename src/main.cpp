#include "mainWindow.h"
#include "global.h"
#include <QApplication>
void crashingMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg(msg.toLocal8Bit());
    switch (type) {
        case QtDebugMsg:
           fprintf(stderr, "Debug: %s\n", localMsg.constData());
           break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtWarningMsg:
            if(!Global::skipWarning) fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            if(!Global::skipError) abort(); //Index out of range is just a Warning!
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            if(!Global::skipError) abort();
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            if(!Global::skipError) abort();
            break;
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(crashingMessageHandler);
    Global::initGlobal();
    MainWindow w;
    w.show();
    return a.exec();
}
