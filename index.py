import web
from web.contrib.template import render_jinja
from datetime import datetime
from datetime import timedelta
import serial
import re
import random

web.config.debug = False

urls = (
  '/admin', 'admin',
  '/arduino/(.*)', 'arduino_change',
  '/music_hook/(.*)', 'music_hook',
  '/(.*)', 'index'
)

min_ttl = 10
change_time = datetime.now() - timedelta(seconds=min_ttl)

render = render_jinja('templates', encoding='utf-8')
app = web.application(urls, globals())
session = web.session.Session(app, web.session.DiskStore('sessions'))
color_chars = "1234567890abcdef"
ser = None


def try_connect():
    global ser
    for t in ['USB', 'ACM']:
        for n in range(0, 20):
            try:
                ser = serial.Serial('/dev/tty%s%s' % (t, n))
                return
            except:
                pass


def sendit(msg):
    global ser
    ctr = 0

    if ser is None:
        try_connect()

    ctr = 0
    while ctr < 4:
        ctr = ctr + 1
        try:
            ser.write(toSend)
            return True
        except:
            ser.close()
            try_connect()

    return False


def doError(msg):
    session.error = msg
    return web.SeeOther('/')


class index:
    def GET(self, page_name):
        all_pages = ['Lights', 'Music']
        if page_name not in all_pages:
            page_name = 'lights'

        numsec = int(min_ttl -
                     ((datetime.now()-change_time).total_seconds()))
        numsec = numsec if numsec > 0 else 0

        error = ""
        try:
            error = session.error
        except:
            pass
        session.error = ""

        r = random.choice(color_chars) + random.choice(color_chars)
        g = random.choice(color_chars) + random.choice(color_chars)
        b = random.choice(color_chars) + random.choice(color_chars)

        return render.base(page_name=page_name,
                           all_pages=all_pages,
                           numsec=numsec,
                           error=error,
                           red=r,
                           green=g,
                           blue=b)


class admin:
    def GET(self):
        return "Nothing to see here"


kinds = ['colorwipe', 'theater', 'rainbow', 'rainbowchase', 'theaterrainbow',
         'knightrider']
colorRegex = re.compile('#[0-9a-fA-F]{6}')


class arduino_change:
    def POST(self, kind):
        global change_time
        global ser
        s = web.input()
        if kind in kinds:
            seconds_passed = (datetime.now()-change_time).total_seconds()
            if seconds_passed > min_ttl:
                toSend = ""
                toSend += chr(kinds.index(kind))
                if kind in ['colorwipe', 'theater', 'knightrider']:
                    if 'color' in s and re.match(colorRegex, s['color']):
                        for i in [1, 3, 5]:
                            toSend += chr(int(s['color'][i]+s['color'][i+1], 16))
                else:
                    for x in range(3):
                        toSend += chr(0)

                if 'speed' in s and 1 < int(s['speed']) < 10:
                    toSend += chr((11 - int(s['speed']))*10)
                else:
                    toSend += chr(5)

                if len(toSend) != 5:
                    doError("Something went seriously wrong :(")

                if not sendit(toSend):
                    doError("<strong>Unable to contact the controller</strong>")

                change_time = datetime.now()
                return web.SeeOther('/')

            else:
                s = 'second' if int(seconds_passed) == 1 else 'seconds'
                session.error = "<strong>Last change was made %s %s ago.</strong> Try later." % (int(seconds_passed), s)
                return web.SeeOther('/')

        else:
            raise web.NotFound('aaaaah')

    def GET(self, kind):
        web.redirect(web.ctx.home)

if __name__ == "__main__":
    app.run()
