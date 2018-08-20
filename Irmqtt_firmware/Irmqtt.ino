// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
Name:       irext_firmware.ino
Created:  2018/6/14 15:54:10
Author:     Caffreyfans
*/

// Define User Types below here or use a .h file
//
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <String.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "./include/ir_decode.h"
#include <PubSubClient.h>

// Define Function Prototypes that use User Types below here or use a .h file
//
#define MAX_PACKETSIZE 512 //UDP包大小
#define SERIAL_DEBUG Serial
#define TRY_COUNT 5
#define UDP_PORT 8000
#define USER_DATA_SIZE 1024

char buffUDP[MAX_PACKETSIZE];
WiFiUDP udp;
DynamicJsonBuffer jsonBuffer;
JsonObject& settings_json = jsonBuffer.createObject();
WiFiClient espClient;
PubSubClient client(espClient);
UINT16 user_data[USER_DATA_SIZE];

static t_remote_ac_status ac_status =
{
	// 默认空调状态
	AC_POWER_ON,
	AC_TEMP_24,
	AC_MODE_COOL,
	AC_SWING_ON,
	AC_WS_AUTO,
	1,
	0,
	0
};



/**
说明：初始化UDP服务端
参数：服务端口
返回值：成功返回1, 失败返回0
*/
uint8_t startUDPServer(uint16 port);


/**
说明：向UDP客户端发送数据
参数：发送数据
返回值：成功返回1，失败返回0
*/
int sendUDP(const char *p);


/**
说明：UDP消息监听
返回值：信息到达返回true, 否则返回false
*/
boolean doUDPServerTick();


/**
说明：获取下载列表
*/
boolean getList();

/**
说明：下载文件
参数：文件id
返回值：成功返回true, 否则返回false
*/
boolean downLoadFile(int index_id);



/**
说明：发射红外信号
参数：
返回值：成功返回true, 失败返回false
*/
boolean sendIR();


/**
说明：连接mqtt服务器
参数：JsonObject 对象（host, port, user, password)
返回值：成功返回true, 失败返回false
*/
boolean connectMqtt();


/**
说明：mqtt消息回调
参数：topic 主题， payload 消息， length 消息长度
*/
void callback(char* topic, byte* payload, unsigned int length);



/**
说明：处理UDP报文
参数：报文内容
*/
void disposeUdpMessage(String msg);



/**
说明：保存设置
*/
boolean saveSettings();


/**
说明：提取设置
*/
boolean getSettings();


/**
说明：将字符串转换成对应枚举值
参数：字符串
*/
int coverToEnum(String str, String type);


void setup()
{
	SERIAL_DEBUG.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.beginSmartConfig();
	//WiFi.begin(ssid);
	SERIAL_DEBUG.println("\nConnecting to WiFi");
	while (1) {
		SERIAL_DEBUG.print(".");
		delay(500);
		if (WiFi.status() == WL_CONNECTED) {
			SERIAL_DEBUG.println("\r\nSmartConfig Success");
			SERIAL_DEBUG.printf("SSID:%s\r\n", WiFi.SSID().c_str());
			SERIAL_DEBUG.printf("PSW:%s\r\n", WiFi.psk().c_str());
			SERIAL_DEBUG.println(WiFi.localIP());
			break;
		}
	}
	startUDPServer(UDP_PORT);
	SPIFFS.begin();
	settings_json["device_name"] = "irmqtt";
	settings_json["device_type"] = "ac";
	if (getSettings() == true) {
		SERIAL_DEBUG.println("get settings_json");
		settings_json.printTo(SERIAL_DEBUG);
		if (settings_json.containsKey("mqtt"))
			connectMqtt();
	}
}

// Add the main program code into the continuous loop() function
void loop() {

	client.loop();

	if (doUDPServerTick() == true) {
		String msg = buffUDP;
		SERIAL_DEBUG.println(msg);
		disposeUdpMessage(msg);
	}
	delay(5);
}



uint8_t startUDPServer(uint16 port) {
	SERIAL_DEBUG.print("StartUDPServer at port: ");
	SERIAL_DEBUG.println(port);
	return udp.begin(port);
}



int sendUDP(const char *p) {
	udp.beginPacket(udp.remoteIP(), UDP_PORT);
	udp.write(p);
	return udp.endPacket();
}



boolean doUDPServerTick() {

	int packetSize = udp.parsePacket();
	if (packetSize) {
		SERIAL_DEBUG.print("Received packet of size = ");
		SERIAL_DEBUG.println(packetSize);
		SERIAL_DEBUG.print("From ");
		IPAddress remote = udp.remoteIP();
		for (int i = 0; i < 4; i++) {
			SERIAL_DEBUG.print(remote[i], DEC);
			if (i < 3)
				SERIAL_DEBUG.print(". ");
		}
		SERIAL_DEBUG.print(", port ");
		SERIAL_DEBUG.println(udp.remotePort());
		memset(buffUDP, 0x00, sizeof(buffUDP));
		udp.read(buffUDP, MAX_PACKETSIZE - 1);
		udp.flush();
		return true;
	}
	else
		return false;
}





