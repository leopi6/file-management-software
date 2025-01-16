#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_packageButton_clicked();
    void on_unpackButton_clicked();
    void on_compressButton_clicked();
    void on_decompressButton_clicked();
    void on_backupButton_clicked();
    void on_restoreButton_clicked();
    void on_startScheduledBackupButton_clicked();
    void on_stopScheduledBackupButton_clicked();

    void on_encryptButton_clicked();
    void on_decryptButton_clicked();
    void on_startMonitoringButton_clicked();
    void on_stopMonitoringButton_clicked();
    void on_fileChanged(const QString &path);

    void startScheduledBackup(int interval);      // 启动定时备份定时器
    void performRealTimeBackup(const QString &path);
    void performScheduledBackup();  // <-- 确保这里声明了 performScheduledBackup
    void handleFileDeletion(const QString &path);

private:

    void unpackFiles(const QString &packageDirPath, const QString &outputDirPath);
    void re(const QString &backupDir, const QString &jsonFileName, const QString &restoreDir);
    void ba(const QString &sourceDir, const QString &backupDir, const QStringList &filters);

    QByteArray encryptData(const QByteArray &data, const QString &password);
    QByteArray decryptData(const QByteArray &data, const QString &password);


    bool compressFiles(const QStringList &sourceFiles, const QString &outputFile);
    bool decompressFile(const QString &inputFile, const QString &outputDir);
    bool packageFiles(const QStringList &filePaths, const QString &targetDir);

    Ui::Widget *ui;
    QTimer *scheduledBackupTimer;
    QFileSystemWatcher *fileWatcher;

    // 存储源目录和备份目录路径
    QString backupSourceDir;
    QString backupDestinationDir;

    // 用于存储监视的多个文件路径
    QStringList monitoredFiles;

    // 用于在 UI 中显示监视文件的列表
    QListWidget *fileListWidget;





    QStringList scheduledBackupDirs;  // 存储正在定时备份的目录列表
    // 备份目标目录
};

#endif // WIDGET_H
