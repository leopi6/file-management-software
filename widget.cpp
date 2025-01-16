#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>

#include <QRegularExpression>

#include <QCoreApplication>
#include <QProcess>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDirIterator>

#include <QFileInfo>


#include <QFileSystemWatcher>






struct FileMetadata {
    QString filePath;
    QString fileName;
    QDateTime creationTime;
    QDateTime lastModifiedTime;
    QByteArray fileData;  // 文件的二进制数据
};



// 构造函数：初始化UI，定时器和文件系统监视器
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , scheduledBackupTimer(new QTimer(this))
    , fileWatcher(new QFileSystemWatcher(this))
    , backupSourceDir("")
    , backupDestinationDir("")
{
    ui->setupUi(this);

     fileWatcher = new QFileSystemWatcher(this);
    // 连接定时器的超时信号到执行备份的槽函数
    connect(scheduledBackupTimer, &QTimer::timeout, this, &Widget::performScheduledBackup);



    // 连接文件系统监视器的文件变化信号到相应槽函数
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &Widget::on_fileChanged);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &Widget::on_fileChanged);
}

// 析构函数：清理UI
Widget::~Widget()
{
    delete ui;
}

// 解包文件：将包目录下的所有文件解包到目标目录，保持目录结构和元数据
void Widget::unpackFiles(const QString &packageDirPath, const QString &outputDirPath)
{
    QDir packageDir(packageDirPath);
    if (!packageDir.exists()) {
        QMessageBox::warning(this, tr("解包失败"), tr("包目录不存在。"));
        return;
    }

    QDir outputDir(outputDirPath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            QMessageBox::warning(this, tr("解包失败"), tr("无法创建输出目录。"));
            return;
        }
    }

    // 遍历包目录下的所有文件和子目录
    QFileInfoList entryList = packageDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);
    foreach (const QFileInfo &entry, entryList) {
        QString relativePath = packageDir.relativeFilePath(entry.absoluteFilePath());
        QString destPath = outputDir.filePath(relativePath);

        if (entry.isDir()) {
            // 创建目标子目录
            QDir().mkpath(destPath);
        } else if (entry.isFile()) {
            // 复制文件
            if (!QFile::copy(entry.absoluteFilePath(), destPath)) {
                QMessageBox::warning(this, tr("解包失败"), tr("无法复制文件 %1 到 %2。").arg(entry.absoluteFilePath(), destPath));
                continue;
            }

            // 复制文件元数据（权限）
            QFile::setPermissions(destPath, entry.permissions());
            QFileInfo destInfo(destPath);
            // Qt 不直接支持设置文件的修改时间，这里简化处理
        }
        // 处理符号链接、管道等特殊文件类型（根据需要）
    }
}






// 打包函数实现
bool Widget::packageFiles(const QStringList &filePaths, const QString &targetDir)
{
    if (filePaths.isEmpty()) {
        qDebug() << "没有选择任何文件进行打包。";
        return false;
    }

    QDir targetDirectory(targetDir);

    // 获取所有文件名（不包含路径）
    QStringList fileNames;
    for (const QString &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        fileNames << fileInfo.fileName();
    }

    bool allSuccess = true;

    for (int i = 0; i < filePaths.size(); ++i) {
        const QString &sourceFilePath = filePaths.at(i);
        const QString &fileName = fileNames.at(i);
        QString targetFilePath = targetDirectory.filePath(fileName);

        QFileInfo fileInfo(sourceFilePath);
        if (fileInfo.isFile()) {
            // 复制文件
            if (!QFile::copy(sourceFilePath, targetFilePath)) {
                // 如果文件已存在，尝试覆盖
                if (QFile::exists(targetFilePath)) {
                    if (!QFile::remove(targetFilePath)) {
                        qDebug() << "无法删除已存在的文件:" << targetFilePath;
                        allSuccess = false;
                        continue;
                    }
                    if (!QFile::copy(sourceFilePath, targetFilePath)) {
                        qDebug() << "无法复制文件:" << sourceFilePath << "到" << targetFilePath;
                        allSuccess = false;
                        continue;
                    }
                } else {
                    qDebug() << "无法复制文件:" << sourceFilePath << "到" << targetFilePath;
                    allSuccess = false;
                    continue;
                }
            }
            qDebug() << "已复制文件:" << sourceFilePath << "到" << targetFilePath;
        }
    }

    return allSuccess;
}

