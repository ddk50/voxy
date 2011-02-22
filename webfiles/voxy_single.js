
$(initVoXY);

MainPanel = null;
MsgSerial = 0;
tile_revisions = {};
nVoXYUpdateInterval = 100;

MouseMoved = 0;
MouseX = 0;
MouseY = 0;
MouseBtn = 0;


function rand_int(max)
{
    return Math.floor( Math.random() * max );
}


// 初期化ルーチン
function initVoXY()
{
    $('#voxymain').append("<div id='header'><h2 id='hostname'>"+gTarget+"</h2><a href='javascript:history.back();'>back to list</a></div>");
    $('#voxymain').append("<div id='view'></div>");

    $('#view')
		.width(640).height(480)
		.bind('mousedown', OnMouseDown)
		.bind('mouseup', OnMouseUp)
		.bind('mousemove', OnMouseMove)
		.bind('contextmenu', OnContextMenu)
	;

	$(document)
		.keydown(OnKeyDown)
		.keyup(OnKeyUp)
	;

    getFullImg();
}

function getFullImg()
{
    $.ajax({
        type: "POST",
        url: '/fullimg_revisions',
        data: {target: gTarget},
        dataType: 'json',
        success: recvRevisions
    })

}
function recvRevisions(data)
{
    for(var key in data)
    {
        info = data[key];
        tag_id = "tile_" + key;
        imgtag = $('#'+tag_id);
        if(imgtag.length == 0)
        {
            $('#view').append("<img id='"+tag_id+"' />");
            imgtag = $('#'+tag_id);
            imgtag.css('position', 'absolute');
            imgtag.css('left', info.x);
            imgtag.css('top', info.y);
            imgtag.width(info.w);
            imgtag.height(info.h);
            if($('#view').width() < info.w + info.x)
                $('#view').width(info.w + info.x);
            if($('#view').height() < info.h + info.y)
                $('#view').height(info.h + info.y);
        }
        if(key in tile_revisions && tile_revisions[key] == info.rev)
        {
            continue;
        }
        tile_revisions[key] = info.rev;
        MsgSerial = rand_int(256*256*256*128);
        imgtag.attr('src', '/tileimg?target=' + gTarget + "&tile=" + key + '&sid=' + MsgSerial);
    }
    if(MouseMoved)
    {
        SendMouseEvent();
    }
    window.setTimeout(getFullImg, nVoXYUpdateInterval);
}

function WriteLog(str)
{
	$('#logarea').append(str + '<br />');
}



function SendMouseEvent()
{
    $.ajax({
        type: "POST",
        url: '/mouseevent',
        data: {
            target: gTarget, 
            x: MouseX, 
            y: MouseY,
            btn: MouseBtn
        },
        dataType: 'json'
    })
    MouseMoved = 0;
}


function KeyCodeJSToRFB(js_key)
{
    var norm_key = KeyCode.translate_key_code(js_key);

    // for ASCII code
    if(0x41 <= norm_key && norm_key <= 0x5a)
    {
        return norm_key + 32;
    }

    if(norm_key == 16) return 0xffe1;
    if(norm_key == 17) return 0xffe3;
    if(norm_key == 18) return 0xffe9;
    if(norm_key == 37) return 0xff51;
    if(norm_key == 38) return 0xff52;
    if(norm_key == 39) return 0xff53;
    if(norm_key == 40) return 0xff54;
    if(norm_key == 33) return 0xff55;
    if(norm_key == 34) return 0xff56;
    if(norm_key == 36) return 0xff50;
    if(norm_key == 35) return 0xff57;
    if(norm_key == 45) return 0xff63;
    if(norm_key == 46) return 0xffff;
    
    // for other special keys
    if(norm_key < 0x41)
    {
        return 0xff00 + norm_key;
    }

    return norm_key;
}

function SendKeyEvent(keyCode, s)
{
    $.ajax({
        type: "POST",
        url: '/keyevent',
        data: {
            target: gTarget, 
            key: KeyCodeJSToRFB(keyCode),
            state: s
        },
        dataType: 'json'
    })
    MouseMoved = 0;
}


function OnMouseDown(e)
{
    e.preventDefault();
    MouseX = e.pageX - this.offsetLeft;
    MouseY = e.pageY - this.offsetTop;
    MouseBtn = MouseBtn | (1 << (e.which-1));
    SendMouseEvent();
    WriteLog("Btn:" + MouseBtn);
}

function OnMouseUp(e)
{
    e.preventDefault();
    MouseX = e.pageX - this.offsetLeft;
    MouseY = e.pageY - this.offsetTop;
    flag = (1 << (e.which-1));
    MouseBtn = (MouseBtn | flag) ^ flag;
    SendMouseEvent();
}

function OnMouseMove(e)
{
    e.preventDefault();
    MouseMoved = 1;
    MouseX = e.pageX - this.offsetLeft;
    MouseY = e.pageY - this.offsetTop;
}

function OnKeyUp(e)
{
    e.preventDefault();
    SendKeyEvent(e.keyCode, 1);
}

function OnKeyDown(e)
{
    e.preventDefault();
    SendKeyEvent(e.keyCode, 0);
}

// コンテキストメニューの出現抑制
function OnContextMenu(e)
{
    return false;
}


