package kafka

import (
	"encoding/json"
	"github.com/Shopify/sarama"
	"github.com/astaxie/beego"
	"rasp-cloud/mongo"
)

const (
	kafkaAddrId               = "0"
	kafkaAddrCollectionName   = "kafka"
)

var config *sarama.Config

type Kafka struct {
	KafkaAddr   []string   `json:"kafka_addr"   bson:"kafka_addr"`
	KafkaUser   string     `json:"kafka_user"   bson:"kafka_user"`
	KafkaPwd    string     `json:"kafka_pwd"    bson:"kafka_pwd"`
	KafkaEnable bool       `json:"kafka_enable" bson:"kafka_enable"`
}

func init() {
	config = sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Partitioner = sarama.NewRandomPartitioner
	config.Producer.Return.Successes = true
}

func SendMessage(topic string, key string, val map[string]interface{}) error {
	kafka, _ := GetKafkaConfig()
	if kafka.KafkaEnable {
		producer, err := sarama.NewSyncProducer(kafka.KafkaAddr, config)
		if err != nil {
			beego.Error(err)
			return err
		}
		defer producer.Close()

		var content []byte
		content, err = json.Marshal(val)
		if err != nil{
			return err
		}
		sContent := string(content)
		msg := &sarama.ProducerMessage {
			Partition: int32(1),
			Key:       sarama.StringEncoder(key),
			Value:     sarama.ByteEncoder(sContent),
			Topic:     topic,
		}
		_, _, err = producer.SendMessage(msg)
		if err != nil {
			beego.Error("Send message Fail")
			return err
		}
	}
	return nil
}

func SendMessages(topic string, key string, valMaps []interface{}) error {
	var msgs []*sarama.ProducerMessage
	kafka, _ := GetKafkaConfig()
	if kafka.KafkaEnable{
		producer, err := sarama.NewSyncProducer(kafka.KafkaAddr, config)
		if err != nil {
			beego.Error(err)
			return err
		}
		defer producer.Close()

		var content []byte
		for _, val := range valMaps {
			content, err = json.Marshal(val)
			if err != nil{
				return err
			}
			sContent := string(content)
			msg := &sarama.ProducerMessage {
				Partition: int32(1),
				Key:       sarama.StringEncoder(key),
				Value:     sarama.ByteEncoder(sContent),
				Topic:     topic,
			}
			msgs = append(msgs, msg)
		}
		err = producer.SendMessages(msgs)

		if err != nil {
			beego.Error("Send message Fail")
			return err
		}
	} else {
		beego.Error("kafka is not enabled!")
	}
	return nil
}

func GetKafkaConfig() (kafka *Kafka, err error) {
	err = mongo.FindId(kafkaAddrCollectionName, kafkaAddrId, &kafka)
	if err != nil {
		kafka = &Kafka{
			KafkaAddr:   []string{},
			KafkaUser:   "",
			KafkaPwd:    "",
			KafkaEnable: false,
		}
	}
	return kafka, err
}

func PutKafkaConfig(kafka *Kafka) error {
	err := mongo.UpsertId(kafkaAddrCollectionName, kafkaAddrId, &kafka)
	return err
}