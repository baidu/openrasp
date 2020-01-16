package kafka

import (
	"encoding/json"
	"github.com/Shopify/sarama"
	"github.com/astaxie/beego"
	"rasp-cloud/conf"
	"rasp-cloud/mongo"
	"strings"
)

const (
	kafkaAddrCollectionName   = "kafka"
)

var config *sarama.Config

type Kafka struct {
	KafkaAddr   string     `json:"url"          bson:"url"`
	KafkaUser   string     `json:"user"         bson:"user"`
	KafkaPwd    string     `json:"pwd"          bson:"pwd"`
	KafkaEnable bool       `json:"enable"       bson:"enable"`
	KafkaTopic  string     `json:"topic"        bson:"topic"`
}

func init() {
	config = sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Partitioner = sarama.NewRandomPartitioner
	config.Producer.Return.Successes = true
}

func SendMessage(appId string, key string, val map[string]interface{}) error {
	kafka, _ := GetKafkaConfig(appId)
	if kafka.KafkaEnable && kafka.KafkaTopic != "" && kafka.KafkaAddr != "" {
		addrs := strings.Split(kafka.KafkaAddr, ",")
		if kafka.KafkaUser != "" || kafka.KafkaPwd != "" {
			config.Net.SASL.Enable = true
			config.Net.SASL.User = kafka.KafkaUser
			config.Net.SASL.Password = kafka.KafkaPwd
		}
		producer, err := sarama.NewSyncProducer(addrs, config)
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
			Topic:     kafka.KafkaTopic,
		}
		_, _, err = producer.SendMessage(msg)
		if err != nil {
			beego.Error("Send message Fail")
			return err
		}
	}
	return nil
}

func SendMessages(appId string, key string, valMaps []interface{}) error {
	var msgs []*sarama.ProducerMessage
	kafka, _ := GetKafkaConfig(appId)
	if kafka.KafkaEnable && kafka.KafkaTopic != "" && kafka.KafkaAddr != "" {
		addr := strings.Split(kafka.KafkaAddr, ",")
		if kafka.KafkaUser != "" || kafka.KafkaPwd != "" {
			config.Net.SASL.Enable = true
			config.Net.SASL.User = kafka.KafkaUser
			config.Net.SASL.Password = kafka.KafkaPwd
		}
		producer, err := sarama.NewSyncProducer(addr, config)
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
				Topic:     kafka.KafkaTopic,
			}
			msgs = append(msgs, msg)
		}
		err = producer.SendMessages(msgs)

		if err != nil {
			beego.Error("Send message Fail")
			return err
		}
	}
	return nil
}

func GetKafkaConfig(appId string) (kafka *Kafka, err error) {
	err = mongo.FindId(kafkaAddrCollectionName, appId, &kafka)
	if err != nil {
		kafka = &Kafka{
			KafkaAddr:   conf.AppConfig.KafkaAddr,
			KafkaUser:   conf.AppConfig.KafkaUser,
			KafkaPwd:    conf.AppConfig.KafkaPwd,
			KafkaTopic:  conf.AppConfig.KafkaTopic,
			KafkaEnable: conf.AppConfig.KafkaEnable,
		}
	}
	return kafka, err
}

func PutKafkaConfig(appId string, kafka *Kafka) error {
	err := mongo.UpsertId(kafkaAddrCollectionName, appId, &kafka)
	return err
}