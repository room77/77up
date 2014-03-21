#!/usr/bin/python2.7

# Copyright 2012 Room77, Inc.
# Author: Uygar Oztekin

import os
import sys
import re
import argparse
import datetime
import json
import string

class Pager:
  """
  Cronable pager script for alerts. Monitors emails send to specified address.

  For each email with a subject that matches the criteria, monitors the thread.
  * For initial email, sends a message to this week's primary contact.
  * If anyone replies to the original thread (e.g. the primary contact), alert
    associated with that thread is muted.
  * If the script is run a second time and there is still no reply, backup
    contact is alerted and alert is muted.

  Temporary status files between runs are stored under the status_dir. Hence
  user that runs the pager must be able to create files under that directory.
  """
  config_file = os.path.dirname(os.path.realpath(__file__))+"/pager_config.txt"
  status_dir = "/tmp/pager/"
  status_file = status_dir + "pager_status.json"
  monitor_email = ""
  monitor_pass = ""
  monitor_phone = ""
  alert_match_pattern = ".*ALARM.*"
  reply_match_pattern = "^R[eE]:.*ALARM.*"
  dry_run = 0
  offset_days = 0

  _status = dict()
  _active_list = []
  _sent_one = 0

  class Status:
    NEW = "new"
    OLD = "old"
    REPLIED = "replied"

  def Init(self):
    data = open(self.config_file)
    self._active_list = []
    for line in data.readlines() :
      if re.match("^#", line) : continue
      line = line.strip()
      m = re.match(r"(?P<phone>[\d]{3}-[\d]{3}-[\d]{4})[\t](?P<email>\w+[@][\w\.]+)", line)
      self._active_list += [(m.group("phone").translate(None, "-+"), m.group("email"))]

  def _InfoString(self):
    out = ""
    list = self._active_list
    out += "Pager rotation contains " + str(len(list)) + " contacts:\n"
    for p in list:
      out += p[0] + "\t" + p[1] +"\n"
    primary = self._GetPrimary();
    backup = (primary + 1) % len(list)

    out += "\nCurrent contacts:"
    out += "\nPrimary: " + str(list[primary])
    out += "\nBackup:  " + str(list[backup])
    out += "\n\nPlease turn your phones to MAX volume to ensure you receive pages."

    if self.offset_days != 0:
      primary = self._GetPrimary(self.offset_days);
      backup = (primary + 1) % len(list)
      out += "\n\nContacts " + str(self.offset_days) + " days from now:"
      out += "\nPrimary: " + str(list[primary])
      out += "\nBackup:  " + str(list[backup])
    return out

  def Info(self):
    print self._InfoString()

  def MailInfo(self, sender, receiver):
    import smtplib
    # sending html email described here:
    #  http://stackoverflow.com/questions/882712/sending-html-email-in-python
    from email.mime.multipart import MIMEMultipart
    from email.mime.text import MIMEText
    msg = MIMEMultipart('alternative')
    msg['Subject'] = "Pager rotation notification"
    msg['From'] = sender
    msg['To'] = receiver

    html = """\
<html>
  <head></head>
  <body>
    <p>""" + re.sub("\n", "<br>", self._InfoString()) +  """</p>
    <p>Pager rotation happens every monday at 00:00.</p>
<p>If the primary / backup contacts are unavailable, please
<ol>
<li>modify pager_config.txt</li>
<li>test your changes via ./pager.py --info</li>
<li>check them in and git push changes to be reflected</li>
<li>run ./pager.py --mail_info to broadcast the new info</li>
</ol>
</p>

  </body>
</html>"""
    # Record the MIME types of both parts - text/plain and text/html.
    #part1 = MIMEText(text, 'plain')
    part2 = MIMEText(html, 'html')

    # Attach parts into message container.
    # According to RFC 2046, the last part of a multipart message, in this case
    # the HTML message, is best and preferred.
    #msg.attach(part1)
    msg.attach(part2)
    smtp = smtplib.SMTP('localhost')
    smtp.sendmail(sender, receiver, msg.as_string());

  def SendAlert(self, msg, index = 0):
    list = self._active_list
    n = (self._GetPrimary() + index) % len(list)
    print "Paging " + list[n][1] + " with message: " + msg
    if not self.dry_run: self._SendAlertToPhone(list[n][0], msg)

  def _PrintStatus(self):
    if len(self._status) > 0: print "Status : Message ID"
    for key, value in self._status.items():
      print value["status"] + " : " + key

  def _ProcessStatus(self):
    more = "\nSee https://docs.google.com/a/room77.com/document/d/1YMrE5nM4aTG65ah6B3G_TxOG3vEKs4E84yKc4iAxH38/edit"
    for key, value in self._status.items():
      if value["status"] == self.Status.NEW:
        self.SendAlert("Alert. Please check your email. Subject: " + value["subject"] + more, 0)
      if value["status"] == self.Status.OLD:
        self.SendAlert("Alert. Please check your email. Subject: " + value["subject"] + more, 1)
      # print value["status"] + " : " + key
    remove_list = [k for (k,v) in self._status.items() if v["status"] == self.Status.OLD or v["status"] == self.Status.REPLIED ]
    for key in remove_list: del(self._status[key])

  def _ReadStatusFile(self):
    if not os.path.exists(self.status_file): return
    data = open(self.status_file).read()
    if (data == None or data == ""): return
    self._status = json.loads(data)
    for key, value in self._status.items():
      if value["status"] == self.Status.NEW: self._status[key]["status"] = self.Status.OLD

  def _WriteStatusFile(self):
    if self.dry_run == 1: self._status.clear()
    data = open(self.status_file, "w")
    json.dump(self._status, data)
    data.close()

  def _ProcessMail(self, mid, rid, date, sender, subject, body):
    if rid in self._status:
      # Existing alert.
      self._status[rid]["status"] = self.Status.REPLIED
    elif mid not in self._status:
      # New alert.
      self._status[mid] = dict()
      self._status[mid]["status"] = self.Status.NEW
      self._status[mid]["date"] = date
      self._status[mid]["subject"] = subject
      self._status[mid]["body"] = body
    if re.match(self.reply_match_pattern, subject):
      print "At least one reply found muting current alarms"
      self.dry_run = 1

  def _FetchMails(self):
    import poplib
    mail = poplib.POP3_SSL("pop.gmail.com")
    mail.user(self.monitor_email)
    mail.pass_(self.monitor_pass)
    n = len(mail.list()[1])
    print "Found " + str(n) + " new emails."
    for i in range(n):
      mid = rid = date = sender = subject = body = ""
      body_started = 0
      for line in mail.retr(i+1)[1]:
        if body_started :
          body = body + line + '\n'
        else:
          if re.match("^Message-ID: ", line, re.IGNORECASE) : mid = line[line.find(" ")+1:]
          if re.match("^In-Reply-To: ", line, re.IGNORECASE) : rid = line[line.find(" ")+1:]
          if re.match("^Subject: ", line, re.IGNORECASE) : subject = line[line.find(" ")+1:]
          if re.match("^Date:", line, re.IGNORECASE) : date = line[line.find(" ")+1:]
          if re.match("^From: ", line, re.IGNORECASE) : sender = line[line.find(" ")+1:]
          if not body_started and re.match("^$", line) : body_started = 1
      if re.match(self.alert_match_pattern, subject) : self._ProcessMail(mid, rid, date, sender, subject, body)
      mail.dele(i+1)
    mail.quit()
    # self._PrintStatus()
    self._ProcessStatus()
    self._WriteStatusFile()
    self._PrintStatus()

  def _GetPrimary(self, offset_days = 0) :
    # If offset_days is 0, number of days from some monday. mod 7 of this number
    # is 0 if it is a monday. Rotation occurs at 00:00, every monday.
    days = (datetime.datetime.today() - datetime.datetime.utcfromtimestamp(0)).days + 3 + offset_days
    return (days / 7) % len(self._active_list)

  def _SendAlertToPhone(self, phone, msg):
    if not self._sent_one:
      import code
      from googlevoice import Voice
      voice = Voice()
      voice.login(self.monitor_email, self.monitor_pass)
      voice.send_sms(phone, msg)                # Send an SMS.
      voice.call(phone, self.monitor_phone, 3)  # Call the person as well.
      self._sent_one = 1

  def Run(self):
    if not os.path.exists(self.status_dir):
      os.makedirs(self.status_dir)
    self._ReadStatusFile()
    return self._FetchMails()


