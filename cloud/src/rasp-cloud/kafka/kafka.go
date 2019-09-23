package kafka

import (
	"encoding/json"
	"fmt"
	"github.com/Shopify/sarama"
	"github.com/astaxie/beego"
	"rasp-cloud/conf"
)

var config *sarama.Config

func init() {
	config = sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Partitioner = sarama.NewRandomPartitioner
	config.Producer.Return.Successes = true
}

func SendMessage(topic string, key string, val map[string]interface{}) error {
	producer, err := sarama.NewSyncProducer([]string{conf.AppConfig.KafkaAddr}, config)
	if err != nil {
		beego.Error(err)
		return err
	}

	defer producer.Close()

	msg := &sarama.ProducerMessage {
		Partition: int32(1),
		Key:       sarama.StringEncoder(key),
	}

	msg.Topic = topic
	var content []byte
	content, err = json.Marshal(val)
	if err != nil{
		return err
	}
	sContent := string(content)
	msg.Value = sarama.ByteEncoder(sContent)
	partition, offset, err := producer.SendMessage(msg)
	fmt.Printf("topic = %v\n", topic)

	if err != nil {
		beego.Error("Send message Fail")
		return err
	}
	fmt.Printf("Partition = %d, offset=%d\n", partition, offset)
	return nil
}