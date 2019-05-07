# -*- encoding: utf-8 -*-
""" 
Description: python client for IRmqtt
Email: Caffreyfans@163.com
Date: 2019/4/27
"""

import socket
import json
import time
import threading
import queue
import sys
import requests
import os


queue_msg = queue.Queue(maxsize=1000)   # 全局消息队列


class udpUtils:
    """udp 报文发送、监听工具类
    Attributes:
        _thread: udp 报文接收线程
    """

    def __init__(self):
        """初始化 udpUtils """
        self._local_ip = self.get_local_ip()
        self._thread = None
        self._port = 4210

    def send(self, msg, addr):
        """向 addr 发送 msg 报文"""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        addr_port = (addr, self._port)
        msg = json.dumps(msg)
        sock.sendto(msg.encode('utf-8'), addr_port)
        sock.close()

    def recive(self, local_ip, queue_msg):
        """接收本地 4210 端口报文，并将报文加入到全局信息队列"""
        ip_port = (local_ip, self._port)
        server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        server.bind(ip_port)
        while True:
            data, client_addr = server.recvfrom(1024)
            receive_msg = bytes.decode(data)
            queue_msg.put(receive_msg)

    def get_local_ip(self):
        """获取本机 ip 并返回"""
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        finally:
            s.close()
        return str(ip)

    def start_thread(self):
        """开始 udp 接收线程"""
        self._thread = threading.Thread(target=self.recive, \
            args=(self._local_ip, queue_msg))
        self._thread.setDaemon(True)
        self._thread.start()

    def stop_thread(self):
        """结束 udp 接收线程"""
        self._thread._stop()


