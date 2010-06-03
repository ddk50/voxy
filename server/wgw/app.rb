require 'rubygems'
require 'sinatra'
require 'json'

set :public, "../../client"

get '/' do
  redirect '/test.html'
end

get '/getuser' do
  {:id => 1}.to_json
end

get "/execemu/$userid" do
  {:status => 'ok'}.to_json
end
