#!/bin/env python
# -*- coding: utf-8 -*-

'''
Author: Spike Lin(spikelin@tencent.com)
Version: v1.0.0
Date： 2019-05-29
'''

######################## README ###################################
# 腾讯云IoT AT指令模组python测试工具说明：
#
# 1. 使用环境：基于python3和pyserial模块
#   pip install pyserial
#
# 2. 需要先在腾讯云物联平台创建设备，目前仅支持PSK加密方式的设备：
#   Product_ID / Device_Name / Device_Key
#
# 3. 测试模式说明: CLI/HUB/IE/WIFI/OTA
# CLI:  原始AT命令行模式
# HUB： IoT Hub测试，在Hub控制台创建产品并替换下面代码部分的Hub_Product_ID等设备信息变量
#       循环模式下，会以QoS1循环收发指定数量的报文，并统计成功率和测试时间
# IE：  IoT Explorer测试，在Explorer控制台创建产品并将下面的IE_Product_ID等设备信息变量修改
#       循环模式下，可以循环输入命令以进行发送template或event消息的测试
# WIFI: WIFI配网测试，须结合app进行。配网成功，则进入IoT Explorer循环测试模式
# OTA： 可以进行Hub设备的OTA AT测试，并将成功获取到的固件保存到本地文件
#
# 4. AT模组硬件:
# 已验证模组包括乐鑫ESP8266 WiFi模组及有方NW-N21 NB-IOT模组
# 如新的模组在连接网络的指令、转义字符处理及AT指令长度限制等等不一致，则需要增加定制化代码
# 具体可以参考下面代码里面跟模组关键字如ESP8266或NW-N21有关的部分进行处理
#
# 5. 使用示例：
# 工具help信息：
#   python QCloud_AT_cmd_test_tool.py -h
#
# 对连接到串口COM5的NW-N21模组进行IoT Hub平台下的100次收发MQTT消息循环测试：
#   python QCloud_AT_cmd_test_tool.py --port COM5 --module NW-N21 --mode HUB --loop_cnt 100
#
# WiFi模组配网：
#   python QCloud_AT_cmd_test_tool.py --port COM5 --module ESP8266 --mode WIFI
######################## README ###################################

import threading
import serial
import sys
import argparse
import time
import queue
import json
import random
import hashlib

############################## CUSTOM PARAMETERS ################################

# IoT Explorer 测试 ############################################
# IoT Explorer 测试可根据需要修改以下设备信息及测试msg内容
IE_Product_ID = "YOUR_PRODUCT_ID"
IE_Device_Name = "YOUR_DEVICE_NAME"
IE_Device_Key = "YOUR_DEVICE_KEY"


def gen_template_update_msg():
    brightness = random.randint(1, 100)
    color = random.randint(0, 2)
    power_switch = random.randint(0, 1)
    print(">>> update data >>> brightness:", brightness, "color:", color, "power_switch:", power_switch)
    IEUpdateMsg = '''{	"type": "update",
        "state": {
            "reported": {
                "brightness": %d,
                "color": %d,
                "power_switch":%d
            }
        },
        "version":%d,
        "clientToken": "clientToken-%d" }''' \
                  % (brightness, color, power_switch, 0 , int(time.time()*10)%1000)
    return IEUpdateMsg


def gen_event_post_msg():
    voltage = random.uniform(1.0, 3.0)
    print(">>> post event >>> voltage:", voltage)
    IEEventMsg = '''{
        "method": "event_post",
        "version": "1.0",
        "eventId": "low_voltage",
        "params": {
            "voltage": %f
        },
        "clientToken": "clientToken-%d"}''' % (voltage, int(time.time()*10)%1000)

    return IEEventMsg

# IoT Hub 测试 ################################################
# IoT Hub 测试可根据需要修改以下设备信息及测试topic和msg内容
Hub_Product_ID = 'YOUR_PRODUCT_ID'
Hub_Device_Name = 'YOUR_DEVICE_NAME'
Hub_Device_Key = 'YOUR_DEVICE_KEY'

HubTestTopic = '''%s/%s/data''' % (Hub_Product_ID, Hub_Device_Name)
HubTestLongMsg = '1HELLO1234567890987654321world1234567890987654321QAZWSXEDCRFVTGBYHNUJMIKOPqazwsxedcrfvtgbyhnujmikolp' \
                 '2HELLO1234567890987654321world1234567890987654321QAZWSXEDCRFVTGBYHNUJMIKOPqazwsxedcrfvtgbyhnujmikolp' \
                 '3HELLO1234567890987654321world1234567890987654321QAZWSXEDCRFVTGBYHNUJMIKOPqazwsxedcrfvtgbyhnujmikolp' \
                 '4HELLO1234567890987654321world1234567890987654321QAZWSXEDCRFVTGBYHNUJMIKOPqazwsxedcrfvtgbyhnujmikolp' \
                 '5HELLO1234567890987654321world1234567890987654321QAZWSXEDCRFVTGBYHNUJMIKOPqazwsxedcrfvtgbyhnujmikolp'


def get_hub_test_msg():
    HubTestMsg = '''{"action":"publish_test","time":"%d"}''' % (int(time.time()))
    print(">>> publish test msg", HubTestMsg)
    return HubTestMsg

# WiFi 模组测试 #########################################################
# WiFi 模组测试可以指定要连接的WiFi路由器或热点的SSID和密码
WiFi_SSID = 'YOUR_WIFI_SSID'
WiFi_PSWD = 'YOUR_WIFI_PSW'

############################## CUSTOM PARAMETERS ################################

# AT模组 定制化代码 #####################################################
def esp_at_add_escapes(raw_str):
    return raw_str.replace('\"', '\\\"').replace(',', '\\,')