void disposeUdpMessage(String msg) {
	JsonObject& object = jsonBuffer.parse(msg);
	if (object.success()) {
		if (object.containsKey("auth")) {
			settings_json["download_post"] = object["auth"];
		}
		if (object.containsKey("mqtt")) {
			settings_json["mqtt"] = object["mqtt"];
			if (connectMqtt() == true) {
				sendUDP("mqtt connected");
			}
			else {
				sendUDP("timeout");
			}
		}
		if (object.containsKey("list")) {
			settings_json["list"] = object["list"];
			if (getList() == true)	sendUDP("get list success");
			else sendUDP("get list error");
		}
		if (object.containsKey("device_type")) {
			settings_json["device_type"] = object["device_type"];
		}
		if (object.containsKey("data_pin")) {
			settings_json["data_pin"] = object["data_pin"];
		}
		if (object.containsKey("use_file")) {
			settings_json["use_file"] = object["use_file"];
		}
		if (object.containsKey("cmd")) {
			sendIR();
		}
		settings_json.printTo(SERIAL_DEBUG);
		saveSettings();
	}
	else {
		SERIAL_DEBUG.println("parse JsonObject failed");
	}
}

boolean connectMqtt() {
	
	const char* host = settings_json["mqtt"]["host"];
	const int port = settings_json["mqtt"]["port"];
	const char* user = settings_json["mqtt"]["user"];
	const char* password = settings_json["mqtt"]["password"];
	SERIAL_DEBUG.printf("host = %s\n", host);
	SERIAL_DEBUG.printf("port = %d\n", port);
	SERIAL_DEBUG.printf("user = %s\n", user);
	SERIAL_DEBUG.printf("password = %s\n", password);
	client.setServer(host, port);
	client.setCallback(callback);

	unsigned long time_start = millis();
	unsigned long time_end = millis();
	client.disconnect();
	while (!client.connected()) {
		Serial.println("Connecting to MQTT...");

		if (client.connect("ESP8266Client", user, password)) {
			Serial.println("connected");
		}
		else {
			Serial.print("failed with state ");
			Serial.println(client.state());
			delay(1000);
		}
		time_end = millis();
		if (time_end - time_start > 10000) {
			SERIAL_DEBUG.println("connect timeout");
			sendUDP("time out");
			return false;
		}
	}
	String device_name = settings_json["device_name"];
	String device_type = settings_json["device_type"];
	String topic = String(device_name + "/" + device_type + "/#");
	client.subscribe(topic.c_str());
	sendUDP("connected");
	return true;
}

void callback(char* topic, byte* payload, unsigned int length) {
	
	String cmd = "";
	for (int i = 0; i < length; i++)
		cmd += (char)payload[i];

	String topic_str = String(topic);
	String device_name = settings_json["device_name"];
	String device_type = settings_json["device_type"];
	String topic_head = String(device_name + "/" + device_type + "/");
	String mode_set = String("mode/set");
	String temperature_set = String("temperature/set");
	String swing_set = String("swing/set");
	String speed_set = String("fan/set");

	if ((topic_str == (topic_head + mode_set)) && (cmd == "False")) {
		ac_status.ac_power = AC_POWER_OFF;
	}
	else {
		ac_status.ac_power = AC_POWER_ON;
	}
	if (topic_str == (topic_head + temperature_set)) {
		int tmp = cmd.toInt();
		t_ac_temperature temperature = t_ac_temperature(tmp - 16);
		ac_status.ac_temp = temperature;
	}
	if (topic_str == (topic_head + mode_set) && (cmd != "False")) {
		ac_status.ac_mode = (t_ac_mode)(coverToEnum(cmd, "mode"));
	}
	if (topic_str == (topic_head + swing_set)) {
		ac_status.ac_wind_dir = (t_ac_swing)(coverToEnum(cmd, "swind"));
	}
	if (topic_str == (topic_head + speed_set)) {
		ac_status.ac_wind_speed = (t_ac_wind_speed)(coverToEnum(cmd, "swind_speed"));
	}

	sendIR();
	SERIAL_DEBUG.println("***   ac_status   ***");
	SERIAL_DEBUG.printf("power = %d\n", ac_status.ac_power);
	SERIAL_DEBUG.printf("temperature = %d\n", ac_status.ac_temp);
	SERIAL_DEBUG.printf("swing = %d\n", ac_status.ac_wind_dir);
	SERIAL_DEBUG.printf("speed = %d\n", ac_status.ac_wind_speed);
	SERIAL_DEBUG.printf("mode = %d\n", ac_status.ac_mode);
	SERIAL_DEBUG.println("**********************");
}

