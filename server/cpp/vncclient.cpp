//---------------------------------------------------------------------------
// VREng (Virtual Reality Engine)	http://vreng.enst.fr/
//
// Copyright (C) 1997-2008 Ecole Nationale Superieure des Telecommunications
//
// VREng is a free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public Licence as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// VREng is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//---------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "global.hpp"

//#include <gui.hpp>
#if (UBIT_VERSION_MAJOR < 6 || (UBIT_VERSION_MAJOR >= 6 && UBIT_WITH_X11)) //UBIT-X11
#include <X11/Xlib.h>	//XStringToKeysym
#include "vncclient.hpp"
//#include "ImgTile.h"

/*
 * Constructors
 */

VNCRGB::VNCRGB()
{
    Red = 0;
    Green = 0;
    Blue = 0;
}

VNCRGB::VNCRGB(const uint32_t &pixel)
{
    Red = (uint8_t) (pixel);
    Green = (uint8_t) (pixel >> 8);
    Blue = (uint8_t) (pixel >> 16);
}

VNCRGB::VNCRGB(uint8_t r, uint8_t g, uint8_t b)
{
    Red = r;
    Green = g;
    Blue = b;
}

// start
VNCClient::VNCClient(const char *server, int port, const char *pswdfile)    
    : rfbproto(server, port, pswdfile)
{
    fbWidth = 0;
    fbHeight = 0;
    framebuffer = NULL;
    viewonly = false;
    serverCutText = NULL;
    newServerCutText = false;
    //    rfbbuffer = NULL;
    //    rfbbuffer_size = 0;    
}

VNCClientTextured::VNCClientTextured(char *server, int port, char *pswdfile)
    : VNCClient(server, port, pswdfile)
{
    realScreenWidth = 0;
    realScreenHeight = 0;
}

bool VNCClient::VNCInit()
{    
    if (rfbproto.connectToRFBServer() && rfbproto.initialiseRFBConnection()) {

        rfbproto.setVisual32();

        if (!rfbproto.setFormatAndEncodings()) {
            error("VNCInit: unable to set default PixelFormat");
            VNCClose();
            return false;
        }        
        
        fbWidth = rfbproto.si.framebufferWidth;
        fbHeight = rfbproto.si.framebufferHeight;
        framebuffer = new VNCRGB[fbWidth * fbHeight];        
        if (!framebuffer) {
            error("VNCInit: unable to allocate memory for framebuffer");
            VNCClose();
            return false;
        }
        
        rfbbuffer = new char[sizeof(uint32_t) * fbWidth * fbHeight];        
        rfbbuffer_size = sizeof(uint32_t) * fbWidth * fbHeight;        
        if (!rfbbuffer) {            
            error("VNCInit: unable to allocate memory for framebuffer");
            VNCClose();
            return false;            
        }        
        
        return true;
    }
    else {
        error("VNCInit: connection or initialization impossible");
        VNCClose();
    }
    return false;
}

bool VNCClientTextured::VNCInit()
{
    if (rfbproto.connectToRFBServer() && rfbproto.initialiseRFBConnection()) {

        rfbproto.setVisual32();

        if (!rfbproto.setFormatAndEncodings()) {
            error("VNCInit: unable to set default PixelFormat");
            VNCClose();
            return false;
        }

        realScreenWidth = rfbproto.si.framebufferWidth;
        realScreenHeight = rfbproto.si.framebufferHeight;

        trace(DBG_VNC, "VNCInit: w=%d h=%d", realScreenWidth, realScreenHeight);

        uint16_t powerof2;
        for (powerof2 = 1; powerof2 < realScreenWidth; powerof2 *= 2) ;
        fbWidth = powerof2;
        for (powerof2 = 1; powerof2 < realScreenHeight; powerof2 *= 2) ;
        fbHeight = powerof2;

        trace(DBG_VNC, "VNCInit: fbw=%d fbh=%d", fbWidth, fbHeight);

        framebuffer = new VNCRGB[fbWidth * fbHeight];
        if (!framebuffer) {
            error("VNCInit: unable to allocate memory for framebuffer");
            VNCClose();
            return false;
        }
	
        return true;
    }
    else {
        error("VNCInit: connection or initialization impossible");
        VNCClose();
    }
    return false;
}

bool VNCClient::VNCClose()
{
    close(getSock());
    if (framebuffer) delete[] framebuffer; framebuffer = NULL;
    if (rfbbuffer) delete[] rfbbuffer; rfbbuffer = NULL; rfbbuffer_size = 0;    
    return true;    
}

/*
 * sendRFBEvent is an action which sends an RFB event.  It can be used in two
 * ways.  Without any parameters it simply sends an RFB event corresponding to
 * the X event which caused it to be called.  With parameters, it generates a
 * "fake" RFB event based on those parameters.  The first parameter is the
 * event type, either "ptr", "keydown", "keyup" or "key" (down&up).  For a
 * "key" event the second parameter is simply a keysym string as understood by
 * XStringToKeysym().  For a "ptr" event, the following three parameters are
 * just X, Y and the button mask (0 for all up, 1 for button1 down, 2 for
 * button2 down, 3 for both, etc).
 */
void VNCClient::sendRFBEvent(char **params, unsigned int *num_params)
{
    KeySym ks;
    int buttonMask, x, y;

    if (viewonly || !framebuffer) return;

    if (strncasecmp(params[0], "key", 3) == 0) {
        if (*num_params != 2) {
            error("invalid params: sendRFBEvent(key|keydown|keyup,<keysym>)");
            return;
        }
        ks = XStringToKeysym(params[1]);
        if (ks == NoSymbol) {
            error("Invalid keysym '%s' passed to sendRFBEvent", params[1]);
            return;
        }
        if (strcasecmp(params[0], "keydown") == 0)
            rfbproto.sendKeyEvent(ks, 1);
        else if (strcasecmp(params[0], "keyup") == 0)
            rfbproto.sendKeyEvent(ks, 0);
        else if (strcasecmp(params[0], "key") == 0) {
            rfbproto.sendKeyEvent(ks, 1);
            rfbproto.sendKeyEvent(ks, 0);
        }
        else {
            error("invalid event '%s' passed to sendRFBEvent", params[0]);
            return;
        }
    }
    else if (strcasecmp(params[0], "ptr") == 0) {
        if (*num_params == 4) {
            x = atoi(params[1]);
            y = atoi(params[2]);
            buttonMask = atoi(params[3]);
            rfbproto.sendPointerEvent(x, y, buttonMask);
        }
        else {
            error("invalid params: sendRFBEvent(ptr,x,y,buttonMask)");
            return;
        }
    }
    else
        error("invalid event '%s' passed to sendRFBEvent", params[0]);
	printf("invalid event '%s' passed to sendRFBEvent", params[0]);
}

uint8_t VNCClient::rescalePixValue(uint32_t Pix, uint8_t Shift, uint16_t Max)
{
    return (uint8_t) (((Pix >> Shift)&Max) * (256 / (Max+1))) ;
}

// can be used with a uint8_t, uint16_t or uint32_t Pixel
VNCRGB VNCClient::cardToVNCRGB(uint32_t Pixel)
{
    rfbPixelFormat PF = rfbproto.pixFormat;

    return VNCRGB(rescalePixValue(Pixel, PF.redShift, PF.redMax),
                  rescalePixValue(Pixel, PF.greenShift, PF.greenMax),
                  rescalePixValue(Pixel, PF.blueShift, PF.blueMax));
}

// All the methods we need to handle a given rect message
// And update the framebuffer

bool VNCClient::handleRAW32(int rx, int ry, int rw, int rh)
{
    uint32_t *src = (uint32_t *) rfbbuffer;
    VNCRGB *dest = (framebuffer + ry * fbWidth + rx);
    rfbPixelFormat PF = rfbproto.pixFormat;
    //uint32_t tmp;
  
    for (int h = 0; h < rh; h++) {
        for (int w = 0; w < rw; w++) {
            *dest++ = cardToVNCRGB(swap32IfLE(*src)); //BUG! segfault under Linux
            src++;
        }
        dest += fbWidth - rw;        
    }
    return true;
}

bool VNCClient::handleCR(int srcx, int srcy, int rx, int ry, int rw, int rh)
{
    VNCRGB *src;
    VNCRGB *dest;
    VNCRGB *buftmp = new VNCRGB[rh*rw];

    src = framebuffer + (srcy * fbWidth + srcx);
    dest = buftmp;
    for (int h = 0; h < rh; h++) {
        memcpy(dest,src,rw*sizeof(VNCRGB));
        src += fbWidth;
        dest += rw;
    }

    src = buftmp;
    dest = framebuffer + (ry * fbWidth + rx);
    for (int h = 0; h < rh; h++) {
        memcpy(dest,src,rw*sizeof(VNCRGB));
        src += rw;
        dest += fbWidth;
    }
    delete[] buftmp;
    return true;
}

void VNCClient::fillRect(int rx, int ry, int rw, int rh, VNCRGB pixel)
{
    VNCRGB *dest = framebuffer + (ry * fbWidth + rx);

    //trace(DBG_VNC, "fillRect: rx=%d ry=%d rw=%d rh=%d", rx, ry, rw, rh);

    for (int h = 0; h < rh; h++) {
        for (int w = 0; w < rw; w++)
            *dest++ = pixel;
        dest += fbWidth - rw;
    }
}

bool VNCClient::handleRRE32(int rx, int ry, int rw, int rh)
{
    rfbRREHeader hdr;
    uint32_t pix;
    rfbRectangle subrect;
    VNCRGB pixel;

    trace(DBG_VNC, "handleRRE32: rx=%d ry=%d rw=%d rh=%d", rx, ry, rw, rh);

    if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&hdr, sz_rfbRREHeader))
        return false;

    hdr.nSubrects = swap32IfLE(hdr.nSubrects);

    if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&pix, sizeof(pix)))
        return false;

    pixel = cardToVNCRGB(swap32IfLE(pix));
    fillRect(rx, ry, rw, rh, pixel);

    for (unsigned int i=0; i < hdr.nSubrects; i++) {
        if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&pix, sizeof(pix)))
            return false;

        if (!rfbproto.VNC_sock.ReadFromRFBServer((char*)&subrect, sz_rfbRectangle))
            return false;

        subrect.x = swap16IfLE(subrect.x);
        subrect.y = swap16IfLE(subrect.y);
        subrect.w = swap16IfLE(subrect.w);
        subrect.h = swap16IfLE(subrect.h);

        pixel = cardToVNCRGB(swap32IfLE(pix));
        fillRect(subrect.x, subrect.y, subrect.w, subrect.h, pixel);
    }
    return true;
}

bool VNCClient::handleCoRRE32(int rx, int ry, int rw, int rh)
{
    rfbRREHeader hdr;
    uint32_t pix;
    uint8_t *ptr;
    int x, y, ww, hh;
    VNCRGB pixel;

    trace(DBG_VNC, "handleCoRRE32: rx=%d ry=%d rw=%d rh=%d", rx, ry, rw, rh);

    if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&hdr, sz_rfbRREHeader))
        return false;

    hdr.nSubrects = swap32IfLE(hdr.nSubrects);

    if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&pix, sizeof(pix)))
        return false;

    pixel = cardToVNCRGB(swap32IfLE(pix));
    fillRect(rx, ry, rw, rh, pixel);

    if (!rfbproto.VNC_sock.ReadFromRFBServer(rfbbuffer, hdr.nSubrects * 8))
        return false;

    ptr = (uint8_t *) rfbbuffer;

    for (unsigned int i=0; i < hdr.nSubrects; i++) {
        pix = *(uint32_t *)ptr;
        ptr += 4;
        x = *ptr++;
        y = *ptr++;
        ww = *ptr++;
        hh = *ptr++;

        pixel = cardToVNCRGB(swap32IfLE(pix));
        fillRect(rx+x, ry+y, ww, hh, pixel);
    }
    return true;
}

#define GET_PIXEL32(pix, ptr) (((uint8_t*)&(pix))[0] = *(ptr)++,	\
							   ((uint8_t*)&(pix))[1] = *(ptr)++,	\
							   ((uint8_t*)&(pix))[2] = *(ptr)++,	\
							   ((uint8_t*)&(pix))[3] = *(ptr)++)

bool VNCClient::handleHextile32(int rx, int ry, int rw, int rh)
{
    uint32_t bg, fg;
    int i;
    uint8_t *ptr;
    int x, y, w, h;
    int sx, sy, sw, sh;
    uint8_t subencoding;
    uint8_t nSubrects;
    VNCRGB pixel;

    trace(DBG_VNC, "handleHextile32: rx=%d ry=%d rw=%d rh=%d",rx,ry,rw,rh);

    for (y = ry; y < ry+rh; y += 16) {
        for (x = rx; x < rx+rw; x += 16) {
            w = h = 16;
            if (rx+rw - x < 16)
                w = rx+rw - x;
            if (ry+rh - y < 16)
                h = ry+rh - y;

            if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&subencoding, 1))
                return false;
            if (subencoding & rfbHextileRaw) {
                if (!rfbproto.VNC_sock.ReadFromRFBServer(rfbbuffer, w * h * 4))
                    return false;
	
                handleRAW32(x, y, w, h);
                continue;
            }

            if (subencoding & rfbHextileBackgroundSpecified)
                if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&bg, sizeof(bg)))
                    return false;

            pixel = VNCRGB(swap32IfLE(bg));
            fillRect(x, y, w, h, pixel);

            if (subencoding & rfbHextileForegroundSpecified)
                if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&fg, sizeof(fg)))
                    return false;
            if (!(subencoding & rfbHextileAnySubrects))
                continue;

            if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&nSubrects, 1))
                return false;

            ptr = (uint8_t *) rfbbuffer;

            if (subencoding & rfbHextileSubrectsColoured) {
                if (!rfbproto.VNC_sock.ReadFromRFBServer(rfbbuffer, nSubrects * 6))
                    return false;

                for (i=0; i < nSubrects; i++) {
                    GET_PIXEL32(fg, ptr);
                    sx = rfbHextileExtractX(*ptr);
                    sy = rfbHextileExtractY(*ptr);
                    ptr++;
                    sw = rfbHextileExtractW(*ptr);
                    sh = rfbHextileExtractH(*ptr);
                    ptr++;
                    pixel = VNCRGB(swap32IfLE(fg));
                    fillRect(x+sx, y+sy, sw, sh, pixel);
                }
            }
            else {
                if (!rfbproto.VNC_sock.ReadFromRFBServer(rfbbuffer, nSubrects * 2))
                    return false;

                for (i=0; i < nSubrects; i++) {
                    sx = rfbHextileExtractX(*ptr);
                    sy = rfbHextileExtractY(*ptr);
                    ptr++;
                    sw = rfbHextileExtractW(*ptr);
                    sh = rfbHextileExtractH(*ptr);
                    ptr++;
                    pixel = VNCRGB(swap32IfLE(fg));
                    fillRect(x+sx, y+sy, sw, sh, pixel);
                }
            }
        }
    }
    return true;
}

bool VNCClient::handleRFBServerMessage()
{
    rfbServerToClientMsg msg;
  
    if (!framebuffer)
        return false;
    if (!rfbproto.VNC_sock.ReadFromRFBServer((char *) &msg, 1))        
        return false;

    this->srvmsg = msg.type;
    switch (msg.type) {

    case rfbSetColourMapEntries:
        {
            error("handleRFBServerMessage: rfbSetColourMapEntries not supported yet");
            uint16_t rgb[3];

            if (!rfbproto.VNC_sock.ReadFromRFBServer(((char *)&msg) + 1,
                                                     sz_rfbSetColourMapEntriesMsg - 1))
                return false;

            msg.scme.firstColour = swap16IfLE(msg.scme.firstColour);
            msg.scme.nColours = swap16IfLE(msg.scme.nColours);

            for (int i=0; i < msg.scme.nColours; i++) {
                if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)rgb, 6))
                    return false;
            }
            break;
        }

    case rfbFramebufferUpdate:
        {
            trace(DBG_VNC, "handleRFBServerMessage: rfbFramebufferUpdate");

            rfbFramebufferUpdateRectHeader rect;
            int linesToRead;
            int bytesPerLine;
            int i;

            if (!rfbproto.VNC_sock.ReadFromRFBServer(((char *)&msg.fu) + 1, sz_rfbFramebufferUpdateMsg - 1))
                return false;

            msg.fu.nRects = swap16IfLE(msg.fu.nRects);
	  
            this->recv_rect.r.x = rfbproto.si.framebufferWidth;
            this->recv_rect.r.y = rfbproto.si.framebufferHeight;
            this->recv_rect.r.w = 0;
            this->recv_rect.r.h = 0;
	  
            for (i = 0; i < msg.fu.nRects; i++) {
		
                if (!rfbproto.VNC_sock.ReadFromRFBServer((char *)&rect, sz_rfbFramebufferUpdateRectHeader))
                    return false;

                rect.r.x = swap16IfLE(rect.r.x);
                rect.r.y = swap16IfLE(rect.r.y);
                rect.r.w = swap16IfLE(rect.r.w);
                rect.r.h = swap16IfLE(rect.r.h);

                rect.encoding = swap32IfLE(rect.encoding);
		
                trace(DBG_VNC, "rect.r.x: %d, rect.r.y: %d, rect.r.w: %d, rect.r.h: %d, encode: %d\n",
                      rect.r.x, rect.r.y, rect.r.w, rect.r.h, rect.encoding);
		
                if ((rect.r.x + rect.r.w > rfbproto.si.framebufferWidth) ||
                    (rect.r.y + rect.r.h > rfbproto.si.framebufferHeight)) {
                    error("Rect too large: %dx%d at (%d, %d)",
                          rect.r.w, rect.r.h, rect.r.x, rect.r.y);
                    return false;
                }
                if ((rect.r.h * rect.r.w) == 0) {
                    error("Zero rfbproto.size rect - ignoring");
                    continue;
                }
		
                this->recv_rect.r.x = MIN(rect.r.x, this->recv_rect.r.x);
                this->recv_rect.r.y = MIN(rect.r.y, this->recv_rect.r.y);
                this->recv_rect.r.w = MAX(rect.r.x + rect.r.w, this->recv_rect.r.w);
                this->recv_rect.r.h = MAX(rect.r.y + rect.r.h, this->recv_rect.r.h);
		
                switch (rect.encoding) {

                case rfbEncodingRaw:
                    bytesPerLine = rect.r.w * rfbproto.pixFormat.bitsPerPixel / 8;
                    //                    linesToRead = sizeof(rfbbuffer) / bytesPerLine;                    
                    linesToRead = rfbbuffer_size / bytesPerLine;                    

                    //trace(DBG_VNC, "handleRFBServerMessage: rfbFramebufferUpdate rfbEncodingRaw bytesPerLine=%d linesToRead=%d",
                    //		bytesPerLine, linesToRead);
		  
                    while (rect.r.h > 0) {
                        if (linesToRead > rect.r.h)
                            linesToRead = rect.r.h;
                        if (!rfbproto.VNC_sock.ReadFromRFBServer(rfbbuffer,bytesPerLine * linesToRead))
                            return false;

                        if (!handleRAW32(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                            return false;
			
                        rect.r.h -= linesToRead;
                        rect.r.y += linesToRead;
                    }
                    break;

                case rfbEncodingCopyRect:
                    {
                        rfbCopyRect cr;
                        if (!rfbproto.VNC_sock.ReadFromRFBServer((char *) &cr, sz_rfbCopyRect))
                            return false;

                        cr.srcX = swap16IfLE(cr.srcX);
                        cr.srcY = swap16IfLE(cr.srcY);
	
                        if (!handleCR(cr.srcX, cr.srcY, rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                            return false;
                        break;
                    }

                case rfbEncodingRRE:
                    if (!handleRRE32(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
                        return false;
                    break;
                case rfbEncodingCoRRE:
                    if (!handleCoRRE32(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
                        return false;
                    break;
                case rfbEncodingHextile:
                    if (!handleHextile32(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
                        return false;
                    break;
		  
                default:
                    error("Unknown rect encoding %d", (int) rect.encoding);
                    return false;
                }
            }
	  
            if (!sendIncrementalFramebufferUpdateRequest())
                return false;
            break;
        }

    case rfbBell:
        //XBell(dpy,100);
        break;

    case rfbServerCutText:
        if (!rfbproto.VNC_sock.ReadFromRFBServer(((char *) &msg) + 1, sz_rfbServerCutTextMsg - 1))
            return false;
        
        msg.sct.length = swap32IfLE(msg.sct.length);

        if (serverCutText) delete[] serverCutText; serverCutText = NULL;
        serverCutText = new char[msg.sct.length+1];

        if (!rfbproto.VNC_sock.ReadFromRFBServer(serverCutText, msg.sct.length))
            return false;

        serverCutText[msg.sct.length] = 0;
        newServerCutText = true;
        break;

    default:
        error("Unknown message type %d from VNC server", msg.type);
        return false;
    }
    return true;
}

int VNCClient::getSock()
{
    return rfbproto.getSock();
}

bool VNCClient::sendIncrementalFramebufferUpdateRequest()   
{
    return rfbproto.sendIncrementalFramebufferUpdateRequest();
}

bool VNCClient::sendFramebufferUpdateRequest(int x, int y, int w, int h, bool incremental)
{
    return rfbproto.sendFramebufferUpdateRequest(x, y, w, h, incremental);
}

#endif
