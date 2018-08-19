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
#define IR_LED D5	//默认红外发射引脚
#define TRY_COUNT 5
#define UDP_PORT 8000

char buffUDP[MAX_PACKETSIZE];
WiFiUDP udp;
DynamicJsonBuffer jsonBuffer;
JsonObject& downloadPost = jsonBuffer.createObject();
WiFiClient espClient;
PubSubClient client(espClient);
UINT16 user_data[USER_DATA_SIZE];
IRsend irsend(IR_LED);
String index_id;
// 测试数据
char* mqttServer = "";
int mqttPort = 1883;
char* mqttUser = "";
char* mqttPassword = "";

static t_remote_ac_status ac_status =
{
	// 默认空调状态
	AC_POWER_OFF,
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
boolean getList(String msg);

/**
说明：下载文件
参数：文件id
返回值：成功返回true, 否则返回false
*/
boolean downLoadFile(int index_id);



/**
说明：显示文件内容
参数：文件路径
返回值：成功返回1， 失败返回0
*/
int showFileData(String filename);


/**
说明：显示内存信息
*/
void showMemoryInfo();


/**
说明：发射红外信号
参数：
返回值：成功返回true, 失败返回false
*/
boolean sendIR(String filename);


/**
说明：mqtt消息回调
参数：topic 主题， payload 消息， length 消息长度
*/
void callback(char* topic, byte* payload, unsigned int length);


// The setup() function runs once each time the micro-controller starts
void setup()
{
	SERIAL_DEBUG.begin(115200);
	SERIAL_DEBUG.println("Start module");
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
	SPIFFS.begin();
	startUDPServer(UDP_PORT);
	irsend.begin();

	client.setServer(mqttServer, mqttPort);
	client.setCallback(callback);

	while (!client.connected()) {
		Serial.println("Connecting to MQTT...");

		if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {

			Serial.println("connected");

		}
		else {

			Serial.print("failed with state ");
			Serial.println(client.state());
			delay(2000);

		}
	}

	client.subscribe("#");

	if (SPIFFS.exists("index_id")) {
		File f = SPIFFS.open("index_id", "r");
		index_id = f.readString();
		SERIAL_DEBUG.printf("get the index = %s", index_id.c_str());
	}

}

// Add the main program code into the continuous loop() function
void loop() {

	client.loop();

	if (doUDPServerTick() == true) {
		String msg = buffUDP;
		SERIAL_DEBUG.println(msg);
		int i = msg.toInt();
		if (i > 0 && i < 10000) {
			index_id = String(i);
			File f = SPIFFS.open("index_id", "w");
			if (f) {
				f.println(index_id);
				SERIAL_DEBUG.printf("save the index_id %s\n", index_id.c_str());
			}
			f.close();
		}
		else if (msg.equals("send")) {
			sendIR(index_id);
		}
		else {
			getList(msg);
		}	
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



boolean getList(String msg) {


	JsonArray& array = jsonBuffer.parseArray(msg);
	JsonObject& object = jsonBuffer.parseObject(msg);
	boolean ret = true;

	if (array.success()) {
		for (int i = 0; i < array.size(); i++) {
			int count = 0;
			do {
				ret = downLoadFile(array[i]);
				if (count > TRY_COUNT) {
					break;
				}
				else {
					count++;
				}
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
	}

	if (object.success()) {
		downloadPost["id"] = object["id"];
		downloadPost["token"] = object["token"];
		SERIAL_DEBUG.println("get the id and token");
	}
	return true;
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
			downloadPost["indexId"] = index_id;
			String tmp;
			downloadPost.printTo(tmp);

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

int showFileData(String filename) {

	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open(filename, "r");
		int s = f.size();
		String data = f.readString();
		SERIAL_DEBUG.printf("open \"%s\" right the Size = %d\n", filename.c_str(), s);
		SERIAL_DEBUG.println("\n************* data of file: *************");
		SERIAL_DEBUG.println(data);
		SERIAL_DEBUG.println("*************  end of data  **************\n");
		f.close();

		return s;
	}
	else {
		SERIAL_DEBUG.printf("%s is not exsited", filename.c_str());
		return 0;
	}
}



void showMemoryInfo() {
	FSInfo fs_info;
	SPIFFS.info(fs_info);
	SERIAL_DEBUG.println("\n************* Borad Info *************");
	SERIAL_DEBUG.printf("totalBytes: %d\n", fs_info.totalBytes);
	SERIAL_DEBUG.printf("usedBytes: %d\n", fs_info.usedBytes);
	SERIAL_DEBUG.printf("blockSize: %d\n", fs_info.blockSize);
	SERIAL_DEBUG.printf("pageSize: %d\n", fs_info.pageSize);
	SERIAL_DEBUG.printf("maxOpenFiles: %d\n", fs_info.maxOpenFiles);
	SERIAL_DEBUG.printf("maxPathLength: %d\n", fs_info.maxPathLength);
	SERIAL_DEBUG.println("************* End Of Info ************\n");
}



boolean sendIR(String filename) {
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
			for (int i = 0; i < length; i++) {
				Serial.printf("%d ", user_data[i]);
			}
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



void callback(char* topic, byte* payload, unsigned int length) {

	Serial.print("Message arrived in topic: ");
	Serial.println(topic);
	Serial.print("Message:");
	String str = "";
	for (int i = 0; i < length; i++) {
		str += (char)payload[i];
	}
	Serial.println(str);
	Serial.println("-----------------------");

	if (strcmp(topic, "study/ac/mode/set") == 0 && str.equals("False")) {
		ac_status.ac_power = AC_POWER_OFF;
	}
	else {
		ac_status.ac_power = AC_POWER_ON;
	}

	if (strcmp(topic, "study/ac/temperature/set") == 0) {
		
		int tmp = str.toInt();
		t_ac_temperature temp = (t_ac_temperature)(tmp - 16);
		ac_status.ac_temp = temp;
	}

	else if (strcmp(topic, "study/ac/mode/set") == 0) {

		if (str.equals("auto"))
			ac_status.ac_mode = AC_MODE_AUTO;
		if (str.equals("cool"))
			ac_status.ac_mode = AC_MODE_COOL;
		if (str.equals("heat"))
			ac_status.ac_mode = AC_MODE_HEAT;
		if (str.equals("dry"))
			ac_status.ac_mode = AC_MODE_DRY;
		if (str.equals("fan"))
			ac_status.ac_mode = AC_MODE_FAN;
	}

	else if (strcmp(topic, "study/ac/swing/set") == 0) {

		if (str.equals("False"))
			ac_status.ac_wind_dir = AC_SWING_OFF;
		if (str.equals("True"))
			ac_status.ac_wind_dir = AC_SWING_ON;
	}

	else if (strcmp(topic, "study/ac/fan/set")) {

		if (str.equals("auto"))
			ac_status.ac_wind_speed = AC_WS_AUTO;
		if (str.equals("high"))
			ac_status.ac_wind_speed = AC_WS_HIGH;
		if (str.equals("medium"))
			ac_status.ac_wind_speed = AC_WS_MEDIUM;
		if (str.equals("low"))
			ac_status.ac_wind_speed = AC_WS_LOW;
	}

	sendIR(index_id);

}