// 当点击“打包”按钮时调用
void Widget::on_packageButton_clicked()
{
    // 选择要打包的文件
    QStringList selectedFiles = QFileDialog::getOpenFileNames(
        this,
        tr("选择要打包的文件"),
        QDir::homePath(),
        tr("所有文件 (*.*)")
        );

    if (selectedFiles.isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未选择任何文件，操作已取消。"));
        return;
    }

    // 输入新建文件夹名称
    bool ok;
    QString newFolderName = QInputDialog::getText(
        this,
        tr("输入文件夹名称"),
        tr("请输入新建文件夹的名称:"),
        QLineEdit::Normal,
        "NewPackage",
        &ok
        );

    if (!ok || newFolderName.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未输入文件夹名称，操作已取消。"));
        return;
    }

    // 选择目标包存放目录
    QString packageDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择包存放目录"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (packageDir.isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未选择目标包存放目录，操作已取消。"));
        return;
    }

    // 构建新文件夹的完整路径
    QDir targetDir(packageDir);
    QString newFolderPath = targetDir.filePath(newFolderName);

    // 检查新文件夹是否已存在
    if (targetDir.exists(newFolderName)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            tr("文件夹已存在"),
            tr("文件夹 \"%1\" 已存在。是否要覆盖？").arg(newFolderName),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::No) {
            QMessageBox::information(this, tr("操作取消"), tr("操作已取消。"));
            return;
        }

        // 尝试删除已存在的文件夹
        if (!targetDir.cd(newFolderName) || !targetDir.removeRecursively()) {
            QMessageBox::warning(this, tr("删除失败"), tr("无法删除已存在的文件夹: %1").arg(newFolderPath));
            return;
        }
    }

    // 创建新文件夹
    if (!targetDir.mkdir(newFolderName)) {
        QMessageBox::warning(this, tr("创建文件夹失败"), tr("无法创建文件夹: %1").arg(newFolderPath));
        return;
    }

    // 调用打包函数
    bool success = packageFiles(selectedFiles, newFolderPath);

    if (success) {
        QMessageBox::information(this, tr("打包完成"), tr("文件已成功打包到 %1。").arg(newFolderPath));
    } else {
        QMessageBox::warning(this, tr("打包部分完成"), tr("部分文件在打包过程中出现错误。"));
    }
}







// 当点击“解包”按钮时调用
void Widget::on_unpackButton_clicked()
{
    // 选择包目录
    QString packageDir = QFileDialog::getExistingDirectory(this, tr("选择包目录"));
    if (packageDir.isEmpty()) return;

    // 选择输出目录
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("选择解包输出目录"));
    if (outputDir.isEmpty()) return;

    // 调用解包函数
    unpackFiles(packageDir, outputDir);

    QMessageBox::information(this, tr("解包完成"), tr("文件已成功解包。"));
}




// 压缩函数
void Widget::on_compressButton_clicked()
{
    // 选择要压缩的文件
    QStringList sourceFiles = QFileDialog::getOpenFileNames(this, tr("选择要压缩的文件"));
    if (sourceFiles.isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未选择任何文件进行压缩。"));
        return;
    }

    // 选择压缩后的输出包文件
    QString packageFilePath = QFileDialog::getSaveFileName(this, tr("选择压缩包文件位置"), "", "Compressed Package (*.qzpkg)");
    if (packageFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未选择压缩包文件位置，操作已取消。"));
        return;
    }

    // 输入包内新建文件夹的名称
    bool ok;
    QString newFolderName = QInputDialog::getText(this, tr("输入文件夹名称"),
                                                  tr("请输入包内新建文件夹的名称:"), QLineEdit::Normal,
                                                  "NewPackage", &ok);
    if (!ok || newFolderName.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("操作取消"), tr("未输入文件夹名称，操作已取消。"));
        return;
    }

    // 创建容器数据
    QByteArray containerData;
    QDataStream outStream(&containerData, QIODevice::WriteOnly);
    outStream.setByteOrder(QDataStream::LittleEndian);

    // （可选）添加魔数和版本号
    // const char magicNumber[4] = { 'Q', 'Z', 'P', 'K' };
    // outStream.writeRawData(magicNumber, 4);
    // quint16 version = 1;
    // outStream << version;

    // 写入新建文件夹名称的大小和内容
    QByteArray folderNameBytes = newFolderName.toUtf8();
    quint32 folderNameSize = static_cast<quint32>(folderNameBytes.size());
    outStream << folderNameSize;
    outStream.writeRawData(folderNameBytes.data(), folderNameSize);
    if (outStream.status() != QDataStream::Ok) {
        QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件夹名称。"));
        return;
    }

    // 暂存成功压缩的文件数据
    QByteArray filesData;
    QDataStream filesStream(&filesData, QIODevice::WriteOnly);
    filesStream.setByteOrder(QDataStream::LittleEndian);

    quint32 successfulFileCount = 0;

    // 遍历每个文件，写入文件名大小、文件名、文件数据大小和文件数据
    for (const QString &filePath : sourceFiles) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        QByteArray fileNameBytes = fileName.toUtf8();
        quint32 fileNameSize = static_cast<quint32>(fileNameBytes.size());

        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("压缩警告"), tr("无法打开文件: %1").arg(filePath));
            qDebug() << "无法打开文件:" << filePath;
            continue; // 跳过当前文件
        }

        QByteArray fileData = inputFile.readAll();
        inputFile.close();
        quint32 fileDataSize = static_cast<quint32>(fileData.size());

        // 写入文件名大小和文件名
        filesStream << fileNameSize;
        if (filesStream.status() != QDataStream::Ok) {
            QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件名大小。"));
            return;
        }
        filesStream.writeRawData(fileNameBytes.data(), fileNameSize);
        if (filesStream.status() != QDataStream::Ok) {
            QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件名。"));
            return;
        }

        // 写入文件数据大小和文件数据
        filesStream << fileDataSize;
        if (filesStream.status() != QDataStream::Ok) {
            QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件数据大小。"));
            return;
        }
        filesStream.writeRawData(fileData.data(), fileDataSize);
        if (filesStream.status() != QDataStream::Ok) {
            QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件数据。"));
            return;
        }

        successfulFileCount++;
        qDebug() << "已添加文件:" << fileName << "大小:" << fileDataSize;
    }

    if (successfulFileCount == 0) {
        QMessageBox::warning(this, tr("压缩失败"), tr("没有文件被成功压缩。"));
        return;
    }

    // 写入成功压缩的文件数量
    outStream << successfulFileCount;
    if (outStream.status() != QDataStream::Ok) {
        QMessageBox::warning(this, tr("压缩失败"), tr("无法写入文件数量。"));
        return;
    }

    // 将文件数据追加到容器数据中
    containerData.append(filesData);

    // 压缩容器数据
    QByteArray compressedData = qCompress(containerData, 9); // 压缩级别 9
    qDebug() << "原始数据大小:" << containerData.size() << "压缩后大小:" << compressedData.size();

    // 写入压缩后的数据到包文件
    QFile outputFile(packageFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("压缩失败"), tr("无法创建包文件: %1").arg(packageFilePath));
        return;
    }

    qint64 bytesWritten = outputFile.write(compressedData);
    outputFile.close();

    if (bytesWritten != compressedData.size()) {
        QMessageBox::warning(this, tr("压缩失败"), tr("写入包文件时发生错误。"));
        return;
    }

    QMessageBox::information(this, tr("压缩完成"), tr("文件已成功压缩到 %1。").arg(packageFilePath));
}