def nw_at_add_escapes(raw_str):
    return raw_str.replace('\"', '\\\"')

g_at_module_custom_params = {
    'ESP8266':{
        'add_escapes': esp_at_add_escapes,      # 添加转义字符处理方法
        'err_list': ['ERROR', 'FAIL', 'busy'],  # AT指令出错提示
        'cmd_timeout': 5,                       # AT指令执行超时限制，单位：秒
        'at_cmd_max_len': 254,                  # 单条AT指令最大长度，单位：字节
        'at_cmd_publ_max_payload': 2048},       # PUBL发送长消息最大长度，单位：字节
    'NW-N21':{
        'add_escapes': nw_at_add_escapes,
        'err_list': ['ERROR', 'FAIL'],
        'cmd_timeout': 10,
        'at_cmd_max_len': 1024,
        'at_cmd_publ_max_payload': 10240},
    }

# 模组串口配置，可以根据模组具体情况修改，使用的串口名字和波特率可以在程序启动参数指定
g_serial_port = 'COM8'
g_serial_baudrate = 115200
g_serial_bytesize = serial.EIGHTBITS
g_serial_parity = serial.PARITY_NONE
g_serial_stopbits = serial.STOPBITS_ONE
g_serial_timeout_s = 3


################# python AT cmd framework #########################

# 调试选项
g_debug_print = False


class Singleton(type):
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


class SerialATClient(threading.Thread, metaclass=Singleton):
    def __init__(self, raw_mode=False):
        # thread init
        super(SerialATClient, self).__init__()

        try:
            self.serial_port = serial.Serial(g_serial_port, g_serial_baudrate, g_serial_bytesize,
                                             g_serial_parity, g_serial_stopbits, g_serial_timeout_s)
        except serial.SerialException:
            print("!!! Serial port", self.port, "open error!!")
            sys.exit(-1)

        self.connected = True
        self.timeout = g_serial_timeout_s
        self.raw_mode = raw_mode
        self.port = g_serial_port
        self.data_handlers = dict()
        self.cmd_queue = queue.Queue()
        self.current_cmd = ""
        self.hex_data_handler = self.default_hexdata_handler
        print("Connected to serial port:", self.port)

    # start the thread to read data from serial port
    def start_read(self):
        if not self.connected:
            try:
                self.serial_port.open()
            except serial.SerialException:
                print("!!! Serial port", self.port, "open error!!")
                sys.exit(-1)

        self.connected = True
        if not self.is_alive():
            try:
                self.start()
            except RuntimeError:
                print("!!! Start thread failed!!")
                sys.exit(-1)

    def close_port(self):
        if not self.connected:
            return

        self.connected = False
        try:
            self.serial_port.flush()
            self.serial_port.close()
        except serial.SerialException:
            print("Serial port", self.port, "close error!")

        try:
            self.join(self.timeout)
        except RuntimeError:
            print("Join thread failed")

    def add_data_handler(self, key_str, handler_func):
        if not key_str or len(key_str.strip().encode('utf8')) == 0:
            return
        self.data_handlers[key_str] = handler_func

    def remove_data_handler(self, key_str):
        if not key_str or len(key_str.strip().encode('utf8')) == 0:
            return
        if key_str in self.data_handlers:
            del self.data_handlers[key_str]

    def run(self):
        while self.connected:
            try:
                data_raw = self.serial_port.read_until(bytes([13, 10]))  # \r\n
            except serial.SerialException:
                print("Serial port", self.port, "read error!")
                time.sleep(1)
                continue
            else:
                self.handle_recv_data(data_raw)

    def default_hexdata_handler(self, data_raw):
        data_str = "HEXDATA: "
        for i in range(len(data_raw)):
            # print(hex(data_raw[i])[2:])
            data_str += hex(data_raw[i])
            data_str += ' '
        print(data_str)

    def handle_recv_data(self, data_raw):
        if not data_raw:
            return

        try:
            data_str = data_raw.decode().strip('\r\n')
        except (TypeError, UnicodeDecodeError):
            self.hex_data_handler(data_raw)
            return

        # serial recv data handling
        if len(data_str) == 0:
            return

        if self.raw_mode:
            print(self.port, "RECV:", data_str)
            return

        if g_debug_print:
            print(self.port, "RECV:", data_str)

        for i in self.data_handlers:
            if i in data_str:
                self.data_handlers[i](data_str)
                return

        if len(self.current_cmd):
            self.cmd_queue.put(data_str)
            return

        print("Unhandled data:", data_str)

    def send_cmd(self, data):
        if not self.connected and not data:
            return False

        try:
            self.serial_port.write((data+"\r\n").encode('utf8'))
            return True
        except serial.SerialException:
            print("Serial port", self.port, "write error!")
        except TypeError:
            print("Data type error:", data)

        return False

    def send_cmd_wait_reply(self, cmd_str, ok_str, err_list, timeout=2):
        if not self.connected and not cmd_str:
            return False, ""

        if g_debug_print:
            print("Send CMD:", cmd_str)

        self.current_cmd = cmd_str
        ret = self.send_cmd(cmd_str)
        if not ret:
            self.current_cmd = ''
            return ret, "send cmd error"

        ret = False
        ret_str = ""

        while True:
            try:
                ans = self.cmd_queue.get(True, timeout)
                if ans == ok_str:
                    ret = True
                    break

                for i in err_list:
                    if i in ans:
                        ret_str += ans + '\n'
                        if g_debug_print:
                            print("error", i, "happen in", ans)

                        self.current_cmd = ''
                        return ret, ret_str

                if ans != cmd_str:
                    ret_str += ans + "\n"
                continue
            except queue.Empty:
                print("Cmd Queue Timeout")
                break
            except TypeError:
                print("Type error")
                break
            except KeyboardInterrupt:
                print("User interrupt")
                break

        self.current_cmd = ''
        return ret, ret_str

    def do_one_at_cmd(self, cmd_str, ok_str, hint, err_list, timeout):
        ret, reply = self.send_cmd_wait_reply(cmd_str, ok_str, err_list, timeout)
        if ret:
            print(hint, ">> OK")
            return True
        else:
            print(hint, ">> Failed")
            print(reply)
            return False

    def echo_off(self):
        cmd = "ATE0"
        ok_reply = 'OK'
        hint = "Echo off"

        return self.do_one_at_cmd(cmd, ok_reply, hint, ['ERROR', 'FAIL'], timeout=2)


