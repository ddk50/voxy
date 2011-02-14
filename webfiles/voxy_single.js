
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

	$(window)
		.bind('keydown', OnKeyDown)
		.bind('keyup', OnKeyUp)
	;
    getFullImg();
}

function getFullImg()
{
    $.ajax({
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


function SendKeyEvent(keyCode, s)
{
    $.ajax({
        url: '/keyevent',
        data: {
            target: gTarget, 
            key: keyCode,
            state: s
        },
        dataType: 'json'
    })
    MouseMoved = 0;
}


function OnMouseDown(e)
{
    e.preventDefault();
    MouseX = e.pageX;
	MouseY = e.pageY;
    MouseBtn = MouseBtn | (1 << (e.which-1));
    SendMouseEvent();
    WriteLog("Btn:" + MouseBtn);
}

function OnMouseUp(e)
{
    e.preventDefault();
    MouseX = e.pageX;
	MouseY = e.pageY;
    flag = (1 << (e.which-1));
    MouseBtn = (MouseBtn | flag) ^ flag;
    SendMouseEvent();
}

function OnMouseMove(e)
{
    e.preventDefault();
    MouseMoved = 1;
    MouseX = e.pageX;
	MouseY = e.pageY;
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