// 解压函数
void Widget::on_decompressButton_clicked()
{
    // 选择要解压的压缩包
    QString inputFilePath = QFileDialog::getOpenFileName(this, tr("选择要解压的压缩包"), "", "Compressed Package (*.qzpkg)");
    if (inputFilePath.isEmpty()) return;

    // 选择解压后的输出目录
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("选择解压后的文件夹"));
    if (outputDir.isEmpty()) return;

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("解压失败"), tr("无法打开文件: %1").arg(inputFilePath));
        return;
    }

    QByteArray compressedData = inputFile.readAll();
    inputFile.close();

    QByteArray containerData = qUncompress(compressedData);
    if (containerData.isNull()) {
        QMessageBox::warning(this, tr("解压失败"), tr("无法解压文件: %1").arg(inputFilePath));
        return;
    }

    QDataStream inStream(containerData);
    inStream.setByteOrder(QDataStream::LittleEndian);

    // 读取新建文件夹名称的大小和内容
    quint32 folderNameSize;
    inStream >> folderNameSize;
    if (inStream.status() != QDataStream::Ok) {
        QMessageBox::warning(this, tr("解压失败"), tr("无法读取文件夹名称大小。"));
        return;
    }

    QByteArray folderNameBytes(folderNameSize, 0);
    if (inStream.readRawData(folderNameBytes.data(), folderNameSize) != folderNameSize) {
        QMessageBox::warning(this, tr("解压失败"), tr("无法读取文件夹名称。"));
        return;
    }
    QString folderName = QString::fromUtf8(folderNameBytes);

    // 创建新建文件夹在输出目录
    QDir outputDirectory(outputDir);
    QString newFolderPath = outputDirectory.filePath(folderName);
    if (!outputDirectory.exists(folderName)) {
        if (!outputDirectory.mkdir(folderName)) {
            QMessageBox::warning(this, tr("解压失败"), tr("无法创建文件夹: %1").arg(newFolderPath));
            return;
        }
    }

    // 读取文件数量
    quint32 fileCount;
    inStream >> fileCount;
    if (inStream.status() != QDataStream::Ok) {
        QMessageBox::warning(this, tr("解压失败"), tr("无法读取文件数量。"));
        return;
    }

    bool allSuccess = true;

    for (quint32 i = 0; i < fileCount; ++i) {
        // 读取文件名大小和文件名
        quint32 fileNameSize;
        inStream >> fileNameSize;
        if (inStream.status() != QDataStream::Ok) {
            qDebug() << "读取文件名大小失败，索引:" << i;
            allSuccess = false;
            break;
        }

        QByteArray fileNameBytes(fileNameSize, 0);
        if (inStream.readRawData(fileNameBytes.data(), fileNameSize) != fileNameSize) {
            qDebug() << "读取文件名失败，索引:" << i;
            allSuccess = false;
            break;
        }
        QString fileName = QString::fromUtf8(fileNameBytes);

        // 读取文件数据大小和文件数据
        quint32 fileDataSize;
        inStream >> fileDataSize;
        if (inStream.status() != QDataStream::Ok) {
            qDebug() << "读取文件数据大小失败，文件名:" << fileName;
            allSuccess = false;
            break;
        }

        QByteArray fileData(fileDataSize, 0);
        if (inStream.readRawData(fileData.data(), fileDataSize) != fileDataSize) {
            qDebug() << "读取文件数据失败，文件名:" << fileName;
            allSuccess = false;
            break;
        }

        // 构建输出文件路径
        QString outputFilePath = QDir(newFolderPath).filePath(fileName);
        QFile outputFile(outputFilePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("解压失败"), tr("无法写入文件: %1").arg(outputFilePath));
            allSuccess = false;
            continue;
        }

        // 写入文件数据
        outputFile.write(fileData);
        outputFile.close();

        qDebug() << "已解压:" << fileName << "到" << outputFilePath;
    }

    if (allSuccess) {
        QMessageBox::information(this, tr("解压完成"), tr("文件已成功解压到 %1。").arg(newFolderPath));
    } else {
        QMessageBox::warning(this, tr("解压完成"), tr("文件部分解压完成，但存在错误。"));
    }
}