def main():
  # Fill in the default values relevant for you to avoid adding the flags for each run.
  parser = argparse.ArgumentParser(description="Handle pager rotations and alerts.");
  parser.add_argument("--info",          action = "store_true", help="Outputs active pager duty list and current primary and backup.");
  parser.add_argument("--mail_info",     action = "store_true", help="Mails active pager duty list and current primary and backup to the specified email address.");
  parser.add_argument("--dry_run",       action = "store_true", help = "Do not wake people up. Output information to console instead.");
  parser.add_argument("--call",          type = int, help = "Offset of person on pager duty to call. 0 means primary, 1 means backup, 2 means secondary backup etc.");
  parser.add_argument("--sender",        type = str, default = "", help="In mail_info mode, send the email from this address.");
  parser.add_argument("--receiver",      type = str, default = "", help="In mail_info mode, send the email to this address.");
  parser.add_argument("--offset_days",   type = int, default = 0,  help = "Offset to add to current time. This can be used to compute primary / backup at a future / past time.");
  parser.add_argument("--msg",           type = str, default = "An alert has been issued. Please check your email.", help = "Message to send (for SMS portion)");
  parser.add_argument("--monitor_email", type = str, default = "", help = "Email address to monitor for the alarm pattern. Needs to be a google account with gvoice.");
  parser.add_argument("--monitor_pass",  type = str, default = "", help = "Password for the monitor email address.");
  parser.add_argument("--monitor_phone", type = str, default = "", help = "Google voice phone number associated with the account. Example: 15551231234");
  args = parser.parse_args()
  pager = Pager()
  pager.Init()
  pager.offset_days = args.offset_days
  pager.monitor_email = args.monitor_email
  pager.monitor_pass = args.monitor_pass
  pager.monitor_phone = args.monitor_phone

  if args.dry_run: pager.dry_run = 1
  if args.info: return pager.Info()
  if args.mail_info: return pager.MailInfo(args.sender, args.receiver)
  if args.call != None: return pager.SendAlert(args.msg, args.call)
  return pager.Run()


if __name__ == '__main__':
  sys.exit(main())
