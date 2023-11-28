# INFH 5000 Project 42
# Internet of Things Real Random Number Generator (IoT RRNG)
# User Datagram Protocol (UDP) Client

# Developers: Dayou Du
# Emails: ddu487@connect.hkust-gz.edu.cn

# Copyright (C) 2023 Dayou Du, GNU AGPL3/3+ license.

import socket
import re

# 创建一个 socket 来接收 UDP 数据
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# 绑定到一个端口（例如 50420，您可以根据需要更改）
sock.bind(('', 50420))

print("正在监听端口 50420")

# 正则表达式模式
pattern = rb"packetIndex (\d+) randomNumber (\d+)"

# 循环接收数据
while True:
    data, address = sock.recvfrom(4096)  # 缓冲区大小设置为 4096 字节
    print(f"receive {len(data)} Btyes from {address}: {data}")

    # 使用正则表达式搜索
    match = re.search(pattern, data)

    packetIndex = match.group(1)
    randomNumber = match.group(2)

    # 首先，将字节串解码为字符串
    randomNumber_str = randomNumber.decode('utf-8')

    # 然后，将字符串转换为整数
    randomNumber_int = int(randomNumber_str)
    print("packetIndex:", packetIndex)
    print("randomNumber:", randomNumber_int)