// 备份函数：将文件打包成 JSON 文件
void Widget::ba(const QString &sourceDir, const QString &backupDir, const QStringList &filters)
{
    QJsonArray filesMetadata;  // 保存文件元数据的数组

    QDirIterator dirIterator(sourceDir, filters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        dirIterator.next();

        QFileInfo fileInfo(dirIterator.filePath());
        QFile file(dirIterator.filePath());

        if (file.open(QIODevice::ReadOnly)) {
            // 读取文件数据
            QByteArray fileData = file.readAll();
            file.close();

            // 获取文件的元数据
            FileMetadata metadata;
            metadata.filePath = dirIterator.filePath();
            metadata.fileName = fileInfo.fileName();
            metadata.creationTime = fileInfo.birthTime();
            metadata.lastModifiedTime = fileInfo.lastModified();
            metadata.fileData = fileData;

            // 将文件元数据存储到 JSON 数组中
            QJsonObject fileObj;
            fileObj["filePath"] = metadata.filePath;
            fileObj["fileName"] = metadata.fileName;
            fileObj["creationTime"] = metadata.creationTime.toString(Qt::ISODate);
            fileObj["lastModifiedTime"] = metadata.lastModifiedTime.toString(Qt::ISODate);
            fileObj["fileData"] = QString::fromUtf8(fileData.toBase64());

            filesMetadata.append(fileObj);
        }
    }

    // 将所有文件元数据保存为 JSON 文件
    QFile backupMetadataFile(backupDir + "/backup_metadata.json");
    if (backupMetadataFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(filesMetadata);
        backupMetadataFile.write(doc.toJson());
        backupMetadataFile.close();
    }
}

// 当点击“备份”按钮时调用
void Widget::on_backupButton_clicked()
{
    // 选择要备份的源目录
    QString sourceDir = QFileDialog::getExistingDirectory(this, tr("选择要备份的源目录"));
    if (sourceDir.isEmpty()) return;

    // 选择备份目标目录
    QString backupDir = QFileDialog::getExistingDirectory(this, tr("选择备份目标目录"));
    if (backupDir.isEmpty()) return;

    // 保存选择的目录到成员变量
    backupSourceDir = sourceDir;
    backupDestinationDir = backupDir;

    // 自定义筛选条件（示例：备份所有文件）
    QStringList filters; // 空表示不筛选，备份所有文件

    // 调用打包函数进行备份
    ba(sourceDir, backupDir, filters);

    QMessageBox::information(this, tr("备份完成"), tr("数据已成功备份。"));

    // 更新文件监视器
    if (!fileWatcher->directories().contains(backupSourceDir)) {
        fileWatcher->addPath(backupSourceDir);
    }
}