def interactive_test():
    ser = SerialATClient(raw_mode=True)
    ser.start_read()
    while True:
        cmd = input("CMD:").strip('\n')
        if cmd.lower() == 'quit':
            ser.close_port()
            sys.exit(0)
        elif len(cmd) == 0:
            continue

        ser.send_cmd(cmd)
        time.sleep(1)


class IoTBaseATCmd:
    def __init__(self, at_module='ESP8266'):

        self.serial = SerialATClient()
        # register URC handlers
        self.serial.add_data_handler("+TCMQTTRCVPUB", self.mqtt_msg_handler)
        self.serial.add_data_handler("+TCMQTTDISCON", self.mqtt_state_handler)
        self.serial.add_data_handler("+TCMQTTRECONNECTING", self.mqtt_state_handler)
        self.serial.add_data_handler("+TCMQTTRECONNECTED", self.mqtt_state_handler)
        self.topic_handlers = dict()
        self.ota_state = 'off'
        self.at_module = at_module.upper()

        self.cmd_timeout = g_at_module_custom_params[self.at_module]['cmd_timeout']
        self.at_cmd_max_len = g_at_module_custom_params[self.at_module]['at_cmd_max_len']
        self.at_cmd_publ_max_payload = g_at_module_custom_params[self.at_module]['at_cmd_publ_max_payload']
        self.err_list = g_at_module_custom_params[self.at_module]['err_list']

    def add_escapes(self, raw_str):
        return g_at_module_custom_params[self.at_module]['add_escapes'](raw_str)

    def mqtt_msg_handler(self, urc_msg):
        try:
            msg = urc_msg.split(':', 1)[1].split(',', 2)
            topic = msg[0].lstrip('"').rstrip('"')
            payload = msg[2].lstrip('"').rstrip('"')
        except ValueError:
            print("Invalid MQTT msg:", urc_msg)
            return

        try:
            self.topic_handlers[topic](topic, payload)
            return
        except KeyError:
            pass

        print('''>>>RECV MQTT msg topic: %s\n\t\tpayload: %s:''' % (topic, payload))
        return

    def default_topic_handler(self, topic, payload):
        print('''RECV MQTT msg topic: %s\n\tpayload(len: %d): %s:''' % (topic, len(payload), payload))

    def mqtt_state_handler(self, msg):
        print("MQTT status msg:", msg)

    def set_devinfo(self, pid, dev_name, dev_key, tls=1):
        self.product_id = pid
        self.device_name = dev_name
        self.device_key = dev_key
        self.mqtt_tls_mode = tls

        # AT+TCDEVINFOSET=TLS_mode,"PRODUCTID","DEVICE_NAME","DEVICE_KEY"
        cmd = '''AT+TCDEVINFOSET=%d,"%s","%s","%s"''' % (tls, pid, dev_name, dev_key)
        ok_reply = '+TCDEVINFOSET:OK'
        hint = '''AT+TCDEVINFOSET="%s","%s"''' % (pid, dev_name)

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout)

    def is_mqtt_connected(self):
        # AT+TCMQTTSTATE?
        cmd = 'AT+TCMQTTSTATE?'
        ok_reply = 'OK'

        rc, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, self.err_list, self.cmd_timeout)
        if rc:
            try:
                status = rep.split(':', 1)[1].strip()
                if status == '1':
                    return True
                else:
                    return False
            except ValueError:
                print("Invalid MQTT state value:", rep)
                return False
        else:
            print("MQTT get state failed")
            print(rep)
            return False

    def mqtt_connect(self, timeout_ms=5000, keepalive=240, clean_session=1, reconnect=1):
        # use the larger timeout value
        if (timeout_ms/1000) > self.cmd_timeout:
            self.cmd_timeout = timeout_ms/1000
        else:
            timeout_ms = self.cmd_timeout * 1000

        self.mqtt_cmd_timeout_ms = timeout_ms
        self.mqtt_keepalive = keepalive
        self.mqtt_clean_session = clean_session
        self.mqtt_reconnect = reconnect

        # AT+TCMQTTCONN=1,5000,240,1,1
        cmd = '''AT+TCMQTTCONN=%d,%d,%d,%d,%d''' % (self.mqtt_tls_mode, timeout_ms, keepalive, clean_session, reconnect)
        ok_reply = '+TCMQTTCONN:OK'
        hint = cmd

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, 15)

    def mqtt_disconnect(self):
        # AT+TCMQTTDISCONN
        cmd = 'AT+TCMQTTDISCONN'
        ok_reply = 'OK'
        hint = cmd

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout)

    def publish_long_msg(self, topic, qos, msg, hint=''):
        # AT+TCMQTTPUBL="topic",QoS,msg_length
        msg = self.add_escapes(msg)
        cmd = '''AT+TCMQTTPUBL="%s",%d,%d''' % (topic, qos, len(msg))
        ok_reply = '>'
        hint = cmd

        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout):
            return False

        hint = "AT+TCMQTTPUBL Send msg"
        ok_reply = '+TCMQTTPUBL:OK'
        if not self.serial.do_one_at_cmd(msg, ok_reply, hint, self.err_list, self.cmd_timeout):
            print('>>pub long msg failed: ', msg)
            return False
        else:
            return True

    def publish_msg(self, topic, qos, msg, hint=''):
        # AT+TCMQTTPUB="topic",QoS,"msg"
        cmd = '''AT+TCMQTTPUB="%s",%d,"%s"''' % (topic, qos, self.add_escapes(msg))
        if len(cmd) > self.at_cmd_max_len:
            return self.publish_long_msg(topic, qos, msg, hint)

        ok_reply = '+TCMQTTPUB:OK'
        hint = "AT+TCMQTTPUB="+topic

        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout):
            print('>>cmd failed: ', cmd)
            return False
        else:
            return True

    def subscribe_topic(self, topic, qos, topic_callback=print):
        if topic in self.topic_handlers:
            # just update the dict value
            self.topic_handlers[topic] = topic_callback
            return

        # AT+TCMQTTSUB="topic",QoS
        cmd = '''AT+TCMQTTSUB="%s",%d''' % (topic, qos)
        ok_reply = '+TCMQTTSUB:OK'
        hint = cmd
        self.topic_handlers[topic] = topic_callback

        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout):
            del self.topic_handlers[topic]
            return False
        return True

    def unsubscribe_topic(self, topic):
        # AT+TCMQTTUNSUB="topic"
        cmd = '''AT+TCMQTTUNSUB="%s"''' % (topic)
        ok_reply = '+TCMQTTUNSUB:OK'
        hint = cmd

        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout):
            return False

        if topic in self.topic_handlers:
            del self.topic_handlers[topic]

        return True

    def unsubscribe_all_topics(self):
        ok_reply = '+TCMQTTUNSUB:OK'
        for i in self.topic_handlers:
            cmd = '''AT+TCMQTTUNSUB="%s"''' % (i)
            hint = cmd
            self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout)

        self.topic_handlers.clear()

    def ota_status_handler(self, msg):
        try:
            status = msg.split(':', 1)[1]
        except ValueError:
            print("\nInvalid ota status msg:", msg)
            return

        if status == 'ENTERUPDATE':
            print("\nOTA update start")
            self.ota_state = 'updating'
        elif status == 'UPDATESUCCESS':
            print("\nOTA update success")
            self.ota_state = 'success'
        elif status == 'UPDATEFAIL':
            print("\nOTA update failed")
            self.ota_state = 'failed'
        else:
            print("\nUnknown OTA update msg:", msg)
            self.ota_state = 'unknown'
        return

    def ota_update_setup(self, state, version):
        # AT+TCOTASET=state,"version""
        cmd = '''AT+TCOTASET=%d,"%s"''' % (state, version)
        ok_reply = '+TCOTASET:OK'
        hint = cmd

        self.serial.add_data_handler("+TCOTASTATUS", self.ota_status_handler)
        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, 10):
            return False

        self.ota_state = 'updating'
        return True

    def ota_read_fw_info(self):
        # AT+TCFWINFO?
        cmd = 'AT+TCFWINFO?'
        ok_reply = '+TCFWINFO'
        err_list = ['ERROR', '+TCFWINFO']

        rc, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, err_list, self.cmd_timeout)

        try:
            fw_info = rep.split(':', 1)[1].split(',')
            fw_version = fw_info[0].strip('"')
            fw_size = int(fw_info[1].strip('"'))
            fw_md5 = fw_info[2].strip('"')
            print('FW version:', fw_version, 'size:', fw_size, 'md5:', fw_md5)
            return fw_version, fw_size, fw_md5
        except (ValueError, IndexError):
            print("Invalid OTA FW info value:", rep.encode())
            return None, 0, None

    def ota_data_save_handler(self, data_raw):
        print("HEX data len:", len(data_raw))
        self.ota_data_queue.put(data_raw, timeout=2)

    def ota_read_fw_data(self, fw_version, fw_size, fw_md5):
        if fw_size <= 0:
            return

        file_name = '''ota_fetched_%s.bin''' % fw_version
        try:
            fw_file_out = open(file_name, "wb")
        except IOError:
            print("***** open file error:", file_name)
            return

        self.ota_data_queue = queue.Queue()
        self.serial.hex_data_handler = self.ota_data_save_handler

        # AT+TCREADFWDATA=2048
        cmd = 'AT+TCREADFWDATA=2048'
        ok_reply = '+TCREADFWDATA'
        err_list = ['ERROR', '+TCREADFWDATA']

        read_size = 0
        while read_size < fw_size:
            rc, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, err_list, self.cmd_timeout)

            try:
                read_size += int(rep.split('\n', 1)[1].split(':', 1)[1])
                print("Total read", read_size)
            except (ValueError, IndexError):
                print("Invalid OTA FW data reply:", rep)
                return

            data_raw = self.ota_data_queue.get()
            fw_file_out.write(data_raw)

        self.serial.hex_data_handler = self.serial.default_hexdata_handler
        del self.ota_data_queue
        fw_file_out.close()

        with open(file_name, "rb") as file_read:
            contents = file_read.read()

        md5_calc = hashlib.md5(contents).hexdigest()
        if fw_md5 != md5_calc:
            print("FW corrupted!\n MD5 from module", fw_md5, "\n MD5 from file", md5_calc)
        else:
            print('''OTA FW data (size %d) has been written to file %s.\n And MD5 (%s) is correct'''
                  % (read_size, file_name, md5_calc))

        return


