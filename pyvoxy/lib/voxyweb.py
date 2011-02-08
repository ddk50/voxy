# -*- coding: utf-8 -*-

from voxycommon import *

import cherrypy
from cherrypy.lib.static import serve_file

import json

import subprocess
from threading import Thread
import os
import os.path
from twisted.internet import reactor, protocol
import voxyproxy

import ConfigParser
import StringIO



ANONYMOUS = 'anonymous'
def getAuthUser():
	#get authorization from apache/lighttpd/squid
	try:
		hdr = cherrypy.request.headers['Authorization']
		start = hdr.find('Digest username="')
		end = hdr.find('"', start+17)
		return hdr[start+17:end]
	except KeyError:
		return ANONYMOUS


current_dir = os.path.dirname(os.path.abspath(__file__))

def json_expose(f):
    def decotared(*idx, **kw):
        for key in kw:
            if key.endswith('[]'):
                if isinstance(kw[key], basestring):
                    kw[key] = [kw[key]]
                kw[key[:-2]] = kw[key]
                del kw[key]
        cherrypy.response.headers["content-type"] = "text/json"
        return json.dumps(f(*idx, **kw))
    return cherrypy.expose(decotared)
    

class VoxyWeb:
    
    def serve_file(self, fn, content_type='application/octet-stream'):
        return serve_file(os.path.join(current_dir, fn))

    @cherrypy.expose
    def index(self):
        raise cherrypy.HTTPRedirect("/login")
        cherrypy.session['testf'] = cherrypy.session.get('testf', 0) + 1
        rval = cherrypy.session['testf']
        return "Hello World!: ", str(rval)

    @cherrypy.expose
    def login(self, msg = None):
        if msg:
            msg_box = "<p>" + msg + "</p>"
        else:
            msg_box = ""
        html = """
        <html>
            <head>
                <title>VoXY Login</title>
                <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
                <link href="/files/voxy.css" rel="stylesheet" type="text/css" />
            </head>
            <body>
                {msg}
                <form action="/dologin" method='POST'>
                    User: <input type='text' name='user' size='32' /><br />
                    Password: <input type='password' name='password' size='32' /><br />
                    <button type='submit'>ログイン</button>
                </form>
            </body>
        </html>
        """.format(msg = msg_box)
        return html

    @cherrypy.expose
    def dologin(self, user, password):
        config = GetConfig()
        if config.check_password_for_user(user, password):
            cherrypy.session['login_user'] = user
            raise cherrypy.HTTPRedirect("/files/nvoxy.html")
        else:
            raise cherrypy.HTTPRedirect("/login?msg=LoginError")

    @cherrypy.expose
    def logout(self):
        del cherrypy.session['login_user']
        html = """
        <html>
            <head>
                <title>VoXY Login</title>
                <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
                <link href="/files/voxy.css" rel="stylesheet" type="text/css" />
            </head>
            <body>
                <p>ログアウトしました</p>
                <p><a href='/index'>トップページへ</a></p>
            </body>
        </html>
        """
        return html

    @cherrypy.expose
    def listc(self):
        config = GetConfig()
        targets = config.list_targets()
        return "List: " + str(targets)

    def get_current_user(self):
        if 'login_user' not in cherrypy.session:
            return ""
        return cherrypy.session['login_user']

    @json_expose
    def jlist(self):
        config = GetConfig()
        target_names = config.list_targets_for_user(self.get_current_user())
        targets = []
        for name in target_names:
            targets.append( config.get_target_options(name) )
        return {"targets": targets}

    @json_expose
    def jcmd(self, target, cmd):
        config = GetConfig()
        os_cmd = config.get_command(target, cmd)
        print "Invoke: ", os_cmd
        result = subprocess.Popen(os_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        return {"alert": result[0] + "\n" + result[1]}
        
    @json_expose
    def jrev(self, targets):
        revs = {}
        for t in targets:
            client = voxyproxy.Manager.getClient(t, try_connect = True)
            if client is None:
                revs[t] = -1
            else:
                revs[t] = client.getImageRevision()
        
        return revs


    @cherrypy.expose
    def smallimg(self, target, sid = ""):
        client = voxyproxy.Manager.getClient(target, try_connect = True)
        if client is None:
            return self.serve_file("webfiles/notready.png", "image/png")
        
        strm = StringIO.StringIO()
        client.saveSmallToStrm(strm)
        
        cherrypy.response.headers["content-type"] = "image/png"
        return strm.getvalue()

    @cherrypy.expose
    def tileimg(self, target, tile, sid = ""):
        config = GetConfig()
        if not config.has_target(target):
            return self.serve_file("webfiles/notready.png", "image/png")

        client = voxyproxy.Manager.getClient(target)
        if client is None:
            return self.serve_file("webfiles/notready.png", "image/png")

        strm = StringIO.StringIO()
        pos = tile.strip().split('_')
        client.saveTileToStrm(strm, int(pos[0]), int(pos[1]))
        cherrypy.response.headers["content-type"] = "image/png"
        return strm.getvalue()


    @json_expose
    def fullimg_revisions(self, target):
        client = voxyproxy.Manager.getClient(target, try_connect = True)
        if client is None:
            return {}

        client.useFullImage()
        result = {}
        for tx,ty in client.tile_revisions:
            result["{0}_{1}".format(tx,ty)] = {
                'x': tx * voxyproxy.TILE_WIDTH,
                'y': ty * voxyproxy.TILE_HEIGHT,
                'w': voxyproxy.TILE_WIDTH,
                'h': voxyproxy.TILE_HEIGHT,
                'rev': client.tile_revisions[(tx,ty)],
            }
        return result


    @json_expose
    def mouseevent(self, target, x, y, btn):
        config = GetConfig()
        if not config.has_target(target):
            return {}

        client = voxyproxy.Manager.getClient(target)
        if client is None or not client.isConnected():
            return "failure"
        
        reactor.callFromThread(client.pointerEvent, int(x), int(y), int(btn))
        return "ok"


    @json_expose
    def keyevent(self, target, key, state):
        config = GetConfig()
        if not config.has_target(target):
            return {}

        client = voxyproxy.Manager.getClient(target)
        if client is None or not client.isConnected():
            return "failure"
        reactor.callFromThread(client.keyEvent, int(key), int(state))
        return "ok"

    @cherrypy.expose
    def view(self, target):
        html = """
        <html>
            <head>
                <title>VoXY</title>
                <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
                <link href="/files/voxy.css" rel="stylesheet" type="text/css" />
                <script src="/files/jquery-1.4.2.min.js" type="text/javascript"></script>
                <script src="/files/jquery.timers-1.2.js" type="text/javascript"></script>
                <script src="/files/voxy_single.js" type="text/javascript"></script>
                <script type="text/javascript">
                    gTarget = '{0}';
                </script>                
            </head>
            <body>
                <div id='voxymain'>
                </div>
                <h2>Log area</h2>
                <div id='logarea'>
                </div>
            </body>
        </html>
        """.format(target)
        return html


def threadMain():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    cherrypy.config.update({
	    'server.socket_port': 8012,
	    'server.socket_timeout': 400,
	    'environment': 'production',
	    'log.error_file': 'voxy.log',
	    'log.screen': False,
	    'tools.sessions.on': True,
	    'tools.sessions.storage_type': "file",
	    'tools.sessions.storage_path': "./data",
	    'tools.sessions.timeout': 60
    })
    conf = {'/files': {'tools.staticdir.on': True,
	                  'tools.staticdir.dir': os.path.join(current_dir, 'webfiles')
    }}
    cherrypy.quickstart(VoxyWeb(), config = conf)


def start():
    cherrypy.server.socket_host = "0.0.0.0"
    wst = Thread(target=threadMain)
    wst.setDaemon(True)
    wst.start()