// huanyuan函数：
void Widget::re(const QString &backupDir, const QString &jsonFileName, const QString &restoreDir)
{
    QFile backupMetadataFile(backupDir + "/" + jsonFileName);

    if (!backupMetadataFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("还原失败"), tr("无法读取备份的元数据文件。"));
        return;
    }

    QByteArray metadataContent = backupMetadataFile.readAll();
    backupMetadataFile.close();

    // 解析 JSON 数据
    QJsonDocument doc = QJsonDocument::fromJson(metadataContent);
    QJsonArray filesMetadata = doc.array();

    // 遍历所有文件的元数据
    for (const QJsonValue &value : filesMetadata) {
        QJsonObject fileObj = value.toObject();

        QString originalFilePath = fileObj["filePath"].toString();
        QString fileName = fileObj["fileName"].toString();
        QDateTime creationTime = QDateTime::fromString(fileObj["creationTime"].toString(), Qt::ISODate);
        QDateTime lastModifiedTime = QDateTime::fromString(fileObj["lastModifiedTime"].toString(), Qt::ISODate);
        QByteArray fileData = QByteArray::fromBase64(fileObj["fileData"].toString().toUtf8());

        // 确定目标文件路径
        QString targetFilePath;
        if (restoreDir == QFileInfo(originalFilePath).absolutePath()) {
            // 如果选择还原到原位置，在文件名后添加“换原版”
            QFileInfo fi(originalFilePath);
            QString newFileName = fi.fileName() + "换原版";
            QString newFilePath = fi.absolutePath() + "/" + newFileName;
            targetFilePath = newFilePath;
        } else {
            // 还原到指定目录，保持原文件名
            targetFilePath = restoreDir + "/" + fileName;
        }

        // 创建目标路径的文件夹并写入数据
        QFile file(targetFilePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData);
            file.close();

            // 恢复文件的时间属性
            QFile restoredFile(targetFilePath);  // 创建目标文件的 QFile 对象
            restoredFile.setFileTime(creationTime, QFile::FileTime::FileBirthTime);  // 设置创建时间
            restoredFile.setFileTime(lastModifiedTime, QFile::FileTime::FileModificationTime);  // 设置修改时间

            qDebug() << "已还原文件:" << targetFilePath;
        } else {
            QMessageBox::warning(this, tr("还原失败"), tr("无法写入文件: %1").arg(targetFilePath));
        }
    }

    QMessageBox::information(this, tr("还原完成"), tr("文件已成功还原。"));
}

// 当点击“还原”按钮时调用
void Widget::on_restoreButton_clicked()
{
    // 选择备份文件（只显示 .json 文件）
    QString backupFile = QFileDialog::getOpenFileName(this, tr("选择备份文件"), "", tr("JSON 文件 (*.json)"));
    if (backupFile.isEmpty()) return;  // 用户取消选择文件

    // 读取备份元数据文件
    QFile backupMetadataFile(backupFile);
    if (!backupMetadataFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("还原失败"), tr("无法读取备份的元数据文件。"));
        return;
    }

    QByteArray metadataContent = backupMetadataFile.readAll();
    backupMetadataFile.close();

    // 解析 JSON 数据
    QJsonDocument doc = QJsonDocument::fromJson(metadataContent);
    QJsonArray filesMetadata = doc.array();

    // 提示用户选择还原方式
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("是否还原到原位置"),
                                  tr("请选择还原位置：\n是: 还原到原来的位置\n否: 选择新的文件夹"),
                                  QMessageBox::Yes | QMessageBox::No);

    QString restoreDir;

    // 如果用户选择“还原到原来的位置”
    if (reply == QMessageBox::Yes) {
        // 遍历所有文件的元数据
        for (const QJsonValue &value : filesMetadata) {
            QJsonObject fileObj = value.toObject();

            QString originalFilePath = fileObj["filePath"].toString();
            QString fileName = fileObj["fileName"].toString();
            QDateTime creationTime = QDateTime::fromString(fileObj["creationTime"].toString(), Qt::ISODate);
            QDateTime lastModifiedTime = QDateTime::fromString(fileObj["lastModifiedTime"].toString(), Qt::ISODate);
            QByteArray fileData = QByteArray::fromBase64(fileObj["fileData"].toString().toUtf8());

            // 修改文件名，添加“换原版”
            QFileInfo fi(originalFilePath);
            QString newFileName = fi.fileName() + "换原版";  // 在文件名后添加“换原版”
            QString newFilePath = fi.absolutePath() + "/" + newFileName;

            // 创建目标路径的文件夹并写入数据
            QFile file(newFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(fileData);
                file.close();

                // 恢复文件的时间属性
                QFile restoredFile(newFilePath);  // 创建目标文件的 QFile 对象
                restoredFile.setFileTime(creationTime, QFile::FileTime::FileBirthTime);  // 设置创建时间
                restoredFile.setFileTime(lastModifiedTime, QFile::FileTime::FileModificationTime);  // 设置修改时间

                qDebug() << "已还原文件:" << newFilePath;
            } else {
                QMessageBox::warning(this, tr("还原失败"), tr("无法写入文件: %1").arg(newFilePath));
            }
        }
    } else {
        // 用户选择“选择新的文件夹”
        restoreDir = QFileDialog::getExistingDirectory(this, tr("选择还原目标目录"));
        if (restoreDir.isEmpty()) return;  // 用户取消选择目录

        // 遍历所有文件的元数据并执行还原到新位置
        for (const QJsonValue &value : filesMetadata) {
            QJsonObject fileObj = value.toObject();

            QString fileName = fileObj["fileName"].toString();
            QDateTime creationTime = QDateTime::fromString(fileObj["creationTime"].toString(), Qt::ISODate);
            QDateTime lastModifiedTime = QDateTime::fromString(fileObj["lastModifiedTime"].toString(), Qt::ISODate);
            QByteArray fileData = QByteArray::fromBase64(fileObj["fileData"].toString().toUtf8());

            // 构建目标文件路径
            QString targetFilePath = restoreDir + "/" + fileName;

            // 创建目标路径的文件夹并写入数据
            QFile file(targetFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(fileData);
                file.close();

                // 恢复文件的时间属性
                QFile restoredFile(targetFilePath);  // 创建目标文件的 QFile 对象
                restoredFile.setFileTime(creationTime, QFile::FileTime::FileBirthTime);  // 设置创建时间
                restoredFile.setFileTime(lastModifiedTime, QFile::FileTime::FileModificationTime);  // 设置修改时间

                qDebug() << "已还原文件:" << targetFilePath;
            } else {
                QMessageBox::warning(this, tr("还原失败"), tr("无法写入文件: %1").arg(targetFilePath));
            }
        }
    }

    QMessageBox::information(this, tr("还原完成"), tr("文件已成功还原。"));
}