int coverToEnum(String str, String type) {
	String swing_str[] = {"True", "False"};
	String swing_speed_str[] = { "auto", "low", "medium", "high" };
	String mode_str[] = { "cool", "heat", "auto", "fan", "dry" };
	if (type == "mode") 
		for (int i = 0; i < sizeof(mode_str) / sizeof(mode_str[0]); i++) {
			if (str == mode_str[i])	return i;
		}
	
	if (type == "swind")
		for (int i = 0; i < sizeof(swing_str) / sizeof(swing_str[0]); i++) {
			if (str == swing_str[i])	return i;
		}
	if (type == "swind_speed")
		for (int i = 0; i < sizeof(swing_speed_str) / sizeof(swing_speed_str[0]); i++) {
			if (str == swing_speed_str[i])	return i;
		}
}

boolean getList() {
	boolean ret = true;
	for (int i = 0; i < settings_json["list"].size(); i++) {
		int count = 0;
		do {
			ret = downLoadFile(settings_json["list"][i]);
			if (count > TRY_COUNT)	break;
			else  count++;
		} while (ret == false);
	}

	if (ret == true) {
		SERIAL_DEBUG.println("download success");
		sendUDP("download success");
	}
	else {
		SERIAL_DEBUG.println("download failed");
		sendUDP("download failed");
	}
	return ret;
}

boolean downLoadFile(int index_id) {

	HTTPClient http;
	const char* download_url = "http://irext.net/irext-server/operation/download_bin";
	boolean flag = false;

	if (WiFi.status() == WL_CONNECTED) {

		// Prepare cache file
		String filename = String(index_id);
		if (SPIFFS.exists(filename)) {
			SERIAL_DEBUG.println("already have file");
			return true;
		}
		File cache = SPIFFS.open(filename, "w");
		File* filestream = &cache;
		if (!cache) {
			SERIAL_DEBUG.println("Could not create cache file");
			return false;
		}
		else {
			int httpCode = 0;
			settings_json["download_post"]["indexId"] = index_id;
			String tmp;
			settings_json["download_post"].printTo(tmp);

			http.begin(download_url);
			http.addHeader("Content-Type", "application/json");
			httpCode = http.POST(tmp);


			SERIAL_DEBUG.print("[HTTP] POST return code: ");
			SERIAL_DEBUG.println(httpCode);

			if (httpCode == HTTP_CODE_OK) {
				http.writeToStream(filestream);
				SERIAL_DEBUG.printf("download %s ok\n", filename.c_str());
				flag = true;
			}
			else {
				SPIFFS.remove(filename);
			}
			cache.close();
			http.end();
			delay(10);
		}
	}
	else {
		SERIAL_DEBUG.println("Error in WiFi connection");
	}
	return flag;
}

boolean sendIR() {

	String filename = settings_json["use_file"];
	String tmp = settings_json["data_pin"];
	int data_pin = tmp.toInt();
	IRsend irsend = IRsend(data_pin);
	irsend.begin();
	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open(filename, "r");
		File *fp = &f;
		if (f) {
			UINT16 content_length = f.size();
			if (content_length == 0) {
				return false;
			}
			SERIAL_DEBUG.printf("content_length = %d\n", content_length);
			UINT8 *content = (UINT8 *)malloc(content_length * sizeof(UINT8));
			f.seek(0L, fs::SeekSet);
			f.readBytes((char*)content, content_length);
			INT8 ret = ir_binary_open(IR_CATEGORY_AC, 1, content, content_length);
			int length = ir_decode(2, user_data, &ac_status, 0);
			SERIAL_DEBUG.println();
			for (int i = 0; i < length; i++) {
				Serial.printf("%d ", user_data[i]);
			}
			SERIAL_DEBUG.println();
			irsend.sendRaw(user_data, length, 38);
			ir_close();
			return true;
		}
		else {
			SERIAL_DEBUG.printf("open %s was failed\n", filename.c_str());
			return false;
		}
		f.close();
	}
	else {
		SERIAL_DEBUG.printf("%s is not exsits\n", filename.c_str());
		return false;
	}
}

boolean saveSettings() {
	File cache = SPIFFS.open("settings", "w");
	Stream* file_stream = &cache;
	if (cache) {
		String tmp;
		settings_json.printTo(tmp);
		cache.println(tmp);
	}
	else {
		SERIAL_DEBUG.println("can't open settings");
		return false;
	}
	cache.close();
	return true;
}

boolean getSettings() {
	File cache = SPIFFS.open("settings", "r");
	if (cache) {
		String tmp = cache.readString();
		JsonObject& json_object = jsonBuffer.parseObject(tmp);
		JsonObject::iterator it;
		for (it = json_object.begin(); it != json_object.end(); ++it) {
			settings_json[it->key] = json_object[it->key];
		}
	}
	else {
		SERIAL_DEBUG.println("settings is not exits");
		return false;
	}
	cache.close();
	return true;
}