class IoTHubATCmd(IoTBaseATCmd):
    def __init__(self, at_module='ESP8266'):
        super(IoTHubATCmd, self).__init__(at_module)

    def publish_normal_msg(self, topic_keyword, qos, msg):
        # "ProductID/device_name/keyword"
        topic = '''%s/%s/%s''' % (self.product_id, self.device_name, topic_keyword)

        return self.publish_msg(topic, qos, msg)

    def publish_hidden_msg(self, topic_keyword, qos, msg):
        # "$topic_keyword/operation/ProductID/device_name"
        topic = '''$%s/operation/%s/%s''' \
                % (topic_keyword, self.product_id, self.device_name)

        return self.publish_msg(topic, qos, msg)

    def subscribe_normal_topic(self, topic_keyword, qos, topic_callback=print):
        # "ProductID/device_name/topic_keyword"
        topic = '''%s/%s/%s''' % (self.product_id, self.device_name, topic_keyword)

        return self.subscribe_topic(topic, qos, topic_callback)

    def subscribe_hidden_topic(self, topic_keyword, qos, topic_callback=print):
        # "$topic_keyword/operation/result/ProductID/device_name"
        topic = '''$%s/operation/result/%s/%s''' \
                % (topic_keyword, self.product_id, self.device_name)

        return self.subscribe_topic(topic, qos, topic_callback)

    def unsubscribe_normal_topic(self, topic_keyword):
        # "ProductID/device_name/topic_keyword"
        topic = '''%s/%s/%s''' % (self.product_id, self.device_name, topic_keyword)

        return self.subscribe_topic(topic)

    def unsubscribe_hidden_topic(self, topic_keyword):
        # "$topic_keyword/operation/result/ProductID/device_name"
        topic = '''$%s/operation/result/%s/%s''' \
                % (topic_keyword, self.product_id, self.device_name)

        return self.subscribe_topic(topic)

    def parse_sys_time(self, topic, payload):
        try:
            obj = json.loads(payload)
            sys_time = obj["time"]
        except KeyError:
            print("Invalid time JSON：", topic, payload)
            return

        print("IoT Hub system time is", sys_time)

    def get_sys_time(self):
        msg = '{"type": "get", "resource": ["time"]}'
        return self.publish_hidden_msg('sys', 0, msg)

    def loop_test_handler(self, topic, payload):
        try:
            self.loop_test_queue.put(payload)
        except AttributeError:
            print("Recv msg", payload, "after loop test finished!")

    def do_loop_test(self, loop_cnt):
        self.loop_test_queue = queue.Queue()
        self.subscribe_topic(HubTestTopic, 1, self.loop_test_handler)

        send_cnt = 0
        fail_cnt = 0
        recv_cnt = 0
        recv_err = 0
        recv_timeout_err = 0
        start_time = time.time()
        max_send_recv_time = 0.0
        while send_cnt < loop_cnt:
            print("------IoT Hub MQTT QoS1 loop test cnt", send_cnt)
            send_time = time.time()
            loop_test_last_msg = get_hub_test_msg()
            ret = self.publish_msg(HubTestTopic, 1, loop_test_last_msg)
            send_cnt += 1
            if not ret:
                fail_cnt += 1
                continue

            recv_timeout_cnt = 0
            while True:
                try:
                    recv_payload = self.loop_test_queue.get(timeout=2 * self.cmd_timeout)
                    print("RECV MQTT loop test msg:", recv_payload)
                    recv_timeout_cnt = 0
                    if recv_payload == loop_test_last_msg:
                        recv_cnt += 1
                        send_recv_time = round(time.time() - send_time, 2) * 100
                        if send_recv_time > max_send_recv_time:
                            max_send_recv_time = send_recv_time
                    else:
                        print("*****Differ with last msg:", loop_test_last_msg)
                        recv_err += 1
                    break
                except queue.Empty:
                    print("*****RECV MQTT timeout!!", loop_test_last_msg)
                    recv_timeout_cnt += 1
                    if recv_timeout_cnt > 3:
                        recv_timeout_cnt = 0
                        recv_timeout_err += 1
                        break
                    continue
                except KeyboardInterrupt:
                    print("Test interrupted")
                    return

        end_time = time.time()
        print("---------IoT Hub MQTT QoS1 loop test result:--------")
        print("Test AT module:", self.at_module)
        print("Test start time:", time.ctime(start_time), " End time:", time.ctime(end_time))
        print("Test duration:", round(end_time - start_time, 1), "seconds")
        print("MQTT Msg Send count:", send_cnt)
        print("MQTT Msg Send failed count:", fail_cnt)
        print("MQTT Msg Recv success count:", recv_cnt)
        print("MQTT Msg Recv error total count:", recv_err, "timeout:", recv_timeout_err)
        print('''MQTT Publish success rate %.2f%%''' % (round(((send_cnt - fail_cnt) / send_cnt) * 100, 2)))
        print('''MQTT Send/Recv success rate %.2f%%''' % (round((recv_cnt / (loop_cnt - fail_cnt)) * 100, 2)))
        print('''MQTT Msg Send/Recv Ave time: %.2f seconds Max time: %.2f seconds'''
              % (round((end_time - start_time) / send_cnt, 2), round(max_send_recv_time / 100, 2)))
        print("---------IoT Hub MQTT QoS1 loop test end------------")

        del self.loop_test_queue

    def iot_hub_test(self, loop=False, set_loop_cnt=0):

        while True:
            if self.is_mqtt_connected():
                self.mqtt_disconnect()

            if not self.set_devinfo(Hub_Product_ID, Hub_Device_Name, Hub_Device_Key):
                break

            if not self.mqtt_connect():
                break

            if not self.subscribe_topic(HubTestTopic, 0, self.default_topic_handler):
                break

            # one shot test
            if not loop:
                if not self.subscribe_hidden_topic("sys", 0, self.parse_sys_time):
                    break

                if not self.publish_msg(HubTestTopic, 0, get_hub_test_msg()):
                    break

                if not self.publish_msg(HubTestTopic, 1, HubTestLongMsg):
                    break

                self.get_sys_time()
                time.sleep(1)
                self.unsubscribe_all_topics()
                time.sleep(0.5)
                self.mqtt_disconnect()
                return

            # loop test
            while loop:
                if set_loop_cnt == 0:
                    cmd = input("---------IoT Hub MQTT QoS1 loop test:--------\n"
                                "Input:\n"
                                "1. 'quit' to break\n"
                                "2. pub/sub loop count times [1..3000]\n"                            
                                "Your choice:\n").strip('\n')

                    if cmd.lower() == 'quit':
                        break

                    try:
                        loop_cnt = int(cmd)
                    except ValueError:
                        print("Invalid loop times:", cmd)
                        continue

                    if loop_cnt < 1 or loop_cnt > 3000:
                        loop_cnt = 10
                        print("loop times out of range, set to ", loop_cnt)

                    self.do_loop_test(loop_cnt)

                    # do it again
                    continue
                else:
                    self.do_loop_test(set_loop_cnt)
                    break

            self.unsubscribe_all_topics()
            time.sleep(0.5)
            self.mqtt_disconnect()
            return

    def ota_update_test(self, version):

        if self.is_mqtt_connected():
            self.mqtt_disconnect()

        ret = self.set_devinfo(Hub_Product_ID, Hub_Device_Name, Hub_Device_Key)
        if not ret:
            return

        ret = self.mqtt_connect()
        if not ret:
            return

        ret = self.ota_update_setup(1, version)
        if not ret:
            return

        while self.ota_state == 'updating':
            print('Wait for OTA update completed...')
            time.sleep(1)
            cmd = input("Input 'quit' to break:").strip('\n')
            if cmd.lower() == 'quit':
                break
            else:
                continue

        self.ota_update_setup(0, version)

        fw_version, fw_size, fw_md5 = self.ota_read_fw_info()

        self.ota_read_fw_data(fw_version, fw_size, fw_md5)

        self.mqtt_disconnect()

        return