// 停止定时备份
void Widget::on_stopScheduledBackupButton_clicked()
{
    // 如果没有备份的目录，弹出警告
    if (scheduledBackupDirs.isEmpty()) {
        QMessageBox::warning(this, tr("定时备份"), tr("没有正在备份的目录。"));
        return;
    }

    // 获取用户在 QListWidget 中选择的目录
    QList<QListWidgetItem *> selectedItems = ui->scheduledBackupListWidget->selectedItems();

    // 如果没有选择任何目录，则返回
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("定时备份"), tr("请先选择要停止备份的目录。"));
        return;
    }

    // 移除选中的目录
    foreach (QListWidgetItem *item, selectedItems) {
        QString dirPath = item->text();

        // 从定时备份目录列表中移除该目录
        scheduledBackupDirs.removeAll(dirPath);

        // 从文件列表中删除该项
        delete item;
    }

    // 停止定时器
    scheduledBackupTimer->stop();
    QMessageBox::information(this, tr("定时备份"), tr("选择的定时备份已停止。"));
}


// 当点击“开始定时备份”按钮时调用
void Widget::on_startScheduledBackupButton_clicked()
{
    // 弹出选择多个源文件目录对话框
    QString sourceDir = QFileDialog::getExistingDirectory(this, tr("选择源文件目录"));
    if (sourceDir.isEmpty()) {
        QMessageBox::warning(this, tr("定时备份"), tr("未选择源文件目录"));
        return;
    }

    // 弹出选择目标文件目录对话框
    QString destinationDir = QFileDialog::getExistingDirectory(this, tr("选择目标文件目录"));
    if (destinationDir.isEmpty()) {
        QMessageBox::warning(this, tr("定时备份"), tr("未选择目标文件目录"));
        return;
    }

    // 获取备份间隔（例如，以分钟为单位）
    bool ok;
    int interval = QInputDialog::getInt(this, tr("设置备份间隔"), tr("请输入备份间隔（分钟）:"), 60, 1, 10080, 1, &ok);
    if (!ok) return;

    backupDestinationDir = destinationDir;

    // 将源目录添加到定时备份目录列表
    scheduledBackupDirs.append(sourceDir);

    // 在 QListWidget 中显示这些目录
    QListWidgetItem *item = new QListWidgetItem(sourceDir);
    ui->scheduledBackupListWidget->addItem(item);  // 将源目录添加到文件列表中

    // 启动定时器
    startScheduledBackup(interval);

    // 提示用户定时备份已启动
    QMessageBox::information(this, tr("定时备份"), tr("定时备份已启动，每 %1 分钟执行一次。").arg(interval));
}

// 执行定时备份
void Widget::performScheduledBackup()
{
    if (scheduledBackupDirs.isEmpty() || backupDestinationDir.isEmpty()) {
        QMessageBox::warning(this, tr("定时备份"), tr("源目录或备份目录未设置。"));
        return;
    }

    // 获取当前时间，用于备份文件命名
    QString backupTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    // 遍历每个源目录并进行备份
    foreach (const QString &sourceDir, scheduledBackupDirs) {
        // 自定义筛选条件（示例：备份所有文件）
        QStringList filters; // 空表示不筛选，备份所有文件
        ba(sourceDir, backupDestinationDir, filters);

        // 重命名备份文件
        QString backupFileName = backupDestinationDir + "/backup_metadata_" + backupTime + ".json";
        QFile::rename(backupDestinationDir + "/backup_metadata.json", backupFileName);

        qDebug() << "定时备份已完成：" << QDateTime::currentDateTime().toString();
    }
}

