
$(initVoXY);

MainPanel = null;
MsgSerial = 0;
target_revisions = {};
target_list = [];
nVoXYUpdateInterval = 1000;

function rand_int(max)
{
    return Math.floor( Math.random() * max );
}

// 初期化ルーチン
function initVoXY()
{
    // $('#voxymain');
    $.ajax({
        url: '/jlist',
        data: 0,
        dataType: 'json',
        context: this,
        success: initList
    });
}

function initList(data)
{
    target_list = new Array();
    for(var i = 0; i < data.targets.length; i++)
    {
        btn_html = "";
        if("cmd_vmstart" in data.targets[i])
        {
            btn_html += "<button onclick='do_cmd(\""+data.targets[i].name+"\", \"vmstart\");'>"
                +"<img src='/files/start.png' alt='start' /></button>";
        }
        if("cmd_vmstop" in data.targets[i])
        {
            btn_html += "<button onclick='do_cmd(\""+data.targets[i].name+"\", \"vmstop\");'>"
                +"<img src='/files/stop.png' alt='stop' /></button>";
        }
        
        $('#voxymain').append(
            "<div class='cell' id='t_div_"+data.targets[i].name+"'>"
            +"<a href='/view?target="+data.targets[i].name+"'>"
            +"<img class='t_img' id='t_img_"+data.targets[i].name+"' /></a><br />"
            + btn_html + data.targets[i].name + "</div>");
        target_list[i] = data.targets[i].name;
    }

    revRequest();
}

function do_cmd(target, cmd)
{
    $.ajax({
        url: '/jcmd',
        data: {"target": target, "cmd": cmd},
        dataType: 'json',
        context: this,
        success: command_done
    });
}

function command_done(data)
{
    if("alert" in data)
    {
        alert(data.alert);
    }
}

function revRequest()
{
    $.ajax({
        url: '/jrev',
        data: {"targets": target_list},
        dataType: 'json',
        context: this,
        success: gotRev
    });

}
function gotRev(data)
{
    for(var target in data)
    {
        if(target in target_revisions)
        {
            if(target_revisions[target] == data[target])
            {
                continue;
            }
        }
        target_revisions[target] = data[target];
        MsgSerial = rand_int(256*256*256*128);
        $('#t_img_' + target).attr('src', '/smallimg?target=' + target + '&sid=' + MsgSerial);
    }

    window.setTimeout(revRequest, nVoXYUpdateInterval);
}

// ログの書き込み
// voxy.jsからも呼ばれる
function WriteLog(str)
{
	$('#logarea').append(str + '<br />');
}

