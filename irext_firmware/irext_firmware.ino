// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       irext_firmware.ino
	Created:	2018/6/14 15:54:10
	Author:     Caffreyfans
*/

// Define User Types below here or use a .h file
//
#include <esp8266wifi.h>
#include <WiFiUdp.h>
#include "ArduinoJson.h"
#include "ESP8266HTTPClient.h"
#include <FS.h>
#include "ir_decode.h"
#include "String.h"
// Define Function Prototypes that use User Types below here or use a .h file
//
#define MAX_PACKETSIZE 512 //定义UDP包的最大值512个字节
#define SERIAL_DEBUG Serial

WiFiUDP udp;
char buffUDP[MAX_PACKETSIZE];	//定义UDP缓冲区
uint8_t buffer[5840];
DynamicJsonBuffer jsonBuffer;
JsonObject& downloadPost = jsonBuffer.createObject();
JsonObject& loginPost = jsonBuffer.createObject();

// Define Functions below here or use other .ino or cpp files

/**
* 打开UDPserver端
* 成功返回1，失败返回0
*/
int startUDPServer(uint16 port) {
	SERIAL_DEBUG.print("StartUDPServer at port ");
	SERIAL_DEBUG.print(port);
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
		SERIAL_DEBUG.print("Received packet of size");
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
boolean downLoadFile() {

	HTTPClient http;
	char body[512];
	const char* download_url = "http://irext.net/irext-server/operation/download_bin";
	SERIAL_DEBUG.printf("try to download indexId: %s\n", buffUDP);
	if (WiFi.status() == WL_CONNECTED) {

		// Prepare cache file
		File cache = SPIFFS.open("download.bin", "w");
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


// 显示文件大小
int showFileSize(char* filename) {

	if (SPIFFS.exists(filename)) {
		File f = SPIFFS.open("download.bin", "r");
		int s = f.size();
		SERIAL_DEBUG.printf("download.bin Size=%d\n", s);
		f.close();
		return s;
	}
	else {
		SERIAL_DEBUG.println("download.bin is not exsited");
		return 0;
	}
}

void getIR() {
	INT8 file_open = ir_file_open(1, 1, "download.bin");
	SERIAL_DEBUG.print("open file return code: ");
	SERIAL_DEBUG.println(file_open);
	UINT16 decoded[1024];
	//t_remote_ac_status ac_status;
	//ac_status.ac_power = AC_POWER_ON;
	//ac_status.ac_temp = AC_TEMP_24;
	//ac_status.ac_mode = AC_MODE_COOL;
	//ac_status.ac_wind_dir = AC_SWING_ON;
	//ac_status.ac_wind_speed = AC_WS_AUTO;
	// UINT16 length = ir_decode(2, decoded, NULL, 0);
	//ir_close();
	//for (int i = 0; i <length; i++) {
	//	SERIAL_DEBUG.print(decoded[i]);
	//}
}


// The setup() function runs once each time the micro-controller starts
void setup()
{
	SERIAL_DEBUG.begin(115200);
	SERIAL_DEBUG.println("Start module");
	WiFi.mode(WIFI_STA);
	WiFi.beginSmartConfig();
	while (1) {
		SERIAL_DEBUG.print(".");
		delay(500);
		if (WiFi.smartConfigDone()) {
			SERIAL_DEBUG.println("\r\nSmartConfig Success");
			SERIAL_DEBUG.printf("SSID:%s\r\n", WiFi.SSID().c_str());
			SERIAL_DEBUG.printf("PSW:%s\r\n", WiFi.psk().c_str());
			break;
		}
	}
	startUDPServer(8000);
	SPIFFS.begin();

	loginPost["appKey"] = "4279187e58326959f1bc047f7900b4ee";
	loginPost["appSecret"] = "157f29992370f02043aca66893716be9";
	loginPost["appType"] = "2";
}

// Add the main program code into the continuous loop() function
void loop()
{
	if (doUDPServerTick() == true) {
		SERIAL_DEBUG.print("Recieve: ");
		SERIAL_DEBUG.println(buffUDP);
		String str = buffUDP;
		int i = str.toInt();
		
		if (str.compareTo("ir") == 0) {
			getIR();
		}
		else if (i>0 && i<10000){
			if (appLogin() == true) {
				downLoadFile();
				showFileSize("download.bin");
				delay(1000);
			}
		}
	}
	delay(1);
}