class IoTExplorerATCmd(IoTBaseATCmd):
    def __init__(self, at_module='ESP8266'):
        super(IoTExplorerATCmd, self).__init__(at_module)
        self.template_result_err_cnt = 0
        self.event_result_err_cnt = 0

    def template_msg_handler(self, topic, payload):
        try:
            obj = json.loads(payload)
            ret_type = obj["type"]
        except KeyError:
            print("Invalid template JSON：", topic, payload)
            return

        try:
            if ret_type == 'get' or ret_type == 'update':
                result = obj["result"]
                state = obj["payload"]["state"]
                version = obj["payload"]["version"]

                print("----- template reply", ret_type, "msg ---------")
                print(json.dumps(state, indent=2))
                print("----- result %d version %d ---------" % (result, version))
                if result != 0:
                    self.template_result_err_cnt += 1
            elif ret_type == 'delta':
                state = obj["payload"]["state"]
                version = obj["payload"]["version"]

                print("----- template recv downlink msg ---------")
                print(json.dumps(state, indent=2))
                print("----- version %d ---------" % (version))
            
        except KeyError:
            print("Invalid template JSON：", topic, payload)
            self.template_result_err_cnt += 1

        return

    def event_msg_handler(self, topic, payload):
        try:
            obj = json.loads(payload)
            code = obj["code"]

            print("----- event result msg ---------")
            print(json.dumps(obj, indent=2))
            print("----- event result_code %d ---------" % code)
            if code != 0:
                self.event_result_err_cnt += 1
        except KeyError:
            print("Invalid event JSON：", topic, payload)
            self.event_result_err_cnt += 1

        return

    def subscribe_template_topic(self, qos=0):
        # "$template/operation/result/ProductID/DeviceName"
        topic = '''$template/operation/result/%s/%s''' % (self.product_id, self.device_name)

        return self.subscribe_topic(topic, qos, self.template_msg_handler)

    def get_template_data(self, qos=1):
        # "$template/operation/ProductID/DeviceName"
        topic = "$template/operation/%s/%s" % (self.product_id, self.device_name)
        msg = '{"type":"get", "clientToken":"clientToken-%d"}' % ( int(time.time()*10)%1000 )
        return self.publish_msg(topic, qos, msg)

    def reply_template_get_msg(self, qos=1):
        # "$template/operation/ProductID/DeviceName"
        topic = "$template/operation/%s/%s" % (self.product_id, self.device_name)
        msg = '{"desired":null, "clientToken":"clientToken-%d"}' % ( int(time.time()*10)%1000 )
        return self.publish_msg(topic, qos, msg)

    def publish_template_msg(self, msg, qos=0):
        # "$template/operation/ProductID/DeviceName"
        topic = "$template/operation/%s/%s" % (self.product_id, self.device_name)

        return self.publish_msg(topic, qos, msg)

    def subscribe_event_topic(self, qos=0):
        # "$thing/down/event/ProductID/DeviceName
        topic = '''$thing/down/event/%s/%s''' % (self.product_id, self.device_name)

        return self.subscribe_topic(topic, qos, self.event_msg_handler)

    def post_event_msg(self, msg, qos=0):
        # "$thing/up/event/ProductID/DeviceName"
        topic = '''$thing/up/event/%s/%s''' % (self.product_id, self.device_name)

        return self.publish_msg(topic, qos, msg)

    def iot_explorer_test(self, loop=False, loop_cnt=0):

        while True:
            if self.is_mqtt_connected():
                self.mqtt_disconnect()

            ret = self.set_devinfo(IE_Product_ID, IE_Device_Name, IE_Device_Key)
            if not ret:
                break

            ret = self.mqtt_connect()
            if not ret:
                break

            ret = self.subscribe_template_topic()
            if not ret:
                break

            ret = self.subscribe_event_topic()
            if not ret:
                break

            ret = self.post_event_msg(gen_event_post_msg())
            if not ret:
                break

            time.sleep(0.5)

            ret = self.get_template_data()
            if not ret:
                break

            time.sleep(0.5)

            ret = self.reply_template_get_msg()
            if not ret:
                break

            time.sleep(0.5)

            ret = self.publish_template_msg(gen_template_update_msg())
            if not ret:
                break

            while loop:
                time.sleep(0.5)

                if loop_cnt > 0:
                    test_cnt = 0
                    while test_cnt < loop_cnt:
                        self.publish_template_msg(gen_template_update_msg())
                        time.sleep(1)
                        self.post_event_msg(gen_event_post_msg())
                        time.sleep(1)
                        test_cnt += 1

                    print("---------IoT Explorer template/event loop test result:--------")
                    print("Test AT module:", self.at_module)
                    print("Test count:", test_cnt)
                    print("Template result error count:", self.template_result_err_cnt)
                    print("Event result error count:", self.event_result_err_cnt)
                    print("---------IoT Explorer template/event loop test end--------")
                    break

                cmd = input("---------IoT Explorer template/event loop test:--------\n"
                            "Input:\n"
                            "1. 'quit' to break\n"
                            "2. 'update' to send template msg\n"
                            "3. 'event' to post event msg\n"
                            "Your choice:\n").strip('\n')
                if cmd.lower() == 'quit':
                    break

                if cmd.lower() == 'update':
                    #self.get_template_data()
                    #time.sleep(0.5)
                    self.publish_template_msg(gen_template_update_msg())

                if cmd.lower() == 'event':
                    self.post_event_msg(gen_event_post_msg())

            time.sleep(0.5)

            self.unsubscribe_all_topics()

            time.sleep(0.5)

            ret = self.mqtt_disconnect()
            if not ret:
                break

            # one shot test
            return