class Irext:
    """Irext 工具类
    Attributes:
        这里 Irext SDK 账户，用于通过认证，只有认证通过换取到 token
        后才能获取红外码信息，这不会涉及到您的隐私问题，如果你还是不放心
        请到 https://irext.net/sdk/ 申请自己的账户进行替换
    """

    def __init__(self, device_name, device_ip):
        """Irext 初始化"""
        self._auther = None
        self.app_login('e984291b9ae0ad2f1c7bbc9dc86518c8', \
            '4f9697c87d75ebaf2522234c72410398')
        self._brand_id = None
        self._category_id = 1
        self._device_ip = device_ip
        self._device_name = device_name
        self._index_id = None
        self._udp_client = udpUtils()

    def parse_ac(self):
        """匹配空调"""
        index_id = self.list_brands(self._category_id)
        index_list = self.list_indexes(self._category_id, index_id)
        i = 0
        while True:
            print('# # # # # # # # # # # # # # # # # # # # # # # #')
            print("【空调匹配】")
            print('''注意：选择【信号发射测试】之前请确保已经设置好红
外发射引脚,并且红外发射管已经与模块连接好。信号
发射之后空调若有响应请再选择【匹配成功】进行确认\n''')
            print("共为你找到 {} 个可用类型，当前正在使用第 {} 个".format(len(index_list), i + 1))
            print("[1] 信号发射测试")
            print("[2] 匹配成功")
            print("[3] 试试下一个")
            print("[4] 试试上一个")
            print("[5] 退出匹配")
            try:
                choice = int(input("所以现在你选择："))
                if choice < 1 or choice > 5:
                    print('输入有误请重新输入')
                    continue
            except:
                print('输入有误请重新输入')
                continue

            data = {}
            data['ir'] = {}
            data['ir']['send'] = {}
            if choice == 1:
                data['ir']['send']['index_id'] = index_list[i] + ".bin"
                data['ir']['send']['device_type'] = "ac"
                data['ir']['send']['test'] = True
                self._udp_client.send(data, self._device_ip)
            elif choice == 2:
                data['ir']['send']['index_id'] = index_list[i] + ".bin"
                data['ir']['send']['device_type'] = "ac"
                data['ir']['send']['test'] = False
                self._udp_client.send(data, self._device_ip)
                print('请将以下内容复制到 HomeAssistant 配置文件中')
                print()
                print(''' 
climate:
  - platform: mqtt
    name: AC
    modes:
        - "cool"
        - "heat"
        - "auto"
        - "fan"
        - "dry"
        - "off"
    swing_modes:
        - "on"
        - "off"
    fan_modes:
        - "high"
        - "medium"
        - "low"
        - "auto"
    mode_command_topic: "/IRmqtt/{0}/ac/{1}/mode"
    temperature_command_topic: "/IRmqtt/{0}/ac/{1}/temperature"
    fan_mode_command_topic: "/IRmqtt/{0}/ac/{1}/fan"
    swing_mode_command_topic: "/IRmqtt/{0}/ac/{1}/swing"
    min_temp: 16
    max_temp: 30                
                '''.format(self._device_name, data['ir']['send']['index_id']))
                input('请按回车继续...')
            elif choice == 3:
                i += 1
                if i > len(index_list) - 1:
                    i = 0
                continue
            elif choice == 4:
                i -= 1
                if i < 0:
                    i = len(index_list) - 1
                continue
            elif choice == 5:
                break
            else:
                continue
        # print("抱歉没有适合你的类型")

    def app_login(self, app_key, app_secret):
        """ 换取 Irext token """
        url = 'https://irext.net/irext-server/app/app_login'
        headers = {'Content-Type':'application/json'}
        data = {'appKey':app_key, 'appSecret':app_secret, 'appType':'2'}
        response = requests.post(url=url, data=json.dumps(data), headers=headers)
        # print(response.text)
        ret_json = json.loads(response.text)
        auther = {'id':ret_json['entity']['id'], 'token':ret_json['entity']['token'] }
        self._auther = auther

    def list_categories(self):
        """列出电器种类

        Returns:
            Irext 中可用的电器种类
            example:

            ['空调','扫地机器人','电视盒子']
        """
        url = 'https://irext.net/irext-server/indexing/list_categories'
        headers = {'Content-Type':'application/json'}
        data = {'id':self._auther['id'], 'token':self._auther['token'], \
            'from':'0', 'count':'20'}
        response = requests.post(url=url, data=json.dumps(data), headers=headers)
        ret_json = json.loads(response.text)
        categories_list = []
        print('获得家电类型列表: ')
        for i in ret_json['entity']:
            print("{}.{}".format(i['id'], i['name']), end='  ')
            categories_list.append(i)
        print()
        
    def list_brands(self, category_id=1):
        """根据电器种类列出品牌列表
        Args:
            category_id: 电器种类代码

        Returns:
            用户选择的电器品牌代码
            example:
            
            1
        """
        url = 'https://irext.net/irext-server/indexing/list_brands'
        headers = {'Content-Type':'application/json'}
        data = {'id':self._auther['id'], 'token':self._auther['token'], \
            'categoryId':category_id, 'from':'0', 'count':'2000'}
        response = requests.post(url=url, data=json.dumps(data), headers=headers)
        ret_json = json.loads(response.text)
        id_list = []
        print('获得家电品牌列表: ')
        while True:
            for i in ret_json['entity']:
                print("{}.{}".format(i['id'], i['name']), end='  ')
                id_list.append(int(i['id']))
            print()
            try:
                index_id = int(input('请选择空调品牌: '))
                if index_id in id_list:
                    return index_id
                else:
                    print('不存在该选项')
                    continue
            except:
                pass
            

    def list_indexes(self, category_id, brand_id):
        """根据电器种类以及品牌列出可用文件

        Args:
            cateroty_id: 电器种类代码
            brand_id: 电器品牌代码

        Returns:
            可以文件列表
            example:

            ['new_ac_5227', 'new_ac_5282', 'new_ac_2807', 'new_ac_3387']
        """

        url = 'https://irext.net/irext-server/indexing/list_indexes'
        headers = {'Content-Type':'application/json'}
        data = {'id':self._auther['id'], 'token':self._auther['token'], \
            'categoryId':category_id, 'brandId':brand_id, 'from':'1', 'count':'2000'}
        response = requests.post(url=url, data=json.dumps(data), headers=headers)
        # print('获得下载 列表: ', end='\n')
        ret_json = json.loads(response.text)
        indexs_list = []
        for i in ret_json['entity']:
            # print(i['remoteMap'], end=' ')
            indexs_list.append(i['remoteMap'])
        return indexs_list


