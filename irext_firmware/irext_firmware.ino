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
#include "./include/ir_decode.h"
#include <PubSubClient.h>

// Define Function Prototypes that use User Types below here or use a .h file
//
#define MAX_PACKETSIZE 512 //定义UDP包的最大值512个字节
#define SERIAL_DEBUG Serial
#define UNIT8 unsigned char

WiFiUDP udp;
char buffUDP[MAX_PACKETSIZE]; //定义UDP缓冲区
DynamicJsonBuffer jsonBuffer;
JsonObject& downloadPost = jsonBuffer.createObject();
JsonObject& loginPost = jsonBuffer.createObject();
WiFiClient espClient;
PubSubClient client(espClient);

// 此为测试配置，应用app端数据替换
const char* ssid = "Tenda_049F28";
const char* password = "";
const char* mqttServer = "m10.cloudmqtt.com";
const int mqttPort = 11327;
const char* mqttUser = "mqtt";
const char* mqttPassword = "mqtt";

static t_remote_ac_status ac_status =
{
	AC_POWER_OFF,
	AC_TEMP_24,
	AC_MODE_COOL,
	AC_SWING_ON,
	AC_WS_AUTO,
	0,
	0,
	0
};
// Define Functions below here or use other .ino or cpp files
int startUDPServer(uint16 port);
void sendUDP(const char *p);
boolean doUDPServerTick();
boolean appLogin();
boolean downloadFile();
int showFileData(const char* filename);
void getIR(const char* filename);
void callback(char* topic, byte* payload, unsigned int length);


// The setup() function runs once each time the micro-controller starts
void setup()
{
	SERIAL_DEBUG.begin(115200);
	SERIAL_DEBUG.println("Start module");
	WiFi.mode(WIFI_STA); //将此 Wifi 芯片设为站点模式
						 // WiFi.beginSmartConfig();
	WiFi.begin(ssid); //接入到无线环境热点
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

	loginPost["appKey"] = "4279187e58326959f1bc047f7900b4ee";
	loginPost["appSecret"] = "157f29992370f02043aca66893716be9";
	loginPost["appType"] = "2";
	
}

// Add the main program code into the continuous loop() function
void loop() {

	client.loop();

	if (doUDPServerTick() == true) {
		SERIAL_DEBUG.print("Recieve: ");
		SERIAL_DEBUG.println(buffUDP);
		String msg = buffUDP;
		int i = msg.toInt();

		if (msg.equals("ir") == true) {
			getIR("/download.bin");
		}
		else if (i > 0 && i < 100000) {
			if (appLogin() == true) {
				if (downloadFile() == true) {
					sendUDP("ok");
				}
				else {
					SERIAL_DEBUG.println("下载文件失败");
				}
				delay(1000);
			}
			else {
				SERIAL_DEBUG.println("换取id token失败");
			}
		}
		else if (msg.equals("data")) {
			showFileData("/download.bin");
		}
		else if (msg.equals("info")) {
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
	}
	delay(10);
}

/**
* 打开UDPserver端
* 成功返回1，失败返回0
*/
int startUDPServer(uint16 port) {
	SERIAL_DEBUG.print("StartUDPServer at port: ");
	SERIAL_DEBUG.println(port);
	return udp.begin(port);
}


/**
* 发送UDP包
*/
void sendUDP(const char *p) {
	udp.beginPacket(udp.remoteIP(), udp.remotePort());
	udp.write(p);
	udp.endPacket();
}


/**
* 监听udp消息并将其缓存在buffUDP中
* 成功返回true, 否则返回false
*/
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

/**
* 换取id、token
* 成功返回true， 否则false
*/
boolean appLogin() {

	HTTPClient http;
	char body[512];
	const char* login_url = "http://irext.net/irext-server/app/app_login";
	SERIAL_DEBUG.println("try to login...");

	if (WiFi.status() == WL_CONNECTED) {
		memset(body, 0x00, sizeof(body));
		loginPost.printTo(body, sizeof(body));
		SERIAL_DEBUG.println("Post Message:");
		SERIAL_DEBUG.println(body);
		http.begin(login_url);
		http.addHeader("Content-Type", "application/json");
		int httpCode = http.POST((uint8_t *)body, strlen(body));
		SERIAL_DEBUG.printf("[HTTP] POST... Code: %d\n", httpCode);

		if (httpCode > 0) {
			String payload = http.getString();
			JsonObject& response = jsonBuffer.parseObject(payload);
			if (response == JsonObject::invalid()) {
				SERIAL_DEBUG.println("JsonObject invalid");
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
			http.end();
			return true;
		}
		else {
			http.end();
			return false;
		}
	}
	else {
		return false;
	}
}


/**
* 下载二进制文件
* 下载成功返回true, 否则false
*/
boolean downloadFile() {

	HTTPClient http;
	char body[512];
	const char* download_url = "http://irext.net/irext-server/operation/download_bin";
	SERIAL_DEBUG.printf("try to download indexId: %s\n", buffUDP);
	if (WiFi.status() == WL_CONNECTED) {

		// Prepare cache file
		File cache = SPIFFS.open("/download.bin", "w");
		File* filestream = &cache;
		if (!cache) {
			SERIAL_DEBUG.println("Could not create cache file");
		}
		SERIAL_DEBUG.println("Open download.bin right");
		downloadPost["indexId"] = buffUDP;
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


/**
* 显示文件信息
*/
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


/**
* IR解码
* 成功返回外码时序码，否则返回NULL
*/
void getIR(const char* filename) {
	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open(filename, "r");
		File *fp = &f;
		if (f) {
			
			UINT16 content_length = f.size();
			SERIAL_DEBUG.printf("content_length = %d\n", content_length);
			UINT8 *content = (UINT8 *)malloc(content_length * sizeof(UINT8));
			f.seek(0L, fs::SeekSet);
			f.readBytes((char*)content, content_length);
			for (int i = 0; i < content_length; i++) {
				SERIAL_DEBUG.print((char)content[i]);
			}
			INT8 ret = ir_binary_open(IR_CATEGORY_AC, 1, content, content_length);
			SERIAL_DEBUG.printf("ret = %d\n", ret);
			free(content);
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


/**
* mqtt回调函数
*/
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

