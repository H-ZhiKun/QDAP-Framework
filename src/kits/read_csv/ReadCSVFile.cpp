#include "ReadCSVFile.h"

// #include <QTextCodec>
#include <QByteArray>
#include <qfile.h>
#include <QTextStream>

int ReadCSVFile::readFile(const std::string& filePath,QList<QStringList>& csvData)
{
    // QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    csvData.clear();
    QByteArray filename = QByteArray::fromStdString(filePath);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        return -1;
    }
    QTextStream in(&file);
    while (!in.atEnd()) 
    {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        csvData.emplace_back(fields);
    }

    file.close();

    return 0;
}