class IRmqttDevice:
    """IRmqttDevice 管理类
    对模块进行红外接收、发射引脚的设置，连接 MQTT 服务器设置
    收录自定义红外码
    """

    def __init__(self):
        """IRmqttDevice 初始化"""
        self._udp_client = udpUtils()
        self._device_ip = None
        self._device_name = None
        self._esp_list = {}
        self._recv_pin = None
        self._send_pin = None
        self._pin_list = ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 
                        'D6', 'D7', 'D8', 'D9', 'D10',
                        '16', '5', '4', '0', '2', 
                        '14', '12', '13', '15', '3', '1']
        self._pin_cover = {'D0':'16', 'D1':'5', 'D2':'4', 'D3':'0',
                        'D4':'2', 'D5':'14', 'D6':'12', 'D7':'13',
                        'D8':'15', 'D9':'3', 'D10':'1'}

    def discover_devices(self):
        """发现在线设备"""
        local_ip = self._udp_client.get_local_ip()
        data = {}
        data["upload_ip"] = local_ip
        time_start = time.time()
        time_end = time.time()
        print('正在发现设备:', end='')
        while time_end - time_start < 1:
            self._udp_client.send(data, addr = '<broadcast>')
            esp_list = queue_msg.get()
            esp_list_json = json.loads(esp_list)
            if 'ret' in esp_list_json:
                esp_list_json = esp_list_json['ret']
                if 'device' in esp_list_json:
                    esp_list_json = esp_list_json['device']
                    for key in esp_list_json:
                        self._esp_list[key] = esp_list_json[key]
            time_end = time.time() 
            time.sleep(1/10)
            print('.', end='')
        while not queue_msg.empty():
            queue_msg.get()
        print()

        k = 0
        while True:
            try:
                i = 1
                for key in self._esp_list:
                    print("[{}] 设备名:{}, IP:{}".format(i, key, self._esp_list[key]))
                    i += 1                
                k = int(input('请选择你将要操作模块：'))
                if k < 1 or k > len(self._esp_list):
                    continue
                break
            except:
                pass
        i = 1
        for key in self._esp_list:
            if k == i:
                self._device_ip = self._esp_list[key]
                self._device_name = key
                break
            i += 1

    def set_recv_pin(self):
        """设置红外接收引脚"""

        data_json = {}
        data_json['save'] = {}
        while True:
            print("【红外接收引脚设置】")
            print("注意：请对照可用引脚进行选择")
            print(self._pin_list)
            data_json['save']['recv_pin'] = str(input('设置引脚: ')).upper()
            if data_json['save']['recv_pin'] not in self._pin_list:
                print('引脚设置错误请重新选择')
                print('# # # # # # # # # # # # # # # # # # # # # # # #')
                continue
            else:
                try:
                    int(data_json['save']['recv_pin'])
                except:
                    data_json['save']['recv_pin'] = self._pin_cover[data_json['save']['recv_pin']]
                self._udp_client.send(data_json, self._device_ip)
                print("设置成功")
                break
        
    def set_mqtt(self):
        """设置 MQTT 服务器"""

        mqtt_json = {}
        mqtt_json['save'] = {}
        mqtt_json['save']['mqtt'] = {}
        print("【MQTT 连接设置】")
        mqtt_json['save']['mqtt']['host'] = input("请输入服务器地址（默认：hassio）:")
        mqtt_json['save']['mqtt']['port'] = input("请输入服务器端口（默认：1883 ）:")
        mqtt_json['save']['mqtt']['user'] = input("用户名: ")
        mqtt_json['save']['mqtt']['password'] = input("密码: ")
        if mqtt_json['save']['mqtt']['host'] == '':
            mqtt_json['save']['mqtt']['host'] = 'hassio'
        if mqtt_json['save']['mqtt']['port'] == '':
            mqtt_json['save']['mqtt']['port'] = '1883'
        self._udp_client.send(mqtt_json, self._device_ip)
        time_start = time.time()
        time_end = time.time()
        print("连接中请稍后:", end='')
        connect_status = '连接失败'
        while time_end - time_start < 2:
            print('.', end='')
            try:
                ret_json = json.loads(queue_msg.get(timeout=1/10))
                if 'ret' in ret_json:
                    if ret_json['ret']['mqtt'] == True:
                        connect_status = '连接成功'
                        break
            except:
                pass
            time_end = time.time()
        print(connect_status)

    def set_send_pin(self):
        """设置红外发射引脚"""

        data = {}
        data['save'] = {}
        while True:
            print("【红外发射引脚设置】")
            print("注意：请对照可用引脚进行选择")
            print(self._pin_list)
            data['save']['send_pin'] = str(input('设置引脚: ')).upper()
            if data['save']['send_pin'] not in self._pin_list:
                print('引脚设置错误请重新选择')
                print('# # # # # # # # # # # # # # # # # # # # # # # #')
                continue
            else:
                try:
                    int(data['save']['send_pin'])
                except:
                    data['save']['send_pin'] = self._pin_cover[data['save']['send_pin']]                
                self._udp_client.send(data, self._device_ip)
                print("设置成功")
                break
        
    def recv_ir(self):
        """收录自定义红外码"""
        data = {}
        data['ir'] = {}
        data['ir']['recv'] = {}

        while True:
            print("【自定义红外码录制】")
            print('注意：使用该功能之前请确认红外接收引脚已设定好\n')
            data['ir']['recv'].clear()
            print('[1] 录制红外信号')
            print('[2] 发送刚才接收到的信号')
            print('[3] 保存信号')
            print('[4] 返回主菜单')
            choice = int(input('请输入选项: '))
            if choice == 1:
                print('请对着红外接收头按下您的按键>>> ', end='')
                data['ir']['recv']['recorder'] = True
                self._udp_client.send(data, self._device_ip)
                try:
                    msg = queue_msg.get()
                    msg = json.loads(msg)
                except:
                    continue
                if msg['ret']['ir']['get_raw']:
                    print('红外信号已接收')
                else:
                    print('未接收到红外信号')
            if choice == 2:
                data['ir']['recv']['send'] = True
                self._udp_client.send(data, self._device_ip)
            if choice == 3:
                filename = str(input('请输入保存名: '))
                data['ir']['recv']['save'] = filename
                self._udp_client.send(data, self._device_ip)
                print('topic\t\t\t\tpayload')
                print('/IRmqtt/{}/custom\t\t{}'.format(self._device_name, data['ir']['recv']['save']))
            if choice == 4:
                break


    def parse_ac(self):
        irext = Irext(self._device_name, self._device_ip)
        irext.parse_ac()