// 启动定时备份
void Widget::startScheduledBackup(int interval)
{
    // 停止之前的定时器
    scheduledBackupTimer->stop();

    // 设置新的定时间隔（转换为毫秒）
    scheduledBackupTimer->start(interval * 60 * 1000);
}







// 加密数据
QByteArray Widget::encryptData(const QByteArray &data, const QString &password)
{
    QByteArray encrypted = data;
    QByteArray key = password.toUtf8();
    int keyLength = key.size();

    // 密码哈希存储在加密文件的前 16 字节中
    QByteArray passwordHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);

    // 创建加密文件数据
    QByteArray result;
    result.append(passwordHash);  // 将密码哈希加入到数据的开头
    for (int i = 0; i < encrypted.size(); ++i) {
        encrypted[i] = encrypted[i] ^ key[i % keyLength];
    }
    result.append(encrypted);  // 加密数据跟随其后

    return result;
}
// 解密数据
QByteArray Widget::decryptData(const QByteArray &encryptedData, const QString &password)
{
    // 提取密码哈希
    QByteArray passwordHash = encryptedData.left(32);  // SHA-256哈希的长度是 32 字节
    QByteArray data = encryptedData.mid(32);  // 剩余的部分是加密的文件数据

    // 使用输入密码计算哈希并验证
    QByteArray key = password.toUtf8();
    QByteArray calculatedHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);

    if (calculatedHash != passwordHash) {
        // 如果哈希不匹配，则说明密码错误
        return QByteArray();  // 返回空数据表示密码错误
    }

    // 进行解密
    int keyLength = key.size();
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % keyLength];
    }

    return data;
}




// 加密
void Widget::on_encryptButton_clicked()
{
    // 选择要加密的文件或文件夹
    QStringList inputPaths = QFileDialog::getOpenFileNames(this, tr("选择要加密的文件"));
    if (inputPaths.isEmpty()) return;

    // 选择加密后的输出文件夹
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("选择加密后文件的保存目录"));
    if (outputDir.isEmpty()) return;

    // 获取用户密码，从 UI 的 passwordLineEdit 获取
    QString password = ui->passwordLineEdit->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("加密失败"), tr("密码不能为空。"));
        return;
    }

    // 遍历选中的文件并加密
    for (const QString &inputPath : inputPaths) {
        QFileInfo fileInfo(inputPath);
        QString outputFile = outputDir + "/" + fileInfo.fileName() + ".enc";

        // 读取输入文件数据
        QFile inFile(inputPath);
        if (!inFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("加密失败"), tr("无法打开文件 %1。").arg(inputPath));
            continue;
        }
        QByteArray data = inFile.readAll();
        inFile.close();

        // 加密数据
        QByteArray encryptedData = encryptData(data, password);

        // 写入加密后的数据到输出文件
        QFile outFile(outputFile);
        if (!outFile.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("加密失败"), tr("无法创建文件 %1。").arg(outputFile));
            continue;
        }
        outFile.write(encryptedData);
        outFile.close();
    }

    QMessageBox::information(this, tr("加密完成"), tr("选中的文件已成功加密。"));
}


// 解密按钮点击事件
void Widget::on_decryptButton_clicked()
{
    // 选择要解密的文件
    QStringList inputFiles = QFileDialog::getOpenFileNames(this, tr("选择要解密的文件"), "", "Encrypted Files (*.enc)");
    if (inputFiles.isEmpty()) return;

    // 选择解密后的输出文件夹
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("选择解密后文件的保存目录"));
    if (outputDir.isEmpty()) return;

    // 获取用户密码，从 UI 的 passwordLineEdit 获取
    QString password = ui->passwordLineEdit->text();
    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("解密失败"), tr("密码不能为空。"));
        return;
    }

    // 遍历选中的文件并解密
    for (const QString &inputFilePath : inputFiles) {
        QFileInfo fileInfo(inputFilePath);
        QString baseName = fileInfo.completeBaseName(); // 去掉 .enc 后缀
        QString outputFilePath = outputDir + "/" + baseName;

        // 读取加密文件数据
        QFile inFile(inputFilePath);
        if (!inFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("解密失败"), tr("无法打开文件 %1。").arg(inputFilePath));
            continue;
        }
        QByteArray encryptedData = inFile.readAll();
        inFile.close();

        // 解密数据
        QByteArray decryptedData = decryptData(encryptedData, password);

        if (decryptedData.isEmpty()) {
            // 如果解密后的数据为空，说明密码错误
            QMessageBox::warning(this, tr("解密失败"), tr("密码错误或解密失败，请检查密码。"));
            return;
        }

        // 写入解密后的数据到输出文件
        QFile outFile(outputFilePath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("解密失败"), tr("无法创建文件 %1。").arg(outputFilePath));
            continue;
        }
        outFile.write(decryptedData);
        outFile.close();
    }

    QMessageBox::information(this, tr("解密完成"), tr("选中的文件已成功解密。"));
}



