require 'rubygems'
require 'sinatra'
require 'json'

set :public, "../../client"

# 通信に使用するソケット
$socket = nil
# 中途半端なS式をためておく受信バッファ
$recv_buf = ""
# 画像ファイルがあるパス
$picture_path = "/pics/"

helpers do
	# cppサーバーからのS式を読み取り、配列にする
	def ReadServerMessage()
		text = $recv_buf + $socket.gets
		sexpr_list = text.split(")")
		if sexpr_list.size == 0 then
			return []
		end
		# 最後の一つは中途半端なS式か空白文字になる
		$recv_buf = sexpr_list.pop
		sexpr_list.map do |i|
			sexpr = i[2..-1].lstrip.split(" ")
			sexpr.map! do |s|
				# ダブルクォートを外す
				if s[0,1] == '"' then
					s[1..-2]
				else
					s
				end
			end
			# UPDATETILEだったら画像ファイルのパスを追加
			if sexpr[0] == "UPDATETILE" then
				sexpr[1] = $picture_path + sexpr[1]
			end
			sexpr
		end
	end
end

# トップページからのリダイレクト
get '/' do
	redirect '/voxyview.html'
end

# データ取得
get '/listen' do
	session_id = params[:session_id]
	$socket.puts "(:GETUPDATA)"
	{
		:smsg => ReadServerMessage()
	}.to_json
end

# ログイン
get '/login' do
	hostname = params[:hostname]
	password = params[:password]
	
	$socket = TCPSocket.open("localhost", 9999)
	
	$socket.puts("(:VNCCONNECT %s %s)" % [hostname, password])
	{
		:stat => "ok",
		:session_id => 1,
		:smsg => ReadServerMessage(),
	}.to_json
end

# 切断
get '/disconnect' do
	session_id = params[:session_id]
	$socket.puts "(:VNCDISCONNECT)"
	$socket.close
	{
		:stat => "ok"
	}.to_json
end


# old
get '/getuser' do
	{:id => 1}.to_json
end

get "/execemu/$userid" do
	{:status => 'ok'}.to_json
end
