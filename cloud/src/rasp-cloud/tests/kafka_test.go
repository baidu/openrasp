package test

import (
	"testing"
	"github.com/Shopify/sarama"
	"github.com/bouk/monkey"
	. "github.com/smartystreets/goconvey/convey"
	_ "rasp-cloud/tests/inits"
	_ "rasp-cloud/tests/start"
	"rasp-cloud/kafka"
	"rasp-cloud/mongo"
	"errors"
)

func TestSendMessage(t *testing.T) {
	Convey("Subject: Test Kafka interface\n", t, func() {
		Convey("when GetKafkaConfig occurs error", func() {
			monkey.Patch(kafka.GetKafkaConfig, func() (kafka *kafka.Kafka, err error){
				return nil, errors.New("")
			})
			defer monkey.Unpatch(kafka.GetKafkaConfig)
			err := kafka.SendMessage("test", "test", map[string]interface{}{})
			So(err, ShouldNotBeNil)
		})

		Convey("when NewSyncProducer occurs error", func() {
			monkey.Patch(kafka.GetKafkaConfig, func() (kafkaClient *kafka.Kafka, err error){
				return &kafka.Kafka{KafkaEnable:true}, nil
			})
			defer monkey.Unpatch(kafka.GetKafkaConfig)

			monkey.Patch(sarama.NewAsyncProducer,
				func(addrs []string, conf *sarama.Config) (sarama.AsyncProducer, error){
					return nil, errors.New("")
				})
			defer monkey.Unpatch(sarama.NewAsyncProducer)
			err := kafka.SendMessage("test", "test", map[string]interface{}{})
			So(err, ShouldNotBeNil)
		})

		Convey("when producer SendMessage occurs error", func() {
			monkey.Patch(kafka.GetKafkaConfig, func() (kafkaClient *kafka.Kafka, err error){
				return &kafka.Kafka{KafkaEnable:true}, nil
			})
			defer monkey.Unpatch(kafka.GetKafkaConfig)

			monkey.Patch(sarama.NewAsyncProducer,
				func(addrs []string, conf *sarama.Config) (sarama.AsyncProducer, error){
					return nil, nil
				})
			defer monkey.Unpatch(sarama.NewAsyncProducer)
			err := kafka.SendMessage("test", "test", map[string]interface{}{})
			So(err, ShouldNotBeNil)
		})
	})
}

func TestGetKafkaConfig(t *testing.T) {
	Convey("Subject: Test Get Kafka interface\n", t, func() {
		Convey("when mongos occurs error", func() {
			monkey.Patch(mongo.FindId, func(collection string, id string, result interface{}) (err error){
				return nil
			})
			monkey.Unpatch(mongo.FindId)
			_, err := kafka.GetKafkaConfig()
			So(err, ShouldEqual, nil)
		})
	})
}