class ESPWiFiATCmd:
    def __init__(self, cmd_timeout=3):
        self.serial = SerialATClient()
        self.cmd_timeout = cmd_timeout  # unit: seconds
        self.err_list = ['ERROR', 'busy']
        self.boarding_state = 'off'

    def do_network_connection(self, ssid, psw):
        for i in range(5):
            if self.is_network_connected():
                return True

            print("Connecting to wifi", ssid, psw)
            self.set_wifi_mode(1)
            self.join_wifi_network(ssid, psw)

        return False

    def join_wifi_network(self, ssid, psw):
        # AT+CWJAP="SSID","PSW"
        cmd = '''AT+CWJAP="%s","%s"''' % (ssid, psw)
        ok_reply = 'OK'
        hint = cmd

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, 10)        

    def is_network_connected(self):
        # AT+CWJAP?
        cmd = 'AT+CWJAP?'
        ok_reply = 'OK'

        ret, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, self.err_list, self.cmd_timeout)
        if ret:
            ap_str = rep

        # do this by checking STA IP value
        # AT+CIPSTA?
        cmd = 'AT+CIPSTA?'
        ok_reply = 'OK'

        ret, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, self.err_list, self.cmd_timeout)
        if ret:
            try:
                ip_str = rep.split('\n', 1)[0]
                status = ip_str.find('+CIPSTA:ip:"0.0.0.0"')
                if status == -1:
                    print("Network is connected")
                    print(ap_str+ip_str)
                    return True
                else:
                    print("WiFi network is not connected")
                    return False
            except ValueError:
                print("Invalid WiFi IP status value:", rep)
                return False

        else:
            print("WiFi get IP status failed")
            print(rep)
            return False

    def get_wifi_status(self):
        # AT+CIPSTATUS
        cmd = 'AT+CIPSTATUS'
        ok_reply = 'OK'

        rc, rep = self.serial.send_cmd_wait_reply(cmd, ok_reply, self.err_list, self.cmd_timeout)
        if rc:
            try:
                status = rep.split(':', 1)[1].strip()
                if status == '2':
                    print("WiFi is connected")
                    return True
                else:
                    return False
            except ValueError:
                print("Invalid WiFi state value:", rep)
                return False
        else:
            print("WiFi get state failed")
            print(rep)
            return False

    def set_wifi_mode(self, mode):
        '''
        :param mode:
         0: WIFI OFF
         1: Station
         2: SoftAP
         3: SoftAP+Station
        '''
        # AT+CWMODE=mode
        cmd = '''AT+CWMODE=%d''' % (mode)
        ok_reply = 'OK'
        hint = cmd

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, self.cmd_timeout)
    
    def smartconfig_msg_handler(self, msg):
        try:
            status = msg.split(':', 1)[1]
        except ValueError:
            print("\nInvalid smartconfig msg:", msg)
            return

        if status == 'WIFI_CONNECT_SUCCESS':
            print("\nsmartconfig boarding and connection success")
            self.boarding_state = 'success'
        elif status == 'WIFI_CONNECT_FAILED':
            print("\nsmartconfig boarding and connection failed")
            self.boarding_state = 'failed'
        else:
            print("\nUnknown smartconfig msg:", msg)
            self.boarding_state = 'unknown'
        return

    def start_smartconfig(self):
        # AT+TCSTARTSMART"
        cmd = 'AT+TCSTARTSMART'
        ok_reply = '+TCSTARTSMART:OK'
        hint = cmd

        if not self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, 10):
            return False

        self.serial.add_data_handler("+TCSTARTSMART", self.smartconfig_msg_handler)
        self.boarding_state = 'boarding'
        return True

    def stop_smartconfig(self):
        # AT+TCSTOPSMART"
        cmd = 'AT+TCSTOPSMART'
        ok_reply = '+TCSTOPSMART:OK'
        hint = cmd

        return self.serial.do_one_at_cmd(cmd, ok_reply, hint, self.err_list, 10)

    def wifi_boarding_test(self):

        while True:
            qcloud = IoTBaseATCmd('ESP8266')

            if qcloud.is_mqtt_connected():
                qcloud.mqtt_disconnect()

            ret = qcloud.set_devinfo(IE_Product_ID, IE_Device_Name, IE_Device_Key)
            if not ret:
                break

            ret = self.start_smartconfig()
            if not ret:
                break

            while self.boarding_state == 'boarding':
                print('Wait for WiFi boarding completed...')
                time.sleep(1)
                cmd = input("Input 'quit' to break:").strip('\n')
                if cmd.lower() == 'quit':
                    break
                else:
                    continue

            if self.boarding_state == 'success':
                IoTExplorerATCmd('ESP8266').iot_explorer_test(loop=True)

            break

        return


