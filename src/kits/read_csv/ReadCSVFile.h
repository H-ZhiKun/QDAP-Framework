#pragma once
#include <string>
#include <QList>
#include <QStringList>

/***************************************************************************
 * @file    ReadCSVFile.h
 * @brief   qt读取csv文件
 *          
 *
 *
 * @note   
 ***************************************************************************/

class ReadCSVFile
{
private:
    /* data */
public:
    ReadCSVFile(/* args */) = default;
    ~ReadCSVFile()   = default;

     /**********************
     * @brief 读取csv文件
     * @param  filePath  文件名
     * @param   csvData  读取到的数据
     * @return 
     ************************ */
    int readFile(const std::string& filePath,QList<QStringList>& csvData);


signals:
    /* data */
    void CSVDataSend(const QList<QStringList>& csvData);

private slots:
    void onCSVDataSend(const QList<QStringList>& csvData);



    
};

