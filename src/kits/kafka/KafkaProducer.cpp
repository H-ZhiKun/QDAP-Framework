#include "KafkaProducer.h"
#include "kits/required/log/CRossLogger.h"
namespace _Kits
{
    KafkaProducer::KafkaProducer(const std::string &brokers,
                                 const std::string &topic)
        : producer_(nullptr), topic_(nullptr), config_(rd_kafka_conf_new())
    {

        if (!config_)
        {
            LogError("Kafka: Failed to create config!");
            return;
        }

        // 设置 Kafka 服务器地址
        if (rd_kafka_conf_set(
                config_, "bootstrap.servers", brokers.c_str(), nullptr, 0) !=
            RD_KAFKA_CONF_OK)
        {
            LogError("Kafka: Failed to set brokers");
            return;
        }

        // 绑定错误回调
        rd_kafka_conf_set_error_cb(config_, errorCallback);
        rd_kafka_conf_set_log_cb(config_, logCallback);

        // 创建 Kafka 生产者实例
        char errstr[512];
        producer_ =
            rd_kafka_new(RD_KAFKA_PRODUCER, config_, errstr, sizeof(errstr));
        if (!producer_)
        {
            LogError("Kafka: Failed to create producer: {}", errstr);
            return;
        }

        // 绑定 topic
        topic_ = rd_kafka_topic_new(producer_, topic.c_str(), nullptr);
        if (!topic_)
        {
            LogError("Kafka: Failed to create topic");
        }
    }

    KafkaProducer::~KafkaProducer()
    {
        if (topic_)
        {
            rd_kafka_topic_destroy(topic_);
        }
        if (producer_)
        {
            rd_kafka_flush(producer_, 1000); // 确保所有消息发送完成
            rd_kafka_destroy(producer_);
        }
    }

    bool KafkaProducer::sendMessage(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!producer_ || !topic_)
        {
            LogError("Kafka: Producer not initialized!");
            return false;
        }

        int err = rd_kafka_produce(topic_,
                                   RD_KAFKA_PARTITION_UA,
                                   RD_KAFKA_MSG_F_COPY,
                                   (void *)message.c_str(),
                                   message.size(),
                                   nullptr,
                                   0,
                                   nullptr);

        if (err != 0)
        {
            LogError("Kafka: Failed to send message: {}",
                     rd_kafka_err2str(rd_kafka_last_error()));

            return false;
        }

        rd_kafka_poll(producer_, 0); // 触发 Kafka 发送
        return true;
    }

    // Kafka 错误回调
    void KafkaProducer::errorCallback(rd_kafka_t *rk,
                                      int err,
                                      const char *reason,
                                      void *opaque)
    {
        LogError("Kafka Error [{}]: {}",
                 rd_kafka_err2name((rd_kafka_resp_err_t)err),
                 reason);
    }

    // Kafka 日志回调
    void KafkaProducer::logCallback(const rd_kafka_t *rk,
                                    int level,
                                    const char *fac,
                                    const char *buf)
    {
        LogInfo("Kafka Log [{}]: {}", fac, buf);
    }
} // namespace _Kits