// 按钮点击时启动监视多个文件
void Widget::on_startMonitoringButton_clicked()
{
    // 弹出选择文件对话框，允许用户选择多个要监视的文件
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("选择要监视的文件"));

    // 如果用户没有选择文件，则退出
    if (filePaths.isEmpty()) {
        QMessageBox::warning(this, tr("实时备份"), tr("未选择任何文件"));
        return;
    }

    // 弹出选择备份目录对话框，要求用户选择备份的目标目录
    QString selectedBackupDir = QFileDialog::getExistingDirectory(this, tr("选择备份目标目录"));

    // 如果用户没有选择备份目录，则退出
    if (selectedBackupDir.isEmpty()) {
        QMessageBox::warning(this, tr("实时备份"), tr("未选择备份目标目录"));
        return;
    }

    // 设置备份目标目录
    backupDestinationDir = selectedBackupDir;

    // 将选择的文件路径添加到监视列表中
    foreach (const QString &filePath, filePaths) {
        if (!monitoredFiles.contains(filePath)) {
            monitoredFiles.append(filePath);  // 添加到监视文件列表
            fileWatcher->addPath(filePath);   // 将新文件添加到文件监视器

            // 在 UI 的文件列表中显示文件
            QListWidgetItem *item = new QListWidgetItem(filePath);
            ui->fileListWidget->addItem(item);
        }
    }

    QMessageBox::information(this, tr("实时备份"), tr("文件监视已启动，文件变动时将自动备份。"));
}


// 按钮点击时停止监视多个文件
void Widget::on_stopMonitoringButton_clicked()
{
    // 如果没有监视的文件，弹出警告
    if (monitoredFiles.isEmpty()) {
        QMessageBox::warning(this, tr("实时备份"), tr("没有正在监视的文件。"));
        return;
    }

    // 获取用户在列表中选择的文件
    QList<QListWidgetItem *> selectedItems = ui->fileListWidget->selectedItems();

    // 如果没有选择文件，则弹出警告并返回
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("实时备份"), tr("请先选择要停止监视的文件。"));
        return;
    }

    // 遍历选择的文件，移除监视
    foreach (QListWidgetItem *item, selectedItems) {
        QString filePath = item->text();  // 获取文件路径

        // 如果文件在监视列表中，进行移除操作
        if (monitoredFiles.contains(filePath)) {
            // 从监视文件列表中移除文件
            monitoredFiles.removeAll(filePath);

            // 从文件监视器中移除文件
            fileWatcher->removePath(filePath);

            // 从 UI 中删除该文件项
            delete ui->fileListWidget->takeItem(ui->fileListWidget->row(item));
        }
    }

    // 提示用户文件监视已停止
    QMessageBox::information(this, tr("实时备份"), tr("选择的文件监视已停止。"));
}


// 当监视的文件发生变化时调用此函数
void Widget::on_fileChanged(const QString &path)
{
    qDebug() << "文件发生变化，路径：" << path;

    // 检查文件是否存在
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        // 文件被删除
        handleFileDeletion(path);
        return;
    }

    // 如果目标备份文件夹路径有效且正在监视的文件列表中有此文件，则执行备份
    if (!backupDestinationDir.isEmpty() && monitoredFiles.contains(path)) {
        performRealTimeBackup(path);
    }
}


// 执行实时备份操作
void Widget::performRealTimeBackup(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        // 获取当前时间，用于备份文件命名
        QString backupTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

        // 获取原文件名和扩展名
        QString fileName = QFileInfo(file).completeBaseName(); // 不含扩展名
        QString fileExtension = QFileInfo(file).suffix();

        // 生成备份文件名和路径
        QString backupFileName = fileName + "_" + backupTime + "_备份." + fileExtension;
        QString backupFilePath = QDir(backupDestinationDir).filePath(backupFileName);

        // 执行文件复制操作
        if (file.copy(backupFilePath)) {
            qDebug() << "文件已备份：" << backupFilePath;
            QMessageBox::information(this, tr("备份成功"), tr("文件已备份到：%1").arg(backupFilePath));
        } else {
            qDebug() << "文件备份失败：" << file.errorString();
            QMessageBox::warning(this, tr("备份失败"), tr("无法备份文件: %1").arg(path));
        }
    } else {
        // 文件已被删除，触发删除处理
        handleFileDeletion(path);
    }
}


// 处理文件删除
void Widget::handleFileDeletion(const QString &path)
{
    QMessageBox::warning(this, tr("文件被删除"), tr("监视的文件已被删除：%1").arg(path));

    // 从监视列表中移除
    monitoredFiles.removeAll(path);
    fileWatcher->removePath(path);

    // 从 UI 中移除该文件项
    for (int i = 0; i < ui->fileListWidget->count(); ++i) {
        QListWidgetItem *item = ui->fileListWidget->item(i);
        if (item->text() == path) {
            delete ui->fileListWidget->takeItem(i);
            break;
        }
    }
}