class Menu:

    def __init__(self):
        self._irmqtt_device = IRmqttDevice()
        self._udp_client = udpUtils()
        self._udp_client.start_thread()
        self._choice_dict = None
        self.createChoice()
        self.display_logo()
        self._irmqtt_device.discover_devices()
        self.run(self._choice_dict, self._choice_dict)

    def createChoice(self):
        self._choice_dict = {
            ("1", "设置红外发射引脚"):self._irmqtt_device.set_send_pin,
            ("2", "MQTT 连接设置"):self._irmqtt_device.set_mqtt,
            ("3", "匹配电器(当前只支持空调)"):self._irmqtt_device.parse_ac,
            ("4", "设置红外接收引脚"):self._irmqtt_device.set_recv_pin,
            ("5", "录制自定义红外码"):self._irmqtt_device.recv_ir,
            ("6", "退出程序"):self.close
        }

    def display_logo(self):
        print('''
# # # # # # # # # # # # # # # # # # # # # # # #
#    _____ _____                  _   _       #  
#   |_   _|  __ \                | | | |      # 
#     | | | |__) |_ __ ___   __ _| |_| |_     #
#     | | |  _  /| '_ ` _ \ / _` | __| __|    #
#    _| |_| | \ \| | | | | | (_| | |_| |_     #
#   |_____|_|  \_\_| |_| |_|\__, |\__|\__|    #
#                              | |            #
#                              |_|            #
# # # # # # # # # # # # # # # # # # # # # # # #  
''')

    def display_help(self):
        print('''
使用:
      1. [ h ]  -- 显示帮助信息.
      2. [ 1 | 2 | ... ] -- 输入数字选择对应项.
      3. [ b ]  -- "b" 返回上一级菜单.
      4. [ q | quit | exit ] -- 退出程序.
''')

    def run(self, value, recoder):
        while True:
            # self.clear()
            print('# # # # # # # # # # # # # # # # # # # # # # # #')
            print('【主菜单】')
            for (k, v) in value.items():
                print("[{}] {}".format(k[0], k[1]))
            choice = input("请输入选项: ")
            for (k, v) in value.items():
                if choice == k[0]:
                    print('# # # # # # # # # # # # # # # # # # # # # # # #')
                    v()

    def close(self):
        sys.exit(0)

    def clear(self):
        if os.name == 'nt':
            os.system('cls')
        self.display_logo()
def main():
    menu = Menu()


if __name__ == '__main__':
    main()