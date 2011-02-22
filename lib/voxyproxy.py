#!/usr/bin/python
# -*- coding: utf-8 -*-



from voxycommon import *


import time
import rfb
import Image
from array import array
import ConfigParser


from twisted.python import usage, log
from twisted.internet import reactor, protocol
from twisted.application import internet, service


SMALLMODE_HEIGHT = 120
SMALLMODE_WIDTH = 160

TILE_HEIGHT = 150
TILE_WIDTH = 150

FULLIMAGE_TIMEOUT = 4.0
REFRESH_TIMEOUT = 20.0
RECONNECT_TIMEOUT = 5.0

Manager = None



gClient = None
def kickoff():
    print "Updating"
    gClient.refreshAll()
    return "KickResult"

class VNCClient(rfb.RFBClient):
    def __init__(self):
        rfb.RFBClient.__init__(self)

        self.small_image_buffer = None
        self.width = 640
        self.height = 480
        self.full_mode = False
        self.rebuildBuffer()
        
        self.image_revision = 0
        self.tile_revisions = {}
        self.status = "created"
        self.last_connect_trying_time = 0.0
        self.last_refreshed = 0.0
        self.last_fullaccess = 0.0
        self.full_update_flag = False

    def save(self, fn = "test.png"):
        self.full_image_buffer.save(fn)

    def saveSmallToStrm(self, strm):
        self.small_image_buffer.save(strm, "png")

    def saveTileToStrm(self, strm, tx, ty):
        x0 = tx * TILE_WIDTH
        y0 = ty * TILE_HEIGHT
        x1 = (tx+1) * TILE_WIDTH
        y1 = (ty+1) * TILE_HEIGHT
        self.full_image_buffer.crop( (x0,y0,x1,y1) ).save(strm, "png")
        self.last_fullaccess = time.time()

    def useFullImage(self):
        self.setFullMode(True)
        self.last_fullaccess = time.time()

    def setFullMode(self, mode):
        if self.full_mode == mode:
            return
        self.full_mode = mode
        self.rebuildBuffer()
        self.full_update_flag = True

    def rebuildBuffer(self):
        if self.small_image_buffer is None:
            self.small_image_buffer = Image.new("RGB", (SMALLMODE_WIDTH, SMALLMODE_HEIGHT), (0,0,0))
        self.full_image_buffer = Image.new("RGB", (self.width, self.height), (0,0,0))

    def clearBuffer(self):
        self.small_image_buffer = Image.new("RGB", (SMALLMODE_WIDTH, SMALLMODE_HEIGHT), (0,0,0))
        self.full_image_buffer = Image.new("RGB", (self.width, self.height), (0,0,0))

    def refreshAll(self):
        self.framebufferUpdateRequest()
        self.last_refreshed = time.time()

    def vncConnectionMade(self):
        self.rebuildBuffer()
        self.setPixelFormat(
            bpp=32, depth=24, bigendian=0, truecolor=1,
            redmax=255, greenmax=255, bluemax=255,
            redshift=0, greenshift=8, blueshift=16
        )

        """
        self.setPixelFormat(bpp=8, depth=8, bigendian=0, truecolor=1,
            redmax=7,   greenmax=7,   bluemax=3,
            redshift=5, greenshift=2, blueshift=0
        )
        self.palette = self.screen.get_palette()
        """
        self.status = "connected"
        self.last_refreshed = time.time()
        self.framebufferUpdateRequest()

    def isConnected(self):
        return self.status == "connected"

    def checkMax(self, x,y):
        if self.full_image_buffer.size[0] < x or self.full_image_buffer.size[1] < y:
            self.clearBuffer()
            self.full_update_flag = True
            print "Resolution Changed"
            
    def copyRectangle(self, srcx, srcy, x, y, width, height):
        print "copyrect", (srcx, srcy, x, y, width, height)

    def fillRectangle(self, x, y, width, height, color):
        print "Fill:",x,y,width,height
        #self.screen.fill(struct.unpack("BBBB", color), (x, y, width, height))


    def updateRectangle(self, x, y, w, h, data):
        # print "update: ", (x, y, w, h)
        self.checkMax(x+w, y+h)
        
        adata = array("B", data)
        spix = self.small_image_buffer.load()
        fpix = self.full_image_buffer.load()
        
        scale_x = int(self.width / SMALLMODE_WIDTH) + 1
        scale_y = int(self.height / SMALLMODE_HEIGHT) + 1
        x0 = x / scale_x
        y0 = y / scale_y
        for iy in range(0, int(h/scale_y)):
            for ix in range(0, int(w/scale_x)):
                pbase = ((iy*scale_y)*w + ix*scale_x)*4
                spix[x0+ix, y0+iy] = (adata[pbase], adata[pbase+1], adata[pbase+2])
        if self.full_mode:
            aidx = 0
            for iy in range(0, h):
                for ix in range(0, w):
                    fpix[x+ix, y+iy] = (adata[aidx], adata[aidx+1], adata[aidx+2])
                    aidx += 4
    
        self.updateTileRevision(x,y,w,h)
        
    def updateTileRevision(self,x,y,w,h):
        for iy in range(int(y/TILE_HEIGHT), int((y+h)/TILE_HEIGHT) + 1):
            for ix in range(int(x/TILE_WIDTH), int((x+w)/TILE_WIDTH) + 1):
                self.tile_revisions[(ix,iy)] = self.image_revision

    def beginUpdate(self):
        if self.full_mode and time.time() - self.last_fullaccess > FULLIMAGE_TIMEOUT:
            print "FullImage timeout"
            self.setFullMode(False)
        self.image_revision += 1
    
    def commitUpdate(self, rectangles=None):
        # self.save(self.target_name + "_small.png")
        if time.time() - self.last_refreshed > REFRESH_TIMEOUT:
            self.last_refreshed = time.time()
            self.full_update_flag = True
        
        if self.full_update_flag:
            self.full_update_flag = False
            self.framebufferUpdateRequest()
            print "Full update"
        else:
            self.framebufferUpdateRequest(incremental=1)
        
    def getImageRevision(self):
        return self.image_revision

        
