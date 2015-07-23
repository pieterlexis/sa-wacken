import web
from web.contrib.template import render_jinja
import os

urls = (
  '/', 'index',
  '/admin', 'admin'
)

render = render_jinja('templates', encoding='utf-8')
app = web.application(urls, globals())


class index:
    def GET(self):
        return render.homepage(name="bla")


class admin:
    def GET(self):
        return "Nothing to see here"

if __name__ == "__main__":
    app.run()
