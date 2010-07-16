#!/usr/bin/env ruby
require 'rubygems'
require 'sinatra'
require 'json'
require 'pp'

pubdir = "@DATADIR@/voxy/pub_www"
if pubdir =~ /@/
  pubdir = '../../client'
end
set :public, pubdir

# セッション
$sessions = []

# 画像ファイルがあるパス
$picture_path = "/pics/"

$max_recvlen = 64*1024;

# helpers do

	class VoxySession
		def initialize(sock, id)
			@sock = sock
			@id = id
			@recv_buf = ""
		end
		# cppサーバーからのS式を読み取り、配列にする
		def ReadServerMessage()
			begin
				msg = @sock.recv_nonblock($max_recvlen)
			rescue Errno::EAGAIN
				return []
			end
			text = @recv_buf + msg
			sexpr_list = text.split(")")
			if sexpr_list.size == 0 then
				return []
			end
			# 最後の一つは中途半端なS式か空白文字になる
			@recv_buf = sexpr_list.pop
			pp sexpr_list
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
		
		# VNX Proxyに送信する
		def Send(sexpr)
			@sock.puts sexpr
			p "SEND[%s]: %s" % [@id, sexpr]
		end
		
		# 接続閉じる
		def Close
			@sock.close
			@sock = nil
		end
	end
	
# end

# トップページからのリダイレクト
get '/' do
	redirect '/voxyview.html'
end

# データ取得
get '/listen' do
	session_id = params[:session_id].to_i
	session = $sessions[session_id]

	session.Send "(:GETUPDATA)"
	{
		:smsg => session.ReadServerMessage()
	}.to_json
end



# マウスのイベント
get '/mouseevent' do
	session_id = params[:session_id].to_i
	session = $sessions[session_id]

	btn_code = params[:btn]
	if params[:btn_state].to_i != 0 then
		btn_code = -1
	end
	session.Send "(:MOUSEEVENT %s %s %s)" % [params[:x], params[:y], params[:btn]]
	{
		:smsg => session.ReadServerMessage()
	}.to_json
end


# キーイベント
get '/keyevent' do
	session_id = params[:session_id].to_i
	session = $sessions[session_id]

	if params[:key_state].to_i == 0 then
		session.Send "(:KEYEVENT %s)" % params[:key_code]
		{
			:smsg => session.ReadServerMessage()
		}.to_json
	else
		{
			:smsg => []
		}.to_json	
	end
end

# ログイン
get '/login' do
	hostname = params[:hostname]
	password = params[:password]
	
	
	session_id = -1
	# session-idの再利用
	$sessions.each_with_index do |sess, i| 
		if sess == nil then
			session_id = i
			break
		end
	end
	
	if session_id == -1 then
		session_id = $sessions.length
		new_session = VoxySession.new(TCPSocket.open("localhost", 9999), session_id)
		$sessions.push new_session
	else
		new_session = VoxySession.new(TCPSocket.open("localhost", 9999), session_id)
		$sessions[session_id] = new_session	
	end

	# new_session.Send("(:VNCCONNECT \"%s\" \"%s\")" % [hostname, password])
	# new_session.Send('(:VNCCONNECT "157.82.3.89:5900" "test")')
    new_session.Send('(:VNCCONNECT "127.0.0.1:5900" "test")')
	{
		:stat => "ok",
		:session_id => session_id,
		:smsg => new_session.ReadServerMessage(),
	}.to_json
end

# 切断
get '/disconnect' do
	session_id = params[:session_id].to_i
	session = $sessions[session_id]
	
	session.Send "(:VNCDISCONNECT)"
	session.Close
	$sessions[session_id] = nil
	
	
	{
		:stat => "ok"
	}.to_json
end

