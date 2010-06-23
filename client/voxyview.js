
$(initVoXY);

MainPanel = null;

// 初期化ルーチン
function initVoXY()
{
	MainPanel = new VoXYPanel($('#voxymain'), 'main');
	MainPanel.ShowLoginForm();
}

// ログの書き込み
// voxy.jsからも呼ばれる
function WriteLog(str)
{
	$('#logarea').append(str + '<br />');
}