class VNCClientFactory(protocol.ClientFactory):
    def __init__(self, mgr, target_name, password):
        self.target_name = target_name
        self.mgr = mgr
        self.shared = 0
        self.password = password
        self.client_protocol = VNCClient()
        self.client_protocol.target_name = self.target_name
        self.client_protocol.factory = self
    
    def getClient(self):
        return self.client_protocol

    def clientConnectionLost(self, connector, reason):
        print reason
        self.client_protocol.status = "closed"
        self.client_protocol.clearBuffer()
        self.client_protocol.last_connect_trying_time = time.time()

    def clientConnectionFailed(self, connector, reason):
        print "connection failed:", reason
        self.client_protocol.status = "closed"
        self.client_protocol.clearBuffer()
        self.client_protocol.last_connect_trying_time = time.time()

    def buildProtocol(self, addr):
        return self.client_protocol

class VNCManager:
    def __init__(self):
        host = ""
        port = 5900
        self.application = service.Application("rfbproxy") # create Application
        self.targets = {}

    def run(self):
        reactor.run()
        
    def newClientObject(self, obj):
        self.targets[obj.target_name] = obj

    def newConnection(self, *prms):
        reactor.callFromThread(self.newConnection_inmain, *prms)

    def newConnection_inmain(self, target_name, host, port, password):
        if target_name in self.targets:
            if self.targets[target_name].status != "retry":
                return
        factory = VNCClientFactory(self, target_name, password)
        self.targets[target_name] = factory.getClient()
        vncClient = internet.TCPClient(host, port, factory)
        vncClient.setServiceParent(self.application)
        vncClient.startService()

    def getClient(self, target_name, try_connect = False):
        if not try_connect:
            client = self.targets.get(target_name, None)
            if isinstance(client, basestring):
                return None
            return client
        
        client = self.targets.get(target_name, None)
        if isinstance(client, basestring):
            return None
        if client is not None:
            # 切断された場合の再接続
            if client.status == "closed" and time.time() - client.last_connect_trying_time > RECONNECT_TIMEOUT:
                print "Retry connect: ", target_name
                client.status = "retry"
            else:
                return client

        config = GetConfig()
        if not config.has_target(target_name):
            return None

        self.newConnection(
            target_name,
            config.get_target_host(target_name),
            config.get_target_port(target_name),
            config.get_target_password(target_name)
        )
        return None

def run():
    global Manager
    Manager = VNCManager()
    print "Manager = ", str(Manager)
    Manager.run()



