#pragma once
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <fmt/core.h>
#include <libzippp/libzippp.h>
#include <mutex>
#include <spdlog/details/file_helper.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace _Kits
{
template <typename Mutex>
class PlainTextSink final : public spdlog::sinks::base_sink<Mutex>
{
  public:
    PlainTextSink(std::string logDirectory,
                  std::size_t maxSize,
                  const spdlog::file_event_handlers &eventHandlers = {})
        : m_log_directory(std::move(logDirectory)), m_max_size(maxSize),
          m_file_helper(eventHandlers)
    {
        if (maxSize == 0)
        {
            spdlog::throw_spdlog_ex(
                "PlainTextSink constructor: max_size cannot be zero");
        }

        m_last_rotation_time = QDateTime::currentDateTime();
        m_archive_file_name = m_log_directory + "/collection.zip";
        // archive_and_rotate_();

        m_base_file_name = m_log_directory + generate_timestamped_filename();
        m_file_helper.open(m_base_file_name);
        m_current_size = m_file_helper.size();
    }

    spdlog::filename_t filename()
    {
        std::lock_guard lock(m_mutex);
        return m_file_helper.filename();
    }

  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        if (should_rotate_daily_())
        {
            m_file_helper.flush();
            m_file_helper.close();
            //archive_and_rotate_();
            m_base_file_name =
                m_log_directory + generate_timestamped_filename();
            m_file_helper.open(m_base_file_name);
            m_current_size = 0;
        }

        auto newSize = m_current_size + formatted.size();
        if (newSize > m_max_size)
        {
            m_file_helper.flush();
            if (m_file_helper.size() > 0)
            {
                m_file_helper.close();
                // archive_and_rotate_();
                m_base_file_name =
                    m_log_directory + generate_timestamped_filename();
                m_file_helper.open(m_base_file_name);
                m_current_size = 0;
            }
        }

        m_file_helper.write(formatted);
        m_current_size = newSize;
    }

    void flush_() override
    {
        m_file_helper.flush();
    }

  private:
    std::string generate_timestamped_filename()
    {
        return QDateTime::currentDateTime()
            .toString("/yyyyMMdd_HHmmss.log")
            .toStdString();
    }

    bool should_rotate_daily_()
    {
        auto now = QDateTime::currentDateTime();
        if (m_last_rotation_time.date() != now.date())
        {
            m_last_rotation_time = now;
            return true;
        }
        return false;
    }

    void archive_and_rotate_()
    {
        QDir logDir(m_log_directory.c_str());
        auto logFiles = logDir.entryList({"*.log"}, QDir::Files);
        QString curtime = QString::fromStdString(
            generate_timestamped_filename().substr(1, 8));
        // 按日期分组日志文件
        QMap<QString, QStringList> dateGroups;
        for (const auto &logFile : logFiles)
        {
            // 从文件名中提取日期（格式：yyyyMMdd_HHmmss.log）
            QString dateStr = logFile.left(8); // 获取 yyyyMMdd 部分
            if (dateStr == curtime)
                continue;
            dateGroups[dateStr].append(logFile);
        }

        // 为每个日期创建单独的压缩文件
        for (auto it = dateGroups.begin(); it != dateGroups.end(); ++it)
        {
            QString dateStr = it.key();
            const QStringList &filesForDate = it.value();

            // 创建以日期命名的压缩文件
            std::string archiveName =
                m_log_directory + "/" + dateStr.toStdString() + ".zip";
            libzippp::ZipArchive zf(archiveName);

            if (zf.open(libzippp::ZipArchive::Write))
            {
                for (const auto &logFile : filesForDate)
                {
                    QString filePath = logDir.filePath(logFile);
                    QFile file(filePath);
                    if (file.open(QIODevice::ReadOnly))
                    {
                        QByteArray fileData = file.readAll();
                        zf.addData(logFile.toStdString(),
                                   fileData.data(),
                                   fileData.size());
                        file.close();
                        QFile::remove(filePath);
                    }
                }
                zf.close();
            }
        }
    }

  private:
    std::mutex m_mutex;
    QDateTime m_last_rotation_time;
    std::string m_log_directory;
    std::string m_base_file_name;
    std::string m_archive_file_name;
    std::size_t m_max_size;
    std::size_t m_current_size;
    spdlog::details::file_helper m_file_helper;
};

using PlainTextSink_st = PlainTextSink<spdlog::details::null_mutex>;
using PlainTextSink_mt = PlainTextSink<std::mutex>;
} // namespace _Kits