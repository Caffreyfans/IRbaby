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

char buffUDP[MAX_PACKETSIZE];
WiFiUDP udp;
DynamicJsonBuffer jsonBuffer;
JsonObject& downloadPost = jsonBuffer.createObject();
WiFiClient espClient;
PubSubClient client(espClient);
UINT16 user_data[USER_DATA_SIZE];
IRsend irsend(IR_LED);

// 测试数据
const char* mqttServer = "caffreyfans.top";
const int mqttPort = 1883;
const char* mqttUser = "mqtt";
const char* mqttPassword = "mqtt";

static t_remote_ac_status ac_status =
{
	// 默认空调状态
	AC_POWER_OFF,
	AC_TEMP_24,
	AC_MODE_COOL,
	AC_SWING_ON,
	AC_WS_AUTO,
	0,
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
说明：账号登录
返回值：成功返回true, 否则返回false
*/
boolean appLogin();


/**
说明：获取遥控码列表
返回值：成功返回true, 失败返回false
*/
boolean getList(JsonObject& json_object);



/**
说明：下载文件
参数：文件id
返回值：成功返回true, 否则返回false
*/
boolean downloadFile(int index_id);


/**
说明：显示文件内容
参数：文件路径
返回值：成功返回1， 失败返回0
*/
int showFileData(const char* filename);


/**
说明：显示内存信息
*/
void showMemoryInfo();


/**
说明：发射红外信号
参数：
返回值：成功返回true, 失败返回false
*/
boolean sendIR(const char* filename);


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
	startUDPServer(8000);
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
	client.publish("esp/test", "Hello from ESP8266");
	client.subscribe("esp/test");


}

// Add the main program code into the continuous loop() function
void loop() {

	client.loop();

	if (doUDPServerTick() == true) {
		String msg = buffUDP;
		SERIAL_DEBUG.println(msg);
		int index_id = msg.toInt();

		if (msg.equals("ir") == true) {
			sendIR("/download.bin");
		}
		else if (index_id > 0 && index_id < 100000) {
			for (int i = 0; i < TRY_COUNT; i++) {
				if (downloadFile(index_id) == true)	break;
			}
		}
		else if (msg.equals("data")) {
			showFileData("/download.bin");
		}
		else if (msg.equals("info")) {
			showMemoryInfo();
		}
		else {
			JsonObject& messageJson = jsonBuffer.parseObject(msg);
			getList(messageJson);
		}
	}
	delay(10);
}



uint8_t startUDPServer(uint16 port) {
	SERIAL_DEBUG.print("StartUDPServer at port: ");
	SERIAL_DEBUG.println(port);
	return udp.begin(port);
}



int sendUDP(const char *p) {
	udp.beginPacket(udp.remoteIP(), udp.remotePort());
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



boolean appLogin() {
	JsonObject& loginPost = jsonBuffer.createObject();
	loginPost["appKey"] = "4279187e58326959f1bc047f7900b4ee";
	loginPost["appSecret"] = "157f29992370f02043aca66893716be9";
	loginPost["appType"] = "2";

	HTTPClient http;
	char body[256];
	const char* login_url = "http://irext.net/irext-server/app/app_login";
	SERIAL_DEBUG.println("try to login...");
	boolean flag = true;

	if (WiFi.status() == WL_CONNECTED) {
		memset(body, 0x00, sizeof(body));
		loginPost.printTo(body, sizeof(body));
		SERIAL_DEBUG.println("Post Message:");
		SERIAL_DEBUG.println(body);
		http.begin(login_url);
		http.addHeader("Content-Type", "application/json");
		int httpCode = http.POST((uint8_t *)body, strlen(body));
		SERIAL_DEBUG.printf("[HTTP] POST... Code: %d\n", httpCode);

		if (httpCode == HTTP_CODE_OK) {
			String payload = http.getString();
			JsonObject& response = jsonBuffer.parseObject(payload);
			if (response == JsonObject::invalid()) {
				SERIAL_DEBUG.println("JsonObject invalid");
				flag = false;
			}
			else {
				int id = response["entity"]["id"];
				String token = response["entity"]["token"];
				downloadPost["id"] = id;
				downloadPost["token"] = token;
				SERIAL_DEBUG.println("Get the id and token:");
				SERIAL_DEBUG.println(id);
				SERIAL_DEBUG.println(token);
			}
		}
		else {
			flag = false;
		}
	}
	else {
		flag = false;
	}
	http.end();
	return flag;
}


boolean getList(JsonObject& json_object) {
	const char* getlist_url = "http://irext.net/irext-server/indexing/list_indexes";
	char body[256];


	if (WiFi.status() == WL_CONNECTED) {
		//while (appLogin() != true) {
		//	delay(10);
		//}
		//json_object["id"] = downloadPost["id"];
		//json_object["token"] = downloadPost["token"];
		HTTPClient http;
		http.begin(getlist_url);
		http.addHeader("Content-Type", "application/json");
		memset(body, 0x00, sizeof(body));
		json_object.printTo(body, sizeof(body));
		int http_code = http.POST((uint8_t *)body, strlen(body));
		String payload = http.getString();
		SERIAL_DEBUG.println(payload);

		if (http_code == HTTP_CODE_OK) {
			JsonObject& json_object = jsonBuffer.parseObject(payload);
			SERIAL_DEBUG.println(json_object["entity"].size());
		}
		else {
			SERIAL_DEBUG.printf("httpCode = %d\n", http_code);
			
		}
		http.end();
	}
	else {
		return false;
	}
}


boolean downloadFile(int index_id) {
	HTTPClient http;
	char body[256];
	const char* download_url = "http://irext.net/irext-server/operation/download_bin";
	SERIAL_DEBUG.printf("try to download indexId: %d\n", index_id);

	if (WiFi.status() == WL_CONNECTED) {

		if (appLogin() != true) {
			return false;
		}

		// Prepare cache file
		String filename = String(index_id);
		File cache = SPIFFS.open(filename, "w");
		File* filestream = &cache;
		if (!cache) {
			SERIAL_DEBUG.println("Could not create cache file");
			return false;
		}
		SERIAL_DEBUG.println("Open download.bin right");
		downloadPost["indexId"] = index_id;
		memset(body, 0x00, sizeof(body));
		downloadPost.printTo(body, sizeof(body));

		http.begin(download_url);
		http.addHeader("Content-Type", "application/json");
		int httpCode = http.POST((uint8_t *)body, strlen(body));
		SERIAL_DEBUG.println("Try to post:");
		SERIAL_DEBUG.println(body);
		SERIAL_DEBUG.print("[HTTP] POST return code: ");
		SERIAL_DEBUG.println(httpCode);
		http.writeToStream(filestream);
		cache.close();
		http.end();
		return true;
	}
	else {
		SERIAL_DEBUG.println("Error in WiFi connection");
		return false;
	}
}



int showFileData(const char* filename) {

	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open(filename, "r");
		int s = f.size();
		String data = f.readString();
		SERIAL_DEBUG.printf("open \"%s\" right the Size = %d\n", filename, s);
		SERIAL_DEBUG.println("\n************* data of file: *************");
		SERIAL_DEBUG.println(data);
		SERIAL_DEBUG.println("*************  end of data  **************\n");
		f.close();

		return s;
	}
	else {
		SERIAL_DEBUG.println("download.bin is not exsited");
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



boolean sendIR(const char* filename) {
	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open(filename, "r");
		File *fp = &f;
		if (f) {
			UINT16 content_length = f.size();
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
		}
		else {
			SERIAL_DEBUG.printf("open %s was failed\n", filename);
		}
		f.close();
	}
	else {
		SERIAL_DEBUG.printf("%s is not exsits\n", filename);
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

}
