# -*- coding: UTF-8 -*-
'''
发送txt文本邮件
小五义：http://www.cnblogs.com/xiaowuyi
'''
import smtplib
from email.mime.text import MIMEText
gmail_user="roy_wang109"    #用户名
gmail_pwd="609330246"   #口令

def send_email(user, pwd, recipient, subject, body):
    import smtplib

    gmail_user = user
    gmail_pwd = pwd
    FROM = user
    TO = recipient if type(recipient) is list else [recipient]
    SUBJECT = subject
    TEXT = body

    # Prepare actual message
    message = """\From: %s\nTo: %s\nSubject: %s\n\n%s
    """ % (FROM, ", ".join(TO), SUBJECT, TEXT)
    # try:
    server = smtplib.SMTP("smtp.163.com", 587, timeout=120)
    server.ehlo()
    server.starttls()
    server.login(gmail_user, gmail_pwd)
    server.sendmail(FROM, TO, message)
    server.close()
    print 'successfully sent the mail'
    # except:
    #     print "failed to send mail"

send_email(gmail_user, gmail_pwd, "jhcz@sjtu.edu.cn", "test", "test")

# SMTP_SSL Example
# server_ssl = smtplib.SMTP_SSL("smtp.gmail.com", 465)
# server_ssl.ehlo() # optional, called by login()
# server_ssl.login(gmail_user, gmail_pwd)
# # ssl server doesn't support or need tls, so don't call server_ssl.starttls()
# server_ssl.sendmail("mny316@gmail.com", "roy_wang109@163.com", "test")
# #server_ssl.quit()
# server_ssl.close()
# print 'successfully sent the mail'