def nw_network_connect():
    serial = SerialATClient()

    ok_reply = 'OK'
    err_list = ['ERROR']
    cmd_timeout = 10

    network_reg = False
    # AT+CREG? and expect: +CREG: 0,1 or 0,5
    cmd = "AT+CREG?"
    for i in range(10):
        rc, rep = serial.send_cmd_wait_reply(cmd, ok_reply, err_list, cmd_timeout)
        if rc:
            try:
                status = rep.split(':', 1)[1].strip()
                if status == '0,1' or status == '0,5':
                    print("Network is registered")
                    network_reg = True
                    break
                else:
                    print("Network is NOT registered", status)

            except ValueError:
                print("Invalid network state value:", rep)

            time.sleep(1)
            continue
        else:
            print(cmd, "error:", rep)
            return False

    if not network_reg:
        return False

    cmd = "AT+XIIC=1"
    rc, rep = serial.send_cmd_wait_reply(cmd, ok_reply, err_list, cmd_timeout)
    if not rc:
        print(cmd, "error:", rep)
        return False

    # AT+XIIC? and expect: +XIIC: 1,xxx.xxx.xxx.xxx
    cmd = "AT+XIIC?"

    for i in range(10):
        rc, rep = serial.send_cmd_wait_reply(cmd, ok_reply, err_list, cmd_timeout)
        if rc:
            try:
                status = rep.split(':', 1)[1].strip()
                if status != '0,0.0.0.0':
                    print("Network is activated")
                    return True
                else:
                    print("Network is NOT activated", status)

            except ValueError:
                print("Invalid network state value:", rep)

            time.sleep(1)
            continue
        else:
            print(cmd, "error:", rep)
            break

    return False


