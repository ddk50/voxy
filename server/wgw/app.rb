require 'rubygems'
require 'sinatra'

set :public, "../../client"

get '/getuser' do
	%q!<setuserid id="0" />!
end
