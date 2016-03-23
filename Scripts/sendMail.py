# -*- coding: UTF-8 -*-
'''
发送txt文本邮件
小五义：http://www.cnblogs.com/xiaowuyi
'''
import smtplib
from email.mime.text import MIMEText
mailto_list=["roy_wang109@163.com"]
mail_host="smtp.sjtu.edu.cn"  #设置服务器
mail_user="jhcz"    #用户名
mail_pass="jhcz316,."   #口令
mail_postfix="sjtu.edu.cn"  #发件箱的后缀
#-*- encoding: gb2312 -*-
import os, sys, string
import smtplib

# 邮件服务器地址
mailserver = "smtp.sjtu.edu.cn"
# smtp会话过程中的mail from地址
from_addr = "jhcz@sjtu.edu.cn"
# smtp会话过程中的rcpt to地址
to_addr = "roy_wang109@163.com"
# 信件内容
msg = "test mail"
import os, sys, string, socket
import smtplib


class SMTP_SSL (smtplib.SMTP):
    def __init__(self, host='', port=465, local_hostname=None, key=None, cert=None):
        self.cert = cert
        self.key = key
        smtplib.SMTP.__init__(self, host, port, local_hostname)

    def connect(self, host='localhost', port=465):
        if not port and (host.find(':') == host.rfind(':')):
            i = host.rfind(':')
            if i >= 0:
                host, port = host[:i], host[i+1:]
                try: port = int(port)
                except ValueError:
                    raise socket.error, "nonnumeric port"
        if not port: port = 654
        if self.debuglevel > 0: print>>stderr, 'connect:', (host, port)
        msg = "getaddrinfo returns an empty list"
        self.sock = None
        for res in socket.getaddrinfo(host, port, 0, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                self.sock = socket.socket(af, socktype, proto)
                if self.debuglevel > 0: print>>stderr, 'connect:', (host, port)
                self.sock.connect(sa)
                # 新增加的创建ssl连接
                sslobj = socket.ssl(self.sock, self.key, self.cert)
            except socket.error, msg:
                if self.debuglevel > 0:
                    print>>stderr, 'connect fail:', (host, port)
                if self.sock:
                    self.sock.close()
                self.sock = None
                continue
            break
        if not self.sock:
            raise socket.error, msg

        # 设置ssl
        self.sock = smtplib.SSLFakeSocket(self.sock, sslobj)
        self.file = smtplib.SSLFakeFile(sslobj);

        (code, msg) = self.getreply()
        if self.debuglevel > 0: print>>stderr, "connect:", msg
        return (code, msg)

if __name__ == '__main__':
    smtp = SMTP_SSL('192.168.2.10')
    smtp.set_debuglevel(1)
    smtp.sendmail("zzz@xxx.com", "zhaowei@zhaowei.com", "xxxxxxxxxxxxxxxxx")
    smtp.quit()