def at_module_network_connect(at_module):
    if at_module == 'ESP8266':
        return ESPWiFiATCmd().do_network_connection(WiFi_SSID, WiFi_PSWD)
    elif at_module == 'NW-N21':
        return nw_network_connect()
    else:
        return True


def main():
    parser = argparse.ArgumentParser(description="QCloud IoT AT commands test tool")
    parser.add_argument('--version', action='version', version='%(prog)s v1.0.0')
    test_mode_group = parser.add_argument_group('AT commands mode parameters')
    test_mode_group.add_argument(
            "--mode", required=True,
            help="Test mode: CLI/WIFI/HUB/IE/OTA")

    test_mode_group.add_argument(
        "--module",
        help='AT module HW: ESP8266/NW-N21(default: ESP8266)',
        default="ESP8266")

    test_mode_group.add_argument(
        "--loop", type=str,
        help='To do loop test or not',
        default="False")

    test_mode_group.add_argument(
        "--loop_cnt", type=int,
        help='loop test times count',
        default=0)

    test_mode_group.add_argument(
        "--debug",
        help='To print debug message or not',
        default="False")

    serial_port_group = parser.add_argument_group('Serial port parameters')
    serial_port_group.add_argument(
        "--port", required=True,
        help='which serial port. COM5 or /dev/ttyUSB0')

    serial_port_group.add_argument(
        "--baudrate",
        help='serial port baudrate(default: 115200)',
        default=115200)

    args = parser.parse_args()

    global g_serial_port
    g_serial_port = args.port
    global g_serial_baudrate
    g_serial_baudrate = args.baudrate

    at_module = args.module
    test_mode = args.mode

    if args.loop.upper() == "TRUE" or args.loop_cnt > 0:
        loop = True
        loop_cnt = args.loop_cnt
    else:
        loop = False
        loop_cnt = 0

    global g_debug_print
    if args.debug.upper() == "TRUE":
        g_debug_print = True
    else:
        g_debug_print = False

    # Let's Rock'n'Roll

    # interactive command line test
    if test_mode.upper() == 'CLI':
        interactive_test()
        return

    SerialATClient().start_read()

    SerialATClient().echo_off()
    while True:
        # WiFi boarding test
        if test_mode.upper() == 'WIFI':
            ESPWiFiATCmd().wifi_boarding_test()
            break

        # For other tests, connect network first
        if not at_module_network_connect(at_module):
            break

        # IoT Hub test
        if test_mode.upper() == 'HUB':
            IoTHubATCmd(at_module).iot_hub_test(loop, loop_cnt)
            break

        # IoT Hub OTA test
        if test_mode.upper() == 'OTA':
            IoTHubATCmd(at_module).ota_update_test("1.0.0")
            break

        # IoT Explorer test
        if test_mode.upper() == 'IE':
            IoTExplorerATCmd(at_module).iot_explorer_test(loop, loop_cnt)
            break

    SerialATClient().close_port()
    return


if __name__ == "__main__":
    main()