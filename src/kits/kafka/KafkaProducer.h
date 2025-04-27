#pragma once
#include <librdkafka/rdkafka.h>
#include <mutex>
#include <string>
namespace _Kits
{
    class KafkaProducer
    {
      public:
        KafkaProducer(const std::string &brokers, const std::string &topic);
        ~KafkaProducer();

        bool sendMessage(const std::string &message);

      private:
        rd_kafka_t *producer_;
        rd_kafka_topic_t *topic_;
        rd_kafka_conf_t *config_;
        std::mutex mutex_;

        static void errorCallback(rd_kafka_t *rk,
                                  int err,
                                  const char *reason,
                                  void *opaque);
        static void logCallback(const rd_kafka_t *rk,
                                int level,
                                const char *fac,
                                const char *buf);
    };
} // namespace